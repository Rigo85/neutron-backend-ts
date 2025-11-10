/**
 * Authors:
 * Rigoberto Leander Salgado Reyes <rlsalgado2006@gmail.com>
 *
 * Copyright 2018 by Rigoberto Leander Salgado Reyes.
 *
 * This program is licensed to you under the terms of version 3 of the
 * GNU Affero General Public License. This program is distributed WITHOUT
 * ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THOSE OF NON-INFRINGEMENT,
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. Please refer to the
 * AGPL (http://www.gnu.org/licenses/agpl-3.0.txt) for more details.
 *
 * Modified 2025 by Rigoberto Leander Salgado Reyes.
 */

#include <Board.h>
#include <FullMove.h>
#include <MinimaxAsyncWorker.h>
#include <Move.h>
#include <PieceKind.h>
#include <cleaners.h>
#include <gameutils.h>
#include <minimax.h>
#include <napi.h>

#include <limits>

void MinimaxAsyncWorker::Execute() {
    try {
        auto board = std::make_unique<Board>(inputBoard);
        constexpr int alpha = std::numeric_limits<int>::min();
        constexpr int beta = std::numeric_limits<int>::max();

        const auto fm = maxValue(board, depth, alpha, beta, PieceKind::BLACK);

        if (!fm)
            throw new std::runtime_error("no 'fullmove' returned from minimax");

        result = std::make_unique<FullMove>(*fm);
    } catch (const std::exception& ex) {
        SetError(ex.what());
    } catch (...) {
        SetError("Unknown error in MinimaxAsyncWorker");
    }
}

void MinimaxAsyncWorker::OnOK() {
    Napi::Env env = Env();

    Napi::Object out = Napi::Object::New(env);
    Napi::Array moves = Napi::Array::New(env);

    int i = 0;
    for (const auto& move : result->moves) {
        auto jm = Napi::Object::New(env);
        jm.Set("row", Napi::Number::New(env, move->row));
        jm.Set("col", Napi::Number::New(env, move->col));
        jm.Set("kind", Napi::Number::New(env, static_cast<int>(move->kind)));
        moves.Set(i++, jm);
    }

    out.Set("moves", moves);
    out.Set("score", Napi::Number::New(env, result->score));

    deferred.Resolve(out);
}

void MinimaxAsyncWorker::OnError(const Napi::Error& e) {
    deferred.Reject(e.Value());
}
