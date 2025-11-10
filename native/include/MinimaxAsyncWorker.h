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

#pragma once
#include <napi.h>

#include <array>
#include <memory>

#include "FullMove.h"

class MinimaxAsyncWorker : public Napi::AsyncWorker {
   public:
    MinimaxAsyncWorker(Napi::Env env, std::array<uint8_t, 25> pboard, int pdepth, Napi::Promise::Deferred pdeferred)
        : Napi::AsyncWorker(env), inputBoard(pboard), depth(pdepth), deferred(std::move(pdeferred)) {
    }

    void Execute() override;  // hilo worker → llama a tu minimax existente
    void OnOK() override;     // resuelve promesa
    void OnError(const Napi::Error& e) override;

   private:
    std::array<uint8_t, 25> inputBoard;
    uint8_t depth;
    std::unique_ptr<FullMove> result;
    Napi::Promise::Deferred deferred;
};
