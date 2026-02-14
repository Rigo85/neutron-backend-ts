#include "neutron_rl/agent.hpp"

#include <algorithm>
#include <stdexcept>

namespace neutron_rl {

// DifficultyConfig implementation

DifficultyConfig DifficultyConfig::from_preset(Difficulty difficulty) {
    switch (difficulty) {
        case Difficulty::Easy:
            return {100, 0.5f};
        case Difficulty::Medium:
            return {300, 0.2f};
        case Difficulty::Hard:
        default:
            return {800, 0.0f};
    }
}

DifficultyConfig DifficultyConfig::from_simulations(int simulations, float temperature) {
    return {simulations, temperature};
}

// NeutronAgent implementation

NeutronAgent::NeutronAgent(const std::string& device)
    : difficulty_config_(DifficultyConfig::from_preset(Difficulty::Hard)) {
    model_loader_ = std::make_unique<ModelLoader>(device);
}

NeutronAgent::~NeutronAgent() = default;

NeutronAgent::NeutronAgent(NeutronAgent&&) noexcept = default;
NeutronAgent& NeutronAgent::operator=(NeutronAgent&&) noexcept = default;

bool NeutronAgent::load_model(const std::string& model_path) {
    if (!model_loader_->load(model_path)) {
        error_message_ = model_loader_->get_error_message();
        return false;
    }

    // Create MCTS instance
    MCTSConfig config;
    config.num_simulations = difficulty_config_.simulations;
    config.temperature = difficulty_config_.temperature;
    mcts_ = std::make_unique<MCTS>(*model_loader_, config);

    error_message_.clear();
    return true;
}

bool NeutronAgent::is_ready() const {
    return model_loader_ && model_loader_->is_loaded() && mcts_;
}

void NeutronAgent::set_difficulty(Difficulty difficulty) {
    difficulty_config_ = DifficultyConfig::from_preset(difficulty);
    if (mcts_) {
        mcts_->set_num_simulations(difficulty_config_.simulations);
        mcts_->set_temperature(difficulty_config_.temperature);
    }
}

void NeutronAgent::set_difficulty(int simulations, float temperature) {
    difficulty_config_ = DifficultyConfig::from_simulations(simulations, temperature);
    if (mcts_) {
        mcts_->set_num_simulations(difficulty_config_.simulations);
        mcts_->set_temperature(difficulty_config_.temperature);
    }
}

bool NeutronAgent::set_difficulty(const std::string& difficulty_name) {
    std::string lower_name = difficulty_name;
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (lower_name == "easy") {
        set_difficulty(Difficulty::Easy);
        return true;
    } else if (lower_name == "medium") {
        set_difficulty(Difficulty::Medium);
        return true;
    } else if (lower_name == "hard") {
        set_difficulty(Difficulty::Hard);
        return true;
    }

    error_message_ = "Unknown difficulty: " + difficulty_name;
    return false;
}

DifficultyConfig NeutronAgent::get_difficulty_config() const {
    return difficulty_config_;
}

int NeutronAgent::get_move(const std::array<int8_t, 25>& board,
                           int current_player,
                           Phase phase) {
    GameState state(board, current_player, phase);
    return get_move(state);
}

int NeutronAgent::get_move(const GameState& state) {
    if (!is_ready()) {
        throw std::runtime_error("Agent not ready - load a model first");
    }

    if (state.is_terminal()) {
        throw std::runtime_error("Cannot get move for terminal state");
    }

    return mcts_->search(state);
}

std::pair<int, std::vector<std::pair<int, float>>>
NeutronAgent::get_move_with_probs(const GameState& state) {
    if (!is_ready()) {
        throw std::runtime_error("Agent not ready - load a model first");
    }

    if (state.is_terminal()) {
        throw std::runtime_error("Cannot get move for terminal state");
    }

    auto probs = mcts_->search_with_probs(state);

    // Find best action
    int best_action = -1;
    float best_prob = -1.0f;
    for (const auto& [action, prob] : probs) {
        if (prob > best_prob) {
            best_prob = prob;
            best_action = action;
        }
    }

    return {best_action, probs};
}

std::string NeutronAgent::get_error_message() const {
    return error_message_;
}

}  // namespace neutron_rl
