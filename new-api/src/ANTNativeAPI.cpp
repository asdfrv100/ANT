/* Copyright (c) 2017 SKKU ESLAB, and contributors. All rights reserved.
 *
 * Contributor: Gyeonghwan Hong<redcarrottt@gmail.com>
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

#include <node.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "AppBase.h"
#include "AppNativeAPI.h"
#include "CommNativeAPI.h"
#include "MLNativeAPI.h"
#include "ResourceNativeAPI.h"

using namespace v8;

#define MAX_NUMBER_OF_ITEMS 20
#define MESSAGE_BUFFER_SIZE 1024

// Utility Functions
#define getV8String(isolate, cstr)                                             \
  (String::NewFromOneByte(isolate, (const uint8_t *)cstr))

void initAppBase() {
  AppBase* appBase = AppBase::singleton();
  appBase->run();
}

// antnative Object
void init(Local<Object> exports) {
  initAppBase();

  // Initialize API object's prototypes
  AppNativeAPI::InitPrototype(exports->GetIsolate());
  MLNativeAPI::InitPrototype(exports->GetIsolate());
  CommNativeAPI::InitPrototype(exports->GetIsolate());
  ResourceNativeAPI::InitPrototype(exports->GetIsolate());

  // Set entries as API object's constructors
  NODE_SET_METHOD(exports, "app", AppNativeAPI::NewInstance);
  NODE_SET_METHOD(exports, "ml", MLNativeAPI::NewInstance);
  NODE_SET_METHOD(exports, "comm", CommNativeAPI::NewInstance);
  NODE_SET_METHOD(exports, "resource", ResourceNativeAPI::NewInstance);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, init)