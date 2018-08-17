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

#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <RefCount.h>

#include <thread>
#include <mutex>

#include <stdio.h>

namespace cm {

typedef enum {
  kOff        = 0,
  kTurningOn  = 1,
  kOn         = 2,
  kTurningOff = 3
} DeviceState;

class Device {
public:
  bool hold_and_turn_on(void);
  bool release_and_turn_off(void);

  virtual bool turn_on_impl(void) = 0;
  virtual bool turn_off_impl(void) = 0;

  DeviceState get_state(void) {
    return this->mState;
  }

  Device(const char* name) {
    this->mState = DeviceState::kOff;
    snprintf(this->mName, sizeof(this->mName), name);
  }

  ~Device() {
  }

protected:
  void set_state(DeviceState new_state) {
    this->mState = new_state;
  }

  DeviceState mState;
  char mName[256];

  /* Reference Count */
  RefCount mRefCount;
}; /* class Device */

} /* namespace cm */

#endif /* !defined(_DEVICE_H_) */
