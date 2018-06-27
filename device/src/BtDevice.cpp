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
#include <BtDevice.h>
#include <Util.h>

#include <counter.h>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <stdio.h>

using namespace cm;

BtDevice* BtDevice::sSingleton = NULL;

bool BtDevice::turn_on_impl(void) {
  char buf[512];
  char *const params[] = {"hciconfig", "hci0", "up", "piscan", NULL};

  int res = Util::run_client(HCICONFIG_PATH, params, buf, 512);
  return (res >= 0);
}

bool BtDevice::turn_off_impl(void) {
  char buf[512];
  char *const params[] = {"hciconfig", "hci0", "down", NULL};

  int res = Util::run_client(HCICONFIG_PATH, params, buf, 512);
  return (res >= 0);
}
