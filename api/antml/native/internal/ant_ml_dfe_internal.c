/* Copyright (c) 2017-2021 SKKU ESLAB, and contributors. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include <python3.6m/Python.h>

#include "../../../common/native/ant_common.h"
#include "./ant_ml_dfe_internal.h"

void interpreters_destroy(void *interpreters) {
  PyObject *pyInterpreters = (PyObject *)interpreters;
  Py_DECREF(pyInterpreters);
}

PyObject *gPyModule = NULL;

void *ant_ml_dfeLoad_internal(const char *modelName, int numFragments) {
  if (gPyModule == NULL) {
    PyObject *pyModuleName = PyUnicode_FromString("ant_ml_dfe");
    gPyModule = PyImport_Import(pyModuleName);
    Py_DECREF(pyModuleName);
  }

  if (gPyModule == NULL) {
    fprintf(stderr, "ERROR: ant_ml_dfeLoad_internal - Module not imported\n");
    return NULL;
  }

  PyObject *pyFunc = PyObject_GetAttrString(gPyModule, "dfe_load");
  if (pyFunc == NULL) {
    fprintf(stderr, "ERROR: ant_ml_dfeLoad_internal - Null function\n");
    return NULL;
  } else if (!PyCallable_Check(pyFunc)) {
    fprintf(stderr, "ERROR: ant_ml_dfeLoad_internal - Function not callable\n");
    Py_DECREF(pyFunc);
    return NULL;
  }

  PyObject *pyArgs = PyTuple_New(2);
  int ret;
  // Arg 0: string model_name
  PyObject *pyModelName = Py_BuildValue("s#", modelName, strlen(modelName));
  assert(pyModelName != NULL);
  ret = PyTuple_SetItem(pyArgs, 0, pyModelName);
  assert(ret == 0);
  Py_DECREF(pyModelName);
  // Arg 1: int num_fragments
  PyObject *pyNumFragments = PyLong_FromLong((long)numFragments);
  assert(pyNumFragments != NULL);
  ret = PyTuple_SetItem(pyArgs, 1, pyNumFragments);
  assert(ret == 0);
  Py_DECREF(pyNumFragments);

  // Call inner python function
  PyObject *pyInterpreters = PyObject_CallObject(pyFunc, pyArgs);
  assert(pyInterpreters != NULL);
  Py_DECREF(pyFunc);
  Py_DECREF(pyArgs);

  // Return: array[object] interpreters
  void *interpreters = (void *)pyInterpreters;

  return interpreters;
}

void *ant_ml_dfeExecute_internal(void *interpreters, void *inputTensor,
                                 size_t inputTensorLength, int startLayerNum,
                                 int endLayerNum) {
  if (gPyModule == NULL) {
    PyObject *pyModuleName = PyUnicode_FromString("ant_ml_dfe");
    gPyModule = PyImport_Import(pyModuleName);
    Py_DECREF(pyModuleName);
  }

  if (gPyModule == NULL) {
    fprintf(stderr,
            "ERROR: ant_ml_dfeExecute_internal - Module not imported\n");
    return NULL;
  }

  PyObject *pyFunc = PyObject_GetAttrString(gPyModule, "dfe_execute");
  if (pyFunc == NULL) {
    fprintf(stderr, "ERROR: ant_ml_dfeExecute_internal - Null function\n");
    return NULL;
  } else if (!PyCallable_Check(pyFunc)) {
    fprintf(stderr,
            "ERROR: ant_ml_dfeExecute_internal - Function not callable\n");
    Py_DECREF(pyFunc);
    return NULL;
  }

  PyObject *pyArgs = PyTuple_New(2);
  int ret;
  // Arg 0: object interpreters
  PyObject *pyInterpreters = (PyObject *)interpreters;
  ret = PyTuple_SetItem(pyArgs, 0, pyInterpreters);
  assert(ret == 0);

  // Arg 1: bytebuffer input_tensor
  PyObject *pyInputTensor = Py_BuildValue("S", inputTensor, inputTensorLength);
  assert(pyInputTensor);
  ret = PyTuple_SetItem(pyArgs, 1, pyInputTensor);
  assert(ret == 0);
  Py_DECREF(pyInputTensor);

  // Arg 2: int start_layer_num
  PyObject *pyStartLayerNum = PyLong_FromLong((long)startLayerNum);
  assert(pyStartLayerNum != NULL);
  ret = PyTuple_SetItem(pyArgs, 2, pyStartLayerNum);
  assert(ret == 0);
  Py_DECREF(pyStartLayerNum);

  // Arg 3: int end_layer_num
  PyObject *pyEndLayerNum = PyLong_FromLong((long)endLayerNum);
  assert(pyEndLayerNum != NULL);
  ret = PyTuple_SetItem(pyArgs, 2, pyEndLayerNum);
  assert(ret == 0);
  Py_DECREF(pyEndLayerNum);

  // Call inner python function
  PyObject *pyOutputTensor = PyObject_CallObject(pyFunc, pyArgs);
  assert(pyOutputTensor != NULL);
  Py_DECREF(pyArgs);
  Py_DECREF(pyFunc);

  // Return: bytebuffer output_tensor
  void *outputTensor = (void *)pyOutputTensor;
  return outputTensor;
}

void *ant_ml_dfeExecute_getOutputBufferWithLength(void *outputTensor,
                                                  size_t *pOutputTensorLength) {
  PyObject *pyOutputTensor = (PyObject *)outputTensor;
  char *outputTensorBuffer = NULL;
  int ret = PyBytes_AsStringAndSize(pyOutputTensor, &outputTensorBuffer,
                                    (Py_ssize_t *)pOutputTensorLength);
  assert(ret >= 0);
  return (void *)outputTensorBuffer;
}
void ant_ml_dfeExecute_releaseOutput(void *outputTensor) {
  PyObject *pyOutputTensor = (PyObject *)outputTensor;
  Py_DECREF(pyOutputTensor);
}

void initANTMLDFEInternal(void) {
  // Initialize Python interpreter
  Py_Initialize();
}