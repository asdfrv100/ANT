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

#ifndef _WFD_DEVICE_H_
#define _WFD_DEVICE_H_

#include <Device.h>

#include <counter.h>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <stdio.h>

namespace cm {

class WfdDevice : Device {
public:
  virtual bool turn_on_impl(void);
  virtual bool turn_off_impl(void);

  static WfdDevice* getSingleton() {
    if(WfdDevice::sSingleton == NULL) {
      WfdDevice::sSingleton = new WfdDevice();
    }
    return WfdDevice::sSingleton;
  }

  static WfdDevice* sSingleton;

  ~WfdDevice(void) {
  }

protected:
  WfdDevice(int id) : Device(id, "Wi-fi Direct") {
  }

}; /* class WfdDevice */

} /* namespace cm */

#endif /* !defined(_WFD_DEVICE_H_) */
