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
#ifndef __ARM_COMPUTE_GCBATCHNORMALIZATIONLAYERKERNEL_H__
#define __ARM_COMPUTE_GCBATCHNORMALIZATIONLAYERKERNEL_H__

#include "arm_compute/core/GLES_COMPUTE/IGCKernel.h"

namespace arm_compute
{
class IGCTensor;

/** Interface for the BatchNormalization layer kernel.
 */
class GCBatchNormalizationLayerKernel : public IGCKernel
{
public:
    /** Constructor */
    GCBatchNormalizationLayerKernel();
    /** Prevent instances of this class from being copied (As this class contains pointers) */
    GCBatchNormalizationLayerKernel(const GCBatchNormalizationLayerKernel &) = delete;
    /** Prevent instances of this class from being copied (As this class contains pointers) */
    GCBatchNormalizationLayerKernel &operator=(const GCBatchNormalizationLayerKernel &) = delete;
    /** Default Move Constructor. */
    GCBatchNormalizationLayerKernel(GCBatchNormalizationLayerKernel &&) = default;
    /** Default move assignment operator */
    GCBatchNormalizationLayerKernel &operator=(GCBatchNormalizationLayerKernel &&) = default;
    /** Default destructor */
    ~GCBatchNormalizationLayerKernel() = default;

    /** Set the input and output tensors.
     *
     * @param[in]  input    Source tensor. 3 lower dimensions represent a single input with dimensions [width, height, FM].
     *                      The rest are optional and used for representing batches. Data types supported: F16/F32.
     * @param[out] output   Destination tensor. Output will have the same number of dimensions as input. Data type supported: same as @p input
     * @param[in]  mean     Mean values tensor. 1 dimension with size equal to the feature maps [FM]. Data types supported: Same as @p input
     * @param[in]  var      Variance values tensor. 1 dimension with size equal to the feature maps [FM]. Data types supported: Same as @p input
     * @param[in]  beta     (Optional) Beta values tensor info. 1 dimension with size equal to the feature maps [FM]. If not provided, default value for beta is 0. Data types supported: Same as @p input
     * @param[in]  gamma    (Optional) Gamma values tensor info. 1 dimension with size equal to the feature maps [FM]. If not provided, default value for gamma is 1. Data types supported: Same as @p input
     * @param[in]  epsilon  (optional) Small value to avoid division with zero.
     * @param[in]  act_info (Optional) Activation layer information in case of a fused activation. Only RELU, BOUNDED_RELU and LU_BOUNDED_RELU supported.
     */
    void configure(const IGCTensor *input, IGCTensor *output, const IGCTensor *mean, const IGCTensor *var, const IGCTensor *beta = nullptr, const IGCTensor *gamma = nullptr, float epsilon = 0.001f,
                   ActivationLayerInfo act_info = ActivationLayerInfo());
    /** Static function to check if given info will lead to a valid configuration of @ref GCBatchNormalizationLayerKernel
     *
     * @param[in] input    Source tensor info. In case of @p output tensor info = nullptr, this tensor will store the result.
     *                     3 lower dimensions represent a single input with dimensions [width, height, FM].
     *                     The rest are optional and used for representing batches. Data types supported: F16/F32.
     * @param[in] output   Destination tensor info. Output will have the same number of dimensions as input. Data type supported: same as @p input
     * @param[in] mean     Mean values tensor info. 1 dimension with size equal to the feature maps [FM]. Data types supported: Same as @p input
     * @param[in] var      Variance values tensor info. 1 dimension with size equal to the feature maps [FM]. Data types supported: Same as @p input
     * @param[in] beta     (Optional) Beta values tensor info. 1 dimension with size equal to the feature maps [FM]. If not provided, default value for beta is 0. Data types supported: Same as @p input
     * @param[in] gamma    (Optional) Gamma values tensor info. 1 dimension with size equal to the feature maps [FM]. If not provided, default value for gamma is 1. Data types supported: Same as @p input
     * @param[in] epsilon  (Optional) Small value to avoid division with zero. Default value is 0.001f.
     * @param[in] act_info (Optional) Activation layer information in case of a fused activation. Only RELU, BOUNDED_RELU and LU_BOUNDED_RELU supported.
     *
     * @return a status
     */
    static Status validate(const ITensorInfo *input, const ITensorInfo *output,
                           const ITensorInfo *mean, const ITensorInfo *var,
                           const ITensorInfo *beta = nullptr, const ITensorInfo *gamma = nullptr,
                           float epsilon = 0.001f, ActivationLayerInfo act_info = ActivationLayerInfo());

    // Inherited methods overridden:
    void run(const Window &window) override;

private:
    const IGCTensor *_input;
    IGCTensor       *_output;
    const IGCTensor *_mean;
    const IGCTensor *_var;
    const IGCTensor *_beta;
    const IGCTensor *_gamma;
    float            _epsilon;
};
}
#endif /*__ARM_COMPUTE_GCBATCHNORMALIZATIONLAYERKERNEL_H__ */
