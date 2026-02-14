#include "RlAsyncWorker.h"

#include <napi.h>

#include <array>
#include <stdexcept>
#include <utility>

namespace {

constexpr int kBoardSize = 5;
constexpr int kCellValue = 4;
constexpr int kBlackValue = 1;
constexpr int kWhiteValue = 2;

int8_t to_rl_piece(int8_t backend_piece) {
    // Backend: BLACK=1, WHITE=2, NEUTRON=3, CELL=4.
    // RL: Player1=1 (home row 4), Player2=2 (home row 0), Neutron=3, Empty=0.
    // In backend, BLACK starts on row 0, so BLACK maps to RL Player2.
    if (backend_piece == kCellValue) return 0;
    if (backend_piece == kBlackValue) return 2;
    if (backend_piece == kWhiteValue) return 1;
    return backend_piece;
}

std::array<int8_t, 25> to_rl_board(const std::array<uint8_t, 25>& js_board) {
    std::array<int8_t, 25> rl_board{};

    for (int r = 0; r < kBoardSize; ++r) {
        for (int c = 0; c < kBoardSize; ++c) {
            int8_t val = static_cast<int8_t>(js_board[c * kBoardSize + r]);
            rl_board[r * kBoardSize + c] = to_rl_piece(val);
        }
    }

    return rl_board;
}

RlMove make_move(int row, int col, int kind) {
    return RlMove{row, col, kind};
}

void append_action_moves(int action, int piece_kind, std::vector<RlMove>& out) {
    auto [cell, direction, distance] = neutron_rl::GameState::decode_action(action);
    auto [from_row, from_col] = neutron_rl::GameState::cell_to_rowcol(cell);

    static constexpr std::array<std::pair<int, int>, 8> kDirectionDeltas = {{
        {-1, 0}, {-1, 1}, {0, 1}, {1, 1},
        {1, 0},  {1, -1}, {0, -1}, {-1, -1}
    }};

    const auto [dr, dc] = kDirectionDeltas.at(direction);
    const int to_row = from_row + dr * distance;
    const int to_col = from_col + dc * distance;

    out.push_back(make_move(from_row, from_col, piece_kind));
    out.push_back(make_move(to_row, to_col, piece_kind));
}

RlMove fallback_black_pawn_move(const std::array<int8_t, 25>& board) {
    for (int cell = 0; cell < static_cast<int>(board.size()); ++cell) {
        if (board[cell] == 2) {
            auto [row, col] = neutron_rl::GameState::cell_to_rowcol(cell);
            return make_move(row, col, 1);
        }
    }

    return make_move(4, 0, 1);
}

}  // namespace

void RlAsyncWorker::Execute() {
    try {
        const auto rl_board = to_rl_board(inputBoard);

        std::lock_guard<std::mutex> lock(g_agent_mutex);

        if (!g_agent || !g_agent->is_ready()) {
            throw std::runtime_error("RL model not loaded");
        }

        if (!g_agent->set_difficulty(difficultyName)) {
            throw std::runtime_error("Invalid RL difficulty: " + difficultyName);
        }

        neutron_rl::GameState state(rl_board, 2, neutron_rl::Phase::MoveNeutron);

        const int neutron_action = g_agent->get_move(state);
        append_action_moves(neutron_action, 3, resultMoves);

        state = state.apply_action(neutron_action);

        if (state.is_terminal()) {
            const auto fallback = fallback_black_pawn_move(state.board());
            resultMoves.push_back(fallback);
            resultMoves.push_back(fallback);
            score = 1.0;
            return;
        }

        const int pawn_action = g_agent->get_move(state);
        append_action_moves(pawn_action, 1, resultMoves);

        score = 1.0;
    } catch (const std::exception& ex) {
        SetError(ex.what());
    } catch (...) {
        SetError("Unknown error in RlAsyncWorker");
    }
}

void RlAsyncWorker::OnOK() {
    Napi::Env env = Env();

    Napi::Object out = Napi::Object::New(env);
    Napi::Array moves = Napi::Array::New(env);

    for (uint32_t i = 0; i < resultMoves.size(); ++i) {
        auto jm = Napi::Object::New(env);
        jm.Set("row", Napi::Number::New(env, resultMoves[i].row));
        jm.Set("col", Napi::Number::New(env, resultMoves[i].col));
        jm.Set("kind", Napi::Number::New(env, resultMoves[i].kind));
        moves.Set(i, jm);
    }

    out.Set("moves", moves);
    out.Set("score", Napi::Number::New(env, score));

    deferred.Resolve(out);
}

void RlAsyncWorker::OnError(const Napi::Error& e) {
    deferred.Reject(e.Value());
}
