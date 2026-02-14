#pragma once

#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include <torch/script.h>
#include <torch/torch.h>

namespace neutron_rl {

/**
 * @brief Result of neural network inference.
 *
 * Contains policy logits (800 values for action probabilities)
 * and value estimate (single scalar for position evaluation).
 */
struct InferenceResult {
    std::vector<float> policy_logits;  // 800 action logits
    float value;                        // Position value estimate [-1, 1]
};

/**
 * @brief Loads and runs inference on TorchScript models.
 *
 * This class handles loading exported TorchScript models (.pt files)
 * and running forward inference for the Neutron game neural network.
 *
 * Example usage:
 * @code
 *     ModelLoader loader;
 *     loader.load("model.pt");
 *
 *     std::vector<float> board_tensor = encode_board(state);
 *     auto result = loader.infer(board_tensor);
 *     // result.policy_logits has 800 values
 *     // result.value is the position evaluation
 * @endcode
 */
class ModelLoader {
public:
    /**
     * @brief Construct a new Model Loader.
     *
     * @param device Device to run inference on ("cpu" or "cuda").
     *               Defaults to "cpu". If "cuda" is specified but not
     *               available, falls back to CPU.
     */
    explicit ModelLoader(const std::string& device = "cpu");

    /**
     * @brief Destroy the Model Loader.
     */
    ~ModelLoader() = default;

    // Non-copyable but movable
    ModelLoader(const ModelLoader&) = delete;
    ModelLoader& operator=(const ModelLoader&) = delete;
    ModelLoader(ModelLoader&&) = default;
    ModelLoader& operator=(ModelLoader&&) = default;

    /**
     * @brief Load a TorchScript model from file.
     *
     * @param model_path Path to the .pt TorchScript model file.
     * @return true if loading succeeded.
     * @return false if loading failed (check get_error_message()).
     */
    bool load(const std::string& model_path);

    /**
     * @brief Check if a model is loaded and ready.
     *
     * @return true if a model is loaded.
     * @return false if no model is loaded.
     */
    bool is_loaded() const;

    /**
     * @brief Run inference on a board state.
     *
     * @param board_tensor Encoded board state as a flat vector of 100 floats
     *                     (4 channels × 5 × 5 board).
     * @return InferenceResult with policy logits and value.
     * @throws std::runtime_error if no model is loaded.
     */
    InferenceResult infer(const std::vector<float>& board_tensor);

    /**
     * @brief Run batched inference on multiple board states.
     *
     * @param board_tensors Vector of encoded board states.
     * @return Vector of InferenceResults, one per input.
     * @throws std::runtime_error if no model is loaded.
     */
    std::vector<InferenceResult> infer_batch(
        const std::vector<std::vector<float>>& board_tensors);

    /**
     * @brief Get the last error message.
     *
     * @return Error message from the last failed operation.
     */
    std::string get_error_message() const;

    /**
     * @brief Get the current device string.
     *
     * @return Device string ("cpu" or "cuda:N").
     */
    std::string get_device() const;

    /**
     * @brief Check if CUDA is available.
     *
     * @return true if CUDA is available.
     */
    static bool cuda_available();

private:
    torch::jit::script::Module model_;
    torch::Device device_;
    bool loaded_ = false;
    std::string error_message_;

    // Expected tensor dimensions
    static constexpr int kInputChannels = 4;
    static constexpr int kBoardSize = 5;
    static constexpr int kActionSize = 800;
};

}  // namespace neutron_rl
