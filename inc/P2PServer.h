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

#ifndef _P2P_SERVER_H_
#define _P2P_SERVER_H_

#include <counter.h>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <stdio.h>

namespace cm {

typedef enum {
  kScanDisallowed = 0,
  kScanAllowed = 1
} P2PServerState;

class P2PServer {
public:
  bool allow_scan(void);
  bool disallow_scan(void);

  virtual bool allow_scan_impl(void) = 0;
  virtual bool disallow_scan_impl(void) = 0;

  P2PServerState get_state(void) {
    return this->mState;
  }

  P2PServer(char* name) {
    this->mState = P2PServerState::kScanDisallowed;
    snprintf(this->mName, sizeof(this->mName), name);
  }
  ~Device() {
  }

protected:
  void set_state(P2PServerState new_state) {
    this->mState = new_state;
  }

  P2PServerState mState;
  char mName[256];
}; /* class P2PServer */

} /* namespace cm */

#endif /* !defined(_P2P_SERVER_H_) */
