/*
 * Copyright (c) 2017-2018 ARM Limited.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "arm_compute/graph.h"
#include "support/ToolchainSupport.h"
#include "utils/CommonGraphOptions.h"
#include "utils/GraphUtils.h"
#include "utils/Utils.h"

using namespace arm_compute::utils;
using namespace arm_compute::graph::frontend;
using namespace arm_compute::graph_utils;
/** Example demonstrating how to implement VGG19's network using the Compute Library's graph API */
class GraphVGG19Example : public Example
{
public:
    GraphVGG19Example()
        : cmd_parser(), common_opts(cmd_parser), common_params(), graph(0, "VGG19")
    {
    }
    bool do_setup(int argc, char **argv) override
    {
        // Check if the system has enough RAM to run the example, systems with less than 2GB have
        // to hint the API to minimize memory consumption otherwise it'll run out of memory and
        // fail throwing the bad_alloc exception
        arm_compute::MEMInfo meminfo;
        const size_t         mem_total = meminfo.get_total_in_kb();
        if(mem_total <= arm_compute::MEMInfo::TWO_GB_IN_KB)
        {
            arm_compute::MEMInfo::set_policy(arm_compute::MemoryPolicy::MINIMIZE);
        }

        // Parse arguments
        cmd_parser.parse(argc, argv);

        // Consume common parameters
        common_params = consume_common_graph_parameters(common_opts);

        // Return when help menu is requested
        if(common_params.help)
        {
            cmd_parser.print_help(argv[0]);
            return false;
        }

        // Checks
        ARM_COMPUTE_EXIT_ON_MSG(arm_compute::is_data_type_quantized_asymmetric(common_params.data_type), "QASYMM8 not supported for this graph");

        // Print parameter values
        std::cout << common_params << std::endl;

        // Get trainable parameters data path
        std::string data_path = common_params.data_path;

        // Create a preprocessor object
        const std::array<float, 3> mean_rgb{ { 123.68f, 116.779f, 103.939f } };
        std::unique_ptr<IPreprocessor> preprocessor = arm_compute::support::cpp14::make_unique<CaffePreproccessor>(mean_rgb);

        // Create input descriptor
        const TensorShape tensor_shape     = permute_shape(TensorShape(224U, 224U, 3U, 1U), DataLayout::NCHW, common_params.data_layout);
        TensorDescriptor  input_descriptor = TensorDescriptor(tensor_shape, common_params.data_type).set_layout(common_params.data_layout);

        // Set weights trained layout
        const DataLayout weights_layout = DataLayout::NCHW;

        graph << common_params.target
              << common_params.fast_math_hint
              << InputLayer(input_descriptor, get_input_accessor(common_params, std::move(preprocessor)))
              // Layer 1
              << ConvolutionLayer(
                  3U, 3U, 64U,
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv1_1_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv1_1_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv1_1")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv1_1/Relu")
              << ConvolutionLayer(
                  3U, 3U, 64U,
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv1_2_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv1_2_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv1_2")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv1_2/Relu")
              << PoolingLayer(PoolingLayerInfo(PoolingType::MAX, 2, PadStrideInfo(2, 2, 0, 0))).set_name("pool1")
              // Layer 2
              << ConvolutionLayer(
                  3U, 3U, 128U,
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv2_1_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv2_1_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv2_1")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv2_1/Relu")
              << ConvolutionLayer(
                  3U, 3U, 128U,
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv2_2_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv2_2_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv2_2")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv2_2/Relu")
              << PoolingLayer(PoolingLayerInfo(PoolingType::MAX, 2, PadStrideInfo(2, 2, 0, 0))).set_name("pool2")
              // Layer 3
              << ConvolutionLayer(
                  3U, 3U, 256U,
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv3_1_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv3_1_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv3_1")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv3_1/Relu")
              << ConvolutionLayer(
                  3U, 3U, 256U,
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv3_2_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv3_2_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv3_2")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv3_2/Relu")
              << ConvolutionLayer(
                  3U, 3U, 256U,
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv3_3_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv3_3_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv3_3")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv3_3/Relu")
              << ConvolutionLayer(
                  3U, 3U, 256U,
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv3_4_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv3_4_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv3_4")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv3_4/Relu")
              << PoolingLayer(PoolingLayerInfo(PoolingType::MAX, 2, PadStrideInfo(2, 2, 0, 0))).set_name("pool3")
              // Layer 4
              << ConvolutionLayer(
                  3U, 3U, 512U,
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv4_1_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv4_1_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv4_1")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv4_1/Relu")
              << ConvolutionLayer(
                  3U, 3U, 512U,
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv4_2_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv4_2_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv4_2")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv4_2/Relu")
              << ConvolutionLayer(
                  3U, 3U, 512U,
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv4_3_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv4_3_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv4_3")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv4_3/Relu")
              << ConvolutionLayer(
                  3U, 3U, 512U,
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv4_4_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv4_4_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv4_4")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv4_4/Relu")
              << PoolingLayer(PoolingLayerInfo(PoolingType::MAX, 2, PadStrideInfo(2, 2, 0, 0))).set_name("pool4")
              // Layer 5
              << ConvolutionLayer(
                  3U, 3U, 512U,
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv5_1_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv5_1_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv5_1")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv5_1/Relu")
              << ConvolutionLayer(
                  3U, 3U, 512U,
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv5_2_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv5_2_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv5_2")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv5_2/Relu")
              << ConvolutionLayer(
                  3U, 3U, 512U,
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv5_3_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv5_3_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv5_3")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv5_3/Relu")
              << ConvolutionLayer(
                  3U, 3U, 512U,
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv5_4_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/conv5_4_b.npy"),
                  PadStrideInfo(1, 1, 1, 1))
              .set_name("conv5_4")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("conv5_4/Relu")
              << PoolingLayer(PoolingLayerInfo(PoolingType::MAX, 2, PadStrideInfo(2, 2, 0, 0))).set_name("pool5")
              // Layer 6
              << FullyConnectedLayer(
                  4096U,
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/fc6_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/fc6_b.npy"))
              .set_name("fc6")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("Relu")
              // Layer 7
              << FullyConnectedLayer(
                  4096U,
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/fc7_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/fc7_b.npy"))
              .set_name("fc7")
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU)).set_name("Relu_1")
              // Layer 8
              << FullyConnectedLayer(
                  1000U,
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/fc8_w.npy", weights_layout),
                  get_weights_accessor(data_path, "/cnn_data/vgg19_model/fc8_b.npy"))
              .set_name("fc8")
              // Softmax
              << SoftmaxLayer().set_name("prob")
              << OutputLayer(get_output_accessor(common_params, 5));

        // Finalize graph
        GraphConfig config;
        config.num_threads = common_params.threads;
        config.use_tuner   = common_params.enable_tuner;
        config.tuner_file  = common_params.tuner_file;

        graph.finalize(common_params.target, config);

        return true;
    }
    void do_run() override
    {
        // Run graph
        graph.run();
    }

private:
    CommandLineParser  cmd_parser;
    CommonGraphOptions common_opts;
    CommonGraphParams  common_params;
    Stream             graph;
};

/** Main program for VGG19
 *
 * Model is based on:
 *      https://arxiv.org/abs/1409.1556
 *      "Very Deep Convolutional Networks for Large-Scale Image Recognition"
 *      Karen Simonyan, Andrew Zisserman
 *
 * @note To list all the possible arguments execute the binary appended with the --help option
 *
 * @param[in] argc Number of arguments
 * @param[in] argv Arguments
 */
int main(int argc, char **argv)
{
    return arm_compute::utils::run_example<GraphVGG19Example>(argc, argv);
}
