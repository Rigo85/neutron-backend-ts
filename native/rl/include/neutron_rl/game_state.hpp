#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace neutron_rl {

/**
 * @brief Piece types on the Neutron board.
 */
enum class Piece : int8_t {
    Empty = 0,
    Player1Pawn = 1,
    Player2Pawn = 2,
    Neutron = 3
};

/**
 * @brief Direction indices for piece movement.
 *
 * Pieces slide in 8 directions until hitting another piece or board edge.
 */
enum class Direction : int8_t {
    North = 0,
    NorthEast = 1,
    East = 2,
    SouthEast = 3,
    South = 4,
    SouthWest = 5,
    West = 6,
    NorthWest = 7
};

/**
 * @brief Game phase - determines which piece type moves.
 */
enum class Phase : int8_t {
    MoveNeutron = 0,  // Player must move the neutron
    MovePawn = 1      // Player must move one of their pawns
};

/**
 * @brief Neutron game state representation.
 *
 * The Neutron board is a 5×5 grid. Each player has 5 pawns starting on
 * opposite sides. The neutron starts in the center. Players alternate
 * moving the neutron then one of their pawns. Win by getting the neutron
 * to your home row.
 */
class GameState {
public:
    static constexpr int kBoardSize = 5;
    static constexpr int kNumCells = 25;
    static constexpr int kNumDirections = 8;
    static constexpr int kMaxDistance = 4;
    static constexpr int kActionSize = 800;  // 25 cells × 8 dirs × 4 distances

    /**
     * @brief Construct the initial game state.
     */
    GameState();

    /**
     * @brief Construct from a board array.
     *
     * @param board 25-element array of piece values (0-3).
     * @param current_player Current player (1 or 2).
     * @param phase Current phase (move neutron or pawn).
     */
    GameState(const std::array<int8_t, kNumCells>& board,
              int current_player,
              Phase phase);

    /**
     * @brief Get all legal actions for the current state.
     *
     * @return Vector of action indices in the 800-action space.
     */
    std::vector<int> get_legal_actions() const;

    /**
     * @brief Apply an action and return the resulting state.
     *
     * @param action Action index in the 800-action space.
     * @return New game state after the action.
     * @throws std::invalid_argument if action is illegal.
     */
    GameState apply_action(int action) const;

    /**
     * @brief Check if the game is in a terminal state.
     *
     * @return true if the game has ended.
     */
    bool is_terminal() const;

    /**
     * @brief Get the winner of a terminal state.
     *
     * @return 1 or 2 for the winning player, std::nullopt if not terminal or draw.
     */
    std::optional<int> get_winner() const;

    /**
     * @brief Get the current player.
     *
     * @return 1 or 2.
     */
    int current_player() const { return current_player_; }

    /**
     * @brief Get the current phase.
     *
     * @return Phase::MoveNeutron or Phase::MovePawn.
     */
    Phase phase() const { return phase_; }

    /**
     * @brief Encode the board state for neural network input.
     *
     * Returns a 100-element vector (4 channels × 5 × 5):
     * - Channel 0: Current player's pawns (1 where present)
     * - Channel 1: Opponent's pawns (1 where present)
     * - Channel 2: Neutron position (1 where present)
     * - Channel 3: Phase indicator (all 1s if neutron phase, all 0s if pawn phase)
     *
     * @return Encoded tensor as flat vector.
     */
    std::vector<float> encode() const;

    /**
     * @brief Get the raw board array.
     *
     * @return Reference to the 25-element board array.
     */
    const std::array<int8_t, kNumCells>& board() const { return board_; }

    /**
     * @brief Get piece at a cell.
     *
     * @param cell Cell index (0-24).
     * @return Piece at the cell.
     */
    Piece get_piece(int cell) const;

    /**
     * @brief Convert cell index to row/column.
     *
     * @param cell Cell index (0-24).
     * @return Pair of (row, col).
     */
    static std::pair<int, int> cell_to_rowcol(int cell);

    /**
     * @brief Convert row/column to cell index.
     *
     * @param row Row (0-4).
     * @param col Column (0-4).
     * @return Cell index (0-24).
     */
    static int rowcol_to_cell(int row, int col);

    /**
     * @brief Decode an action index.
     *
     * @param action Action index (0-799).
     * @return Tuple of (cell, direction, distance).
     */
    static std::tuple<int, int, int> decode_action(int action);

    /**
     * @brief Encode an action.
     *
     * @param cell Starting cell (0-24).
     * @param direction Direction index (0-7).
     * @param distance Distance (1-4).
     * @return Action index (0-799).
     */
    static int encode_action(int cell, int direction, int distance);

    /**
     * @brief Flip an action for vertical board transformation (P2 perspective).
     *
     * @param action Action index (0-799).
     * @return Flipped action index (0-799).
     */
    static int flip_action(int action);

    /**
     * @brief Flip a policy vector for vertical board transformation.
     *
     * @param policy 800-element policy vector.
     * @return Flipped 800-element policy vector.
     */
    static std::vector<float> flip_policy(const std::vector<float>& policy);

    /**
     * @brief Get string representation of the board.
     *
     * @return Multi-line string showing the board.
     */
    std::string to_string() const;

private:
    std::array<int8_t, kNumCells> board_;
    int current_player_;
    Phase phase_;

    // Direction deltas (row, col) for each direction
    static constexpr std::array<std::pair<int, int>, kNumDirections> kDirectionDeltas = {{
        {-1, 0},   // North
        {-1, 1},   // NorthEast
        {0, 1},    // East
        {1, 1},    // SouthEast
        {1, 0},    // South
        {1, -1},   // SouthWest
        {0, -1},   // West
        {-1, -1}   // NorthWest
    }};

    /**
     * @brief Find the neutron's position.
     *
     * @return Cell index of the neutron.
     */
    int find_neutron() const;

    /**
     * @brief Get legal moves for a specific piece.
     *
     * @param cell Cell containing the piece.
     * @return Vector of (direction, distance) pairs.
     */
    std::vector<std::pair<int, int>> get_piece_moves(int cell) const;

    /**
     * @brief Calculate maximum slide distance in a direction.
     *
     * @param cell Starting cell.
     * @param direction Direction index.
     * @return Maximum distance (0 if blocked immediately).
     */
    int max_slide_distance(int cell, int direction) const;
};

}  // namespace neutron_rl
