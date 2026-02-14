#include "neutron_rl/model_loader.hpp"

#include <stdexcept>

namespace neutron_rl {

ModelLoader::ModelLoader(const std::string& device)
    : device_(torch::kCPU) {
    if (device == "cuda" || device == "gpu") {
        if (torch::cuda::is_available()) {
            device_ = torch::kCUDA;
        } else {
            // Silently fall back to CPU if CUDA not available
        }
    }
}

bool ModelLoader::load(const std::string& model_path) {
    try {
        model_ = torch::jit::load(model_path, device_);
        model_.eval();
        loaded_ = true;
        error_message_.clear();
        return true;
    } catch (const c10::Error& e) {
        error_message_ = std::string("Failed to load model: ") + e.what();
        loaded_ = false;
        return false;
    } catch (const std::exception& e) {
        error_message_ = std::string("Failed to load model: ") + e.what();
        loaded_ = false;
        return false;
    }
}

bool ModelLoader::is_loaded() const {
    return loaded_;
}

InferenceResult ModelLoader::infer(const std::vector<float>& board_tensor) {
    if (!loaded_) {
        throw std::runtime_error("No model loaded");
    }

    // Validate input size
    const size_t expected_size = kInputChannels * kBoardSize * kBoardSize;
    if (board_tensor.size() != expected_size) {
        throw std::invalid_argument(
            "Invalid board tensor size: expected " + std::to_string(expected_size) +
            ", got " + std::to_string(board_tensor.size()));
    }

    // Create input tensor [1, 4, 5, 5]
    auto options = torch::TensorOptions().dtype(torch::kFloat32);
    torch::Tensor input = torch::from_blob(
        const_cast<float*>(board_tensor.data()),
        {1, kInputChannels, kBoardSize, kBoardSize},
        options
    ).clone().to(device_);

    // Run inference
    std::vector<torch::jit::IValue> inputs;
    inputs.push_back(input);

    torch::NoGradGuard no_grad;
    auto outputs = model_.forward(inputs);

    // Handle tuple output (policy, value)
    if (outputs.isTuple()) {
        auto tuple = outputs.toTuple();
        torch::Tensor policy_tensor = tuple->elements()[0].toTensor().cpu();
        torch::Tensor value_tensor = tuple->elements()[1].toTensor().cpu();

        InferenceResult result;
        result.policy_logits.resize(kActionSize);
        std::memcpy(result.policy_logits.data(),
                    policy_tensor.data_ptr<float>(),
                    kActionSize * sizeof(float));
        result.value = value_tensor.item<float>();
        return result;
    }

    throw std::runtime_error("Unexpected model output format");
}

std::vector<InferenceResult> ModelLoader::infer_batch(
    const std::vector<std::vector<float>>& board_tensors) {
    if (!loaded_) {
        throw std::runtime_error("No model loaded");
    }

    if (board_tensors.empty()) {
        return {};
    }

    const size_t batch_size = board_tensors.size();
    const size_t tensor_size = kInputChannels * kBoardSize * kBoardSize;

    // Validate and flatten input
    std::vector<float> flat_input;
    flat_input.reserve(batch_size * tensor_size);
    for (const auto& tensor : board_tensors) {
        if (tensor.size() != tensor_size) {
            throw std::invalid_argument("Invalid board tensor size in batch");
        }
        flat_input.insert(flat_input.end(), tensor.begin(), tensor.end());
    }

    // Create input tensor [batch, 4, 5, 5]
    auto options = torch::TensorOptions().dtype(torch::kFloat32);
    torch::Tensor input = torch::from_blob(
        flat_input.data(),
        {static_cast<int64_t>(batch_size), kInputChannels, kBoardSize, kBoardSize},
        options
    ).clone().to(device_);

    // Run inference
    std::vector<torch::jit::IValue> inputs;
    inputs.push_back(input);

    torch::NoGradGuard no_grad;
    auto outputs = model_.forward(inputs);

    // Handle tuple output (policy, value)
    if (outputs.isTuple()) {
        auto tuple = outputs.toTuple();
        torch::Tensor policy_tensor = tuple->elements()[0].toTensor().cpu();
        torch::Tensor value_tensor = tuple->elements()[1].toTensor().cpu();

        std::vector<InferenceResult> results;
        results.reserve(batch_size);

        for (size_t i = 0; i < batch_size; ++i) {
            InferenceResult result;
            result.policy_logits.resize(kActionSize);
            std::memcpy(result.policy_logits.data(),
                        policy_tensor[i].data_ptr<float>(),
                        kActionSize * sizeof(float));
            result.value = value_tensor[i].item<float>();
            results.push_back(std::move(result));
        }
        return results;
    }

    throw std::runtime_error("Unexpected model output format");
}

std::string ModelLoader::get_error_message() const {
    return error_message_;
}

std::string ModelLoader::get_device() const {
    if (device_.is_cuda()) {
        return "cuda:" + std::to_string(device_.index());
    }
    return "cpu";
}

bool ModelLoader::cuda_available() {
    return torch::cuda::is_available();
}

}  // namespace neutron_rl
