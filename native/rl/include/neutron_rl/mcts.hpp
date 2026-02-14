#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "neutron_rl/game_state.hpp"
#include "neutron_rl/model_loader.hpp"

namespace neutron_rl {

/**
 * @brief Configuration for MCTS search.
 */
struct MCTSConfig {
    int num_simulations = 800;    // Number of MCTS simulations
    float c_puct = 1.5f;          // Exploration constant for PUCT
    float temperature = 0.0f;     // Temperature for action selection (0 = greedy)
    float dirichlet_alpha = 0.3f; // Dirichlet noise alpha for root
    float dirichlet_epsilon = 0.0f; // Dirichlet noise weight (0 = no noise)
};

/**
 * @brief Node in the MCTS tree.
 */
class MCTSNode {
public:
    /**
     * @brief Construct a new MCTS Node.
     *
     * @param state Game state at this node.
     * @param prior Prior probability from policy network.
     * @param parent Parent node (nullptr for root).
     * @param action Action that led to this node (-1 for root).
     */
    MCTSNode(const GameState& state, float prior,
             MCTSNode* parent = nullptr, int action = -1);

    /**
     * @brief Check if this node is a leaf (unexpanded).
     */
    bool is_leaf() const { return children_.empty() && !state_.is_terminal(); }

    /**
     * @brief Check if this node is terminal.
     */
    bool is_terminal() const { return state_.is_terminal(); }

    /**
     * @brief Get the Q-value (average value) for this node.
     */
    float q_value() const;

    /**
     * @brief Get the visit count.
     */
    int visit_count() const { return visit_count_; }

    /**
     * @brief Get the prior probability.
     */
    float prior() const { return prior_; }

    /**
     * @brief Get the action that led to this node.
     */
    int action() const { return action_; }

    /**
     * @brief Get the game state.
     */
    const GameState& state() const { return state_; }

    /**
     * @brief Get children nodes.
     */
    const std::vector<std::unique_ptr<MCTSNode>>& children() const { return children_; }

    /**
     * @brief Select the best child using PUCT.
     *
     * @param c_puct Exploration constant.
     * @return Pointer to the best child.
     */
    MCTSNode* select_child(float c_puct);

    /**
     * @brief Expand this node with policy priors.
     *
     * @param policy_logits Policy logits from the neural network.
     */
    void expand(const std::vector<float>& policy_logits);

    /**
     * @brief Backpropagate a value through the tree.
     *
     * @param value Value to backpropagate (from current player's perspective).
     */
    void backpropagate(float value);

    /**
     * @brief Get visit counts for all children.
     *
     * @return Map from action to visit count.
     */
    std::unordered_map<int, int> get_visit_counts() const;

    /**
     * @brief Get the parent node.
     */
    MCTSNode* parent() const { return parent_; }

private:
    GameState state_;
    float prior_;
    MCTSNode* parent_;
    int action_;

    std::vector<std::unique_ptr<MCTSNode>> children_;
    int visit_count_ = 0;
    float value_sum_ = 0.0f;
};

/**
 * @brief Monte Carlo Tree Search implementation.
 *
 * Uses neural network guidance for policy prior and value estimation.
 */
class MCTS {
public:
    /**
     * @brief Construct MCTS with a model loader.
     *
     * @param model Reference to the model loader.
     * @param config MCTS configuration.
     */
    MCTS(ModelLoader& model, const MCTSConfig& config = MCTSConfig{});

    /**
     * @brief Run MCTS search and return the best action.
     *
     * @param state Current game state.
     * @return Best action index.
     */
    int search(const GameState& state);

    /**
     * @brief Run MCTS search and return action probabilities.
     *
     * @param state Current game state.
     * @return Vector of (action, probability) pairs for legal actions.
     */
    std::vector<std::pair<int, float>> search_with_probs(const GameState& state);

    /**
     * @brief Get the configuration.
     */
    const MCTSConfig& config() const { return config_; }

    /**
     * @brief Set the configuration.
     */
    void set_config(const MCTSConfig& config) { config_ = config; }

    /**
     * @brief Set the number of simulations.
     */
    void set_num_simulations(int num) { config_.num_simulations = num; }

    /**
     * @brief Set the temperature.
     */
    void set_temperature(float temp) { config_.temperature = temp; }

private:
    ModelLoader& model_;
    MCTSConfig config_;

    /**
     * @brief Run one simulation (selection, expansion, evaluation, backprop).
     *
     * @param root Root node of the search tree.
     */
    void simulate(MCTSNode* root);

    /**
     * @brief Select action from visit counts.
     *
     * @param visit_counts Map from action to visit count.
     * @return Selected action.
     */
    int select_action(const std::unordered_map<int, int>& visit_counts);

    /**
     * @brief Convert visit counts to probabilities.
     *
     * @param visit_counts Map from action to visit count.
     * @return Vector of (action, probability) pairs.
     */
    std::vector<std::pair<int, float>> visit_counts_to_probs(
        const std::unordered_map<int, int>& visit_counts);

    /**
     * @brief Add Dirichlet noise to root priors.
     *
     * @param root Root node.
     */
    void add_dirichlet_noise(MCTSNode* root);
};

}  // namespace neutron_rl
