/* Copyright 2017-2018 All Rights Reserved.
 *  Gyeonghwan Hong (redcarrottt@gmail.com)
 *  
 * [Contact]
 *  Gyeonghwan Hong (redcarrottt@gmail.com)
 *
 * Licensed under the Apache License, Version 2.0(the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ServerAdapter.h>

using namespace cm;

int ServerAdapter::sNextId = 0;

ServerAdapter::~ServerAdapter() {
  // TODO
}

bool ServerAdapter::connect(void) {
  // TODO
}

bool ServerAdapter::disconnect(void) {
  // TODO
}

int ServerAdapter::send(const void *buf, size_t len) {
  // TODO
}

int ServerAdapter::recv(void *buf, size_t len) {
  // TODO
}

void run_sender(void) {
  // TODO
}

void run_recver(void) {
  // TODO
}

void join_threads() {
  // TODO
}

void return_sending_failed_packet(void *segment) {
  // TODO
}
