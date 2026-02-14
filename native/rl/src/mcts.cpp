#include "neutron_rl/mcts.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <random>

namespace neutron_rl {

// MCTSNode implementation

MCTSNode::MCTSNode(const GameState& state, float prior,
                   MCTSNode* parent, int action)
    : state_(state), prior_(prior), parent_(parent), action_(action) {}

float MCTSNode::q_value() const {
    if (visit_count_ == 0) {
        return 0.0f;
    }
    return value_sum_ / static_cast<float>(visit_count_);
}

MCTSNode* MCTSNode::select_child(float c_puct) {
    float best_score = -std::numeric_limits<float>::infinity();
    MCTSNode* best_child = nullptr;

    float sqrt_parent_visits = std::sqrt(static_cast<float>(visit_count_));

    for (auto& child : children_) {
        // PUCT formula: Q(s,a) + c_puct * P(s,a) * sqrt(N(s)) / (1 + N(s,a))
        // Only negate Q when child has a different player (opponent).
        // In Neutron, neutron-phase -> pawn-phase keeps the same player.
        float q;
        if (child->state().current_player() != state_.current_player()) {
            q = -child->q_value();  // Opponent's node
        } else {
            q = child->q_value();   // Same player's node
        }
        float u = c_puct * child->prior_ * sqrt_parent_visits /
                  (1.0f + static_cast<float>(child->visit_count_));
        float score = q + u;

        if (score > best_score) {
            best_score = score;
            best_child = child.get();
        }
    }

    return best_child;
}

void MCTSNode::expand(const std::vector<float>& policy_logits) {
    auto legal_actions = state_.get_legal_actions();
    if (legal_actions.empty()) {
        return;
    }

    // Compute softmax over legal actions
    float max_logit = -std::numeric_limits<float>::infinity();
    for (int action : legal_actions) {
        max_logit = std::max(max_logit, policy_logits[action]);
    }

    float sum_exp = 0.0f;
    std::vector<float> probs;
    probs.reserve(legal_actions.size());
    for (int action : legal_actions) {
        float exp_logit = std::exp(policy_logits[action] - max_logit);
        probs.push_back(exp_logit);
        sum_exp += exp_logit;
    }

    // Normalize and create children
    children_.reserve(legal_actions.size());
    for (size_t i = 0; i < legal_actions.size(); ++i) {
        float prior = probs[i] / sum_exp;
        GameState child_state = state_.apply_action(legal_actions[i]);
        children_.push_back(std::make_unique<MCTSNode>(
            child_state, prior, this, legal_actions[i]));
    }
}

void MCTSNode::backpropagate(float value) {
    MCTSNode* node = this;
    float current_value = value;

    while (node != nullptr) {
        node->visit_count_++;
        node->value_sum_ += current_value;
        // Only flip sign when parent has a different current_player.
        // In Neutron, neutron-phase -> pawn-phase keeps the same player,
        // so we must NOT flip between those consecutive tree levels.
        if (node->parent_ != nullptr &&
            node->state().current_player() != node->parent_->state().current_player()) {
            current_value = -current_value;
        }
        node = node->parent_;
    }
}

std::unordered_map<int, int> MCTSNode::get_visit_counts() const {
    std::unordered_map<int, int> counts;
    for (const auto& child : children_) {
        counts[child->action()] = child->visit_count();
    }
    return counts;
}

// MCTS implementation

MCTS::MCTS(ModelLoader& model, const MCTSConfig& config)
    : model_(model), config_(config) {}

void MCTS::simulate(MCTSNode* root) {
    MCTSNode* node = root;

    // Selection: traverse tree using PUCT until leaf
    while (!node->is_leaf() && !node->is_terminal()) {
        node = node->select_child(config_.c_puct);
    }

    // Handle terminal nodes
    if (node->is_terminal()) {
        auto winner = node->state().get_winner();
        float value = 0.0f;
        if (winner.has_value()) {
            // Value from perspective of the player to move at this node
            int node_player = node->state().current_player();
            value = (winner.value() == node_player) ? 1.0f : -1.0f;
        }
        node->backpropagate(value);
        return;
    }

    // Expansion and evaluation
    auto tensor = node->state().encode();
    auto result = model_.infer(tensor);

    // For P2, flip policy from player-relative to absolute action space
    auto& policy = result.policy_logits;
    if (node->state().current_player() == 2) {
        policy = GameState::flip_policy(policy);
    }

    node->expand(policy);

    // Backpropagate value from current node player's perspective
    node->backpropagate(result.value);
}

int MCTS::select_action(const std::unordered_map<int, int>& visit_counts) {
    if (config_.temperature == 0.0f) {
        // Greedy: select most visited
        int best_action = -1;
        int best_visits = -1;
        for (const auto& [action, visits] : visit_counts) {
            if (visits > best_visits) {
                best_visits = visits;
                best_action = action;
            }
        }
        return best_action;
    }

    // Temperature sampling
    std::vector<int> actions;
    std::vector<float> probs;
    float sum = 0.0f;

    for (const auto& [action, visits] : visit_counts) {
        actions.push_back(action);
        float prob = std::pow(static_cast<float>(visits), 1.0f / config_.temperature);
        probs.push_back(prob);
        sum += prob;
    }

    // Normalize
    for (float& p : probs) {
        p /= sum;
    }

    // Sample
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::discrete_distribution<> dist(probs.begin(), probs.end());
    return actions[dist(gen)];
}

std::vector<std::pair<int, float>> MCTS::visit_counts_to_probs(
    const std::unordered_map<int, int>& visit_counts) {
    int total_visits = 0;
    for (const auto& [_, visits] : visit_counts) {
        total_visits += visits;
    }

    std::vector<std::pair<int, float>> probs;
    for (const auto& [action, visits] : visit_counts) {
        float prob = static_cast<float>(visits) / static_cast<float>(total_visits);
        probs.emplace_back(action, prob);
    }

    // Sort by action for consistent output
    std::sort(probs.begin(), probs.end());
    return probs;
}

void MCTS::add_dirichlet_noise(MCTSNode* root) {
    if (config_.dirichlet_epsilon <= 0.0f) {
        return;
    }

    // This would add Dirichlet noise to root priors for exploration
    // Implementation omitted for simplicity - mainly needed during training
}

int MCTS::search(const GameState& state) {
    // Create root node
    MCTSNode root(state, 1.0f);

    // Initial expansion
    auto tensor = state.encode();
    auto result = model_.infer(tensor);

    // For P2, flip policy from player-relative to absolute action space
    auto& policy = result.policy_logits;
    if (state.current_player() == 2) {
        policy = GameState::flip_policy(policy);
    }

    root.expand(policy);

    // Add noise if configured
    add_dirichlet_noise(&root);

    // Run simulations
    for (int i = 0; i < config_.num_simulations; ++i) {
        simulate(&root);
    }

    // Select action
    auto visit_counts = root.get_visit_counts();
    return select_action(visit_counts);
}

std::vector<std::pair<int, float>> MCTS::search_with_probs(const GameState& state) {
    // Create root node
    MCTSNode root(state, 1.0f);

    // Initial expansion
    auto tensor = state.encode();
    auto result = model_.infer(tensor);

    // For P2, flip policy from player-relative to absolute action space
    auto& policy = result.policy_logits;
    if (state.current_player() == 2) {
        policy = GameState::flip_policy(policy);
    }

    root.expand(policy);

    // Add noise if configured
    add_dirichlet_noise(&root);

    // Run simulations
    for (int i = 0; i < config_.num_simulations; ++i) {
        simulate(&root);
    }

    // Return visit probabilities
    auto visit_counts = root.get_visit_counts();
    return visit_counts_to_probs(visit_counts);
}

}  // namespace neutron_rl
