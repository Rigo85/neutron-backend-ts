#pragma once

#include <napi.h>

#include <array>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "neutron_rl/agent.hpp"

struct RlMove {
    int row;
    int col;
    int kind;
};

class RlAsyncWorker : public Napi::AsyncWorker {
   public:
    RlAsyncWorker(Napi::Env env,
                  std::array<uint8_t, 25> pboard,
                  std::string pdifficulty,
                  Napi::Promise::Deferred pdeferred)
        : Napi::AsyncWorker(env),
          inputBoard(pboard),
          difficultyName(std::move(pdifficulty)),
          deferred(std::move(pdeferred)) {
    }

    void Execute() override;
    void OnOK() override;
    void OnError(const Napi::Error& e) override;

   private:
    std::array<uint8_t, 25> inputBoard;
    std::string difficultyName;
    std::vector<RlMove> resultMoves;
    double score = 0.0;
    Napi::Promise::Deferred deferred;
};

extern std::unique_ptr<neutron_rl::NeutronAgent> g_agent;
extern std::mutex g_agent_mutex;
