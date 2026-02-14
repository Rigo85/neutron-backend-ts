#include "neutron_rl/game_state.hpp"

#include <sstream>
#include <stdexcept>

namespace neutron_rl {

GameState::GameState() : current_player_(1), phase_(Phase::MoveNeutron) {
    // Initial board setup:
    // Row 0: Player 2 pawns (top)
    // Row 2: Neutron (center)
    // Row 4: Player 1 pawns (bottom)
    board_.fill(static_cast<int8_t>(Piece::Empty));

    // Player 2 pawns on top row
    for (int col = 0; col < kBoardSize; ++col) {
        board_[col] = static_cast<int8_t>(Piece::Player2Pawn);
    }

    // Neutron in center
    board_[12] = static_cast<int8_t>(Piece::Neutron);

    // Player 1 pawns on bottom row
    for (int col = 0; col < kBoardSize; ++col) {
        board_[20 + col] = static_cast<int8_t>(Piece::Player1Pawn);
    }
}

GameState::GameState(const std::array<int8_t, kNumCells>& board,
                     int current_player,
                     Phase phase)
    : board_(board), current_player_(current_player), phase_(phase) {}

std::pair<int, int> GameState::cell_to_rowcol(int cell) {
    return {cell / kBoardSize, cell % kBoardSize};
}

int GameState::rowcol_to_cell(int row, int col) {
    return row * kBoardSize + col;
}

std::tuple<int, int, int> GameState::decode_action(int action) {
    int cell = action / 32;
    int remainder = action % 32;
    int direction = remainder / 4;
    int distance = (remainder % 4) + 1;
    return {cell, direction, distance};
}

int GameState::encode_action(int cell, int direction, int distance) {
    return cell * 32 + direction * 4 + (distance - 1);
}

Piece GameState::get_piece(int cell) const {
    return static_cast<Piece>(board_[cell]);
}

int GameState::find_neutron() const {
    for (int i = 0; i < kNumCells; ++i) {
        if (board_[i] == static_cast<int8_t>(Piece::Neutron)) {
            return i;
        }
    }
    return -1;  // Should never happen in valid game
}

int GameState::max_slide_distance(int cell, int direction) const {
    auto [row, col] = cell_to_rowcol(cell);
    auto [dr, dc] = kDirectionDeltas[direction];

    int distance = 0;
    int new_row = row + dr;
    int new_col = col + dc;

    while (new_row >= 0 && new_row < kBoardSize &&
           new_col >= 0 && new_col < kBoardSize) {
        int new_cell = rowcol_to_cell(new_row, new_col);
        if (board_[new_cell] != static_cast<int8_t>(Piece::Empty)) {
            break;
        }
        distance++;
        new_row += dr;
        new_col += dc;
    }

    return distance;
}

std::vector<std::pair<int, int>> GameState::get_piece_moves(int cell) const {
    std::vector<std::pair<int, int>> moves;

    for (int dir = 0; dir < kNumDirections; ++dir) {
        int max_dist = max_slide_distance(cell, dir);
        for (int dist = 1; dist <= max_dist; ++dist) {
            moves.emplace_back(dir, dist);
        }
    }

    return moves;
}

std::vector<int> GameState::get_legal_actions() const {
    std::vector<int> actions;

    if (phase_ == Phase::MoveNeutron) {
        // Must move the neutron — full slide only (standard Neutron rules)
        int neutron_cell = find_neutron();
        for (int dir = 0; dir < kNumDirections; ++dir) {
            int max_dist = max_slide_distance(neutron_cell, dir);
            if (max_dist > 0) {
                actions.push_back(encode_action(neutron_cell, dir, max_dist));
            }
        }
    } else {
        // Must move one of current player's pawns — full slide only
        Piece my_pawn = (current_player_ == 1) ? Piece::Player1Pawn : Piece::Player2Pawn;

        for (int cell = 0; cell < kNumCells; ++cell) {
            if (board_[cell] == static_cast<int8_t>(my_pawn)) {
                for (int dir = 0; dir < kNumDirections; ++dir) {
                    int max_dist = max_slide_distance(cell, dir);
                    if (max_dist > 0) {
                        actions.push_back(encode_action(cell, dir, max_dist));
                    }
                }
            }
        }
    }

    return actions;
}

GameState GameState::apply_action(int action) const {
    auto [cell, direction, distance] = decode_action(action);

    // Validate the move
    auto [row, col] = cell_to_rowcol(cell);
    auto [dr, dc] = kDirectionDeltas[direction];

    int new_row = row + dr * distance;
    int new_col = col + dc * distance;

    if (new_row < 0 || new_row >= kBoardSize ||
        new_col < 0 || new_col >= kBoardSize) {
        throw std::invalid_argument("Action moves piece off board");
    }

    int new_cell = rowcol_to_cell(new_row, new_col);

    // Create new state
    GameState new_state = *this;
    new_state.board_[new_cell] = new_state.board_[cell];
    new_state.board_[cell] = static_cast<int8_t>(Piece::Empty);

    // Update phase and player
    if (phase_ == Phase::MoveNeutron) {
        new_state.phase_ = Phase::MovePawn;
    } else {
        new_state.phase_ = Phase::MoveNeutron;
        new_state.current_player_ = (current_player_ == 1) ? 2 : 1;
    }

    return new_state;
}

bool GameState::is_terminal() const {
    int neutron_cell = find_neutron();
    if (neutron_cell < 0) return true;  // Invalid state

    int row = neutron_cell / kBoardSize;

    // Neutron on player 1's home row (row 4)
    if (row == kBoardSize - 1) return true;

    // Neutron on player 2's home row (row 0)
    if (row == 0) return true;

    // Check if current player can move
    auto legal = get_legal_actions();
    return legal.empty();
}

std::optional<int> GameState::get_winner() const {
    if (!is_terminal()) {
        return std::nullopt;
    }

    int neutron_cell = find_neutron();
    if (neutron_cell < 0) {
        return std::nullopt;  // Invalid state
    }

    int row = neutron_cell / kBoardSize;

    // Neutron on player 2's home row (row 0) - player 2 wins
    if (row == 0) {
        return 2;
    }

    // Neutron on player 1's home row (row 4) - player 1 wins
    if (row == kBoardSize - 1) {
        return 1;
    }

    // Current player couldn't move - they lose
    return (current_player_ == 1) ? 2 : 1;
}

std::vector<float> GameState::encode() const {
    std::vector<float> tensor(4 * kBoardSize * kBoardSize, 0.0f);

    Piece my_pawn = (current_player_ == 1) ? Piece::Player1Pawn : Piece::Player2Pawn;
    Piece opp_pawn = (current_player_ == 1) ? Piece::Player2Pawn : Piece::Player1Pawn;

    for (int cell = 0; cell < kNumCells; ++cell) {
        int row = cell / kBoardSize;
        int col = cell % kBoardSize;
        Piece piece = get_piece(cell);

        // Flip row for P2 so both players see their home row at row 4
        int encoded_row = (current_player_ == 1) ? row : (kBoardSize - 1 - row);
        int encoded_cell = encoded_row * kBoardSize + col;

        // Channel 0: My pawns
        if (piece == my_pawn) {
            tensor[encoded_cell] = 1.0f;
        }
        // Channel 1: Opponent pawns
        if (piece == opp_pawn) {
            tensor[kNumCells + encoded_cell] = 1.0f;
        }
        // Channel 2: Neutron
        if (piece == Piece::Neutron) {
            tensor[2 * kNumCells + encoded_cell] = 1.0f;
        }
    }

    // Channel 3: Phase indicator (all 1s if neutron phase, all 0s if pawn phase)
    if (phase_ == Phase::MoveNeutron) {
        for (int i = 0; i < kNumCells; ++i) {
            tensor[3 * kNumCells + i] = 1.0f;
        }
    }

    return tensor;
}

int GameState::flip_action(int action) {
    auto [cell, direction, distance] = decode_action(action);
    auto [row, col] = cell_to_rowcol(cell);

    // Flip row: row -> (4 - row)
    int flipped_cell = rowcol_to_cell(kBoardSize - 1 - row, col);

    // Flip direction: N<->S, NE<->SE, NW<->SW, E and W unchanged
    // Original: N=0 NE=1 E=2 SE=3 S=4 SW=5 W=6 NW=7
    // Flipped:  S=4 SE=3 E=2 NE=1 N=0 NW=7 W=6 SW=5
    static constexpr int kDirectionFlip[8] = {4, 3, 2, 1, 0, 7, 6, 5};
    int flipped_direction = kDirectionFlip[direction];

    return encode_action(flipped_cell, flipped_direction, distance);
}

std::vector<float> GameState::flip_policy(const std::vector<float>& policy) {
    std::vector<float> flipped(kActionSize, 0.0f);
    for (int a = 0; a < kActionSize; ++a) {
        flipped[flip_action(a)] = policy[a];
    }
    return flipped;
}

std::string GameState::to_string() const {
    std::ostringstream oss;
    oss << "  0 1 2 3 4\n";
    for (int row = 0; row < kBoardSize; ++row) {
        oss << row << " ";
        for (int col = 0; col < kBoardSize; ++col) {
            int cell = rowcol_to_cell(row, col);
            char c = '.';
            switch (get_piece(cell)) {
                case Piece::Player1Pawn: c = '1'; break;
                case Piece::Player2Pawn: c = '2'; break;
                case Piece::Neutron: c = 'N'; break;
                default: break;
            }
            oss << c << ' ';
        }
        oss << '\n';
    }
    oss << "Player " << current_player_ << " to move";
    if (phase_ == Phase::MoveNeutron) {
        oss << " (neutron)";
    } else {
        oss << " (pawn)";
    }
    return oss.str();
}

}  // namespace neutron_rl
