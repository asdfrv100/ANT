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
 * FITCLSS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONCLCTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "arm_compute/core/TensorShape.h"
#include "arm_compute/core/Types.h"
#include "arm_compute/runtime/CL/CLTensor.h"
#include "arm_compute/runtime/CL/CLTensorAllocator.h"
#include "arm_compute/runtime/CL/functions/CLNormalizationLayer.h"
#include "tests/CL/CLAccessor.h"
#include "tests/benchmark/fixtures/NormalizationLayerFixture.h"
#include "tests/datasets/system_tests/alexnet/AlexNetNormalizationLayerDataset.h"
#include "tests/datasets/system_tests/googlenet/inceptionv1/GoogLeNetInceptionV1NormalizationLayerDataset.h"
#include "tests/framework/Macros.h"
#include "tests/framework/datasets/Datasets.h"
#include "utils/TypePrinter.h"

namespace arm_compute
{
namespace test
{
namespace benchmark
{
namespace
{
const auto data_types = framework::dataset::make("DataType", { DataType::F16, DataType::F32 });
} // namespace

using CLNormalizationLayerFixture = NormalizationLayerFixture<CLTensor, CLNormalizationLayer, CLAccessor>;

TEST_SUITE(CL)

REGISTER_FIXTURE_DATA_TEST_CASE(AlexNetNormalizationLayer, CLNormalizationLayerFixture, framework::DatasetMode::ALL,
                                framework::dataset::combine(framework::dataset::combine(datasets::AlexNetNormalizationLayerDataset(),
                                                                                        data_types),
                                                            framework::dataset::make("Batches", 1)));

REGISTER_FIXTURE_DATA_TEST_CASE(GoogLeNetInceptionV1NormalizationLayer, CLNormalizationLayerFixture, framework::DatasetMode::ALL,
                                framework::dataset::combine(framework::dataset::combine(datasets::GoogLeNetInceptionV1NormalizationLayerDataset(),
                                                                                        data_types),
                                                            framework::dataset::make("Batches", 1)));

TEST_SUITE(NIGHTLY)
REGISTER_FIXTURE_DATA_TEST_CASE(AlexNetNormalizationLayer, CLNormalizationLayerFixture, framework::DatasetMode::NIGHTLY,
                                framework::dataset::combine(framework::dataset::combine(datasets::AlexNetNormalizationLayerDataset(),
                                                                                        data_types),
                                                            framework::dataset::make("Batches", { 4, 8 })));

REGISTER_FIXTURE_DATA_TEST_CASE(GoogLeNetInceptionV1NormalizationLayer, CLNormalizationLayerFixture, framework::DatasetMode::NIGHTLY,
                                framework::dataset::combine(framework::dataset::combine(datasets::GoogLeNetInceptionV1NormalizationLayerDataset(),
                                                                                        data_types),
                                                            framework::dataset::make("Batches", { 4, 8 })));
TEST_SUITE_END()
TEST_SUITE_END()
} // namespace benchmark
} // namespace test
} // namespace arm_compute
