/**
 * Authors:
 * Rigoberto Leander Salgado Reyes <rlsalgado2006@gmail.com>
 *
 * Copyright 2025 by Rigoberto Leander Salgado Reyes.
 *
 * This program is licensed to you under the terms of version 3 of the
 * GNU Affero General Public License. This program is distributed WITHOUT
 * ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THOSE OF NON-INFRINGEMENT,
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. Please refer to the
 * AGPL (http://www.gnu.org/licenses/agpl-3.0.txt) for more details.
 */

#include <napi.h>

#include <array>
#include <cstdint>
#include <cstring>

#include "MinimaxAsyncWorker.h"

using namespace Napi;

// JS signature: minimaxAsync(input: string | Buffer): Promise<string>
Value MinimaxAsync(const CallbackInfo& info) {
    Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsObject()) {
        throw TypeError::New(env, "minimaxAsync(input) expects {board, depth}");
    }

    auto input = info[0].As<Object>();

    auto inputBoard = input.Get("board").As<TypedArrayOf<uint8_t>>();
    std::array<uint8_t, 25> board{};
    std::memcpy(board.data(), inputBoard.Data(), 25 * sizeof(uint8_t));

    int depth = input.Get("depth").As<Number>().Uint32Value();

    auto deferred = Promise::Deferred::New(env);
    (new MinimaxAsyncWorker(env, board, depth, deferred))->Queue();
    return deferred.Promise();
}

Object Init(Env env, Object exports) {
    exports.Set("minimaxAsync", Function::New(env, MinimaxAsync));
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
