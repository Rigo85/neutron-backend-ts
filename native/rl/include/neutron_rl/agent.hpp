#pragma once

#include <array>
#include <memory>
#include <string>

#include "neutron_rl/game_state.hpp"
#include "neutron_rl/mcts.hpp"
#include "neutron_rl/model_loader.hpp"

namespace neutron_rl {

/**
 * @brief Difficulty presets for the AI agent.
 */
enum class Difficulty {
    Easy,    // ~100 MCTS simulations, some randomness
    Medium,  // ~300 MCTS simulations
    Hard     // ~800 MCTS simulations, deterministic
};

/**
 * @brief Configuration for a difficulty level.
 */
struct DifficultyConfig {
    int simulations;
    float temperature;

    static DifficultyConfig from_preset(Difficulty difficulty);
    static DifficultyConfig from_simulations(int simulations, float temperature = 0.0f);
};

/**
 * @brief High-level AI agent for the Neutron game.
 *
 * This class provides a simple interface for game engine integration.
 * It handles model loading, MCTS search, and difficulty configuration.
 *
 * Example usage:
 * @code
 *     NeutronAgent agent;
 *     if (!agent.load_model("model.pt")) {
 *         std::cerr << "Failed to load model" << std::endl;
 *         return;
 *     }
 *     agent.set_difficulty(Difficulty::Hard);
 *
 *     std::array<int8_t, 25> board = get_current_board();
 *     int player = get_current_player();
 *     Phase phase = get_current_phase();
 *
 *     int action = agent.get_move(board, player, phase);
 *     // Apply action to game...
 * @endcode
 */
class NeutronAgent {
public:
    /**
     * @brief Construct a new Neutron Agent.
     *
     * @param device Device for inference ("cpu" or "cuda").
     */
    explicit NeutronAgent(const std::string& device = "cpu");

    /**
     * @brief Destroy the Neutron Agent.
     */
    ~NeutronAgent();

    // Non-copyable
    NeutronAgent(const NeutronAgent&) = delete;
    NeutronAgent& operator=(const NeutronAgent&) = delete;

    // Movable
    NeutronAgent(NeutronAgent&&) noexcept;
    NeutronAgent& operator=(NeutronAgent&&) noexcept;

    /**
     * @brief Load a TorchScript model.
     *
     * @param model_path Path to the .pt model file.
     * @return true if loading succeeded.
     */
    bool load_model(const std::string& model_path);

    /**
     * @brief Check if a model is loaded.
     *
     * @return true if ready to play.
     */
    bool is_ready() const;

    /**
     * @brief Set difficulty by preset.
     *
     * @param difficulty Difficulty preset.
     */
    void set_difficulty(Difficulty difficulty);

    /**
     * @brief Set difficulty by simulation count.
     *
     * @param simulations Number of MCTS simulations.
     * @param temperature Temperature for action selection (0 = deterministic).
     */
    void set_difficulty(int simulations, float temperature = 0.0f);

    /**
     * @brief Set difficulty by name string.
     *
     * @param difficulty_name "easy", "medium", or "hard".
     * @return true if the name was recognized.
     */
    bool set_difficulty(const std::string& difficulty_name);

    /**
     * @brief Get the current difficulty configuration.
     *
     * @return Current DifficultyConfig.
     */
    DifficultyConfig get_difficulty_config() const;

    /**
     * @brief Get the best move for a board state.
     *
     * @param board 25-element array of piece values.
     * @param current_player Current player (1 or 2).
     * @param phase Current phase (MoveNeutron or MovePawn).
     * @return Action index in the 800-action space.
     * @throws std::runtime_error if no model is loaded.
     */
    int get_move(const std::array<int8_t, 25>& board,
                 int current_player,
                 Phase phase);

    /**
     * @brief Get the best move for a game state.
     *
     * @param state Current game state.
     * @return Action index in the 800-action space.
     * @throws std::runtime_error if no model is loaded.
     */
    int get_move(const GameState& state);

    /**
     * @brief Get move with action probabilities.
     *
     * @param state Current game state.
     * @return Pair of (action, probabilities) where probabilities is a
     *         vector of (action, prob) pairs for all legal actions.
     */
    std::pair<int, std::vector<std::pair<int, float>>>
    get_move_with_probs(const GameState& state);

    /**
     * @brief Get the last error message.
     *
     * @return Error message string.
     */
    std::string get_error_message() const;

private:
    std::unique_ptr<ModelLoader> model_loader_;
    std::unique_ptr<MCTS> mcts_;
    DifficultyConfig difficulty_config_;
    std::string error_message_;
};

}  // namespace neutron_rl
