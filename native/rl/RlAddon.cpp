#include <napi.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>

#include "RlAsyncWorker.h"

std::unique_ptr<neutron_rl::NeutronAgent> g_agent;
std::mutex g_agent_mutex;

namespace {

class RlLoadModelWorker : public Napi::AsyncWorker {
   public:
    RlLoadModelWorker(Napi::Env env, std::string pmodelPath, Napi::Promise::Deferred pdeferred)
        : Napi::AsyncWorker(env), modelPath(std::move(pmodelPath)), deferred(std::move(pdeferred)) {
    }

    void Execute() override {
        try {
            std::lock_guard<std::mutex> lock(g_agent_mutex);
            if (!g_agent) {
                g_agent = std::make_unique<neutron_rl::NeutronAgent>("cpu");
            }

            if (!g_agent->load_model(modelPath)) {
                throw std::runtime_error("Failed to load RL model: " + g_agent->get_error_message());
            }
        } catch (const std::exception& ex) {
            SetError(ex.what());
        } catch (...) {
            SetError("Unknown error loading RL model");
        }
    }

    void OnOK() override {
        deferred.Resolve(Env().Undefined());
    }

    void OnError(const Napi::Error& e) override {
        deferred.Reject(e.Value());
    }

   private:
    std::string modelPath;
    Napi::Promise::Deferred deferred;
};

Napi::Value LoadModel(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsString()) {
        throw Napi::TypeError::New(env, "loadModel(path) expects a model path string");
    }

    const auto modelPath = info[0].As<Napi::String>().Utf8Value();
    auto deferred = Napi::Promise::Deferred::New(env);
    (new RlLoadModelWorker(env, modelPath, deferred))->Queue();
    return deferred.Promise();
}

Napi::Value MoveAsync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsObject()) {
        throw Napi::TypeError::New(env, "moveAsync(input) expects {board, difficulty}");
    }

    const auto input = info[0].As<Napi::Object>();
    if (!input.Has("board") || !input.Get("board").IsTypedArray()) {
        throw Napi::TypeError::New(env, "moveAsync expects board: Uint8Array(25)");
    }

    auto inputBoard = input.Get("board").As<Napi::TypedArrayOf<uint8_t>>();
    if (inputBoard.ElementLength() != 25) {
        throw Napi::TypeError::New(env, "moveAsync expects board length of 25");
    }

    std::array<uint8_t, 25> board{};
    std::memcpy(board.data(), inputBoard.Data(), 25 * sizeof(uint8_t));

    std::string difficulty = "hard";
    if (input.Has("difficulty") && input.Get("difficulty").IsString()) {
        difficulty = input.Get("difficulty").As<Napi::String>().Utf8Value();
    }

    auto deferred = Napi::Promise::Deferred::New(env);
    (new RlAsyncWorker(env, board, difficulty, deferred))->Queue();
    return deferred.Promise();
}

}  // namespace

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("loadModel", Napi::Function::New(env, LoadModel));
    exports.Set("moveAsync", Napi::Function::New(env, MoveAsync));
    return exports;
}

NODE_API_MODULE(neutron_rl_addon, Init)
