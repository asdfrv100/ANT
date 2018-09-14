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

#ifndef _BT_P2P_SERVER_H_
#define _BT_P2P_SERVER_H_

#include <P2PServer.h>

#include <thread>
#include <mutex>

#include <stdio.h>

namespace sc {

class BtP2PServer : public P2PServer {
public:
  virtual bool allow_discover_impl(void);
  virtual bool disallow_discover_impl(void);

  static BtP2PServer* getSingleton() {
    if(BtP2PServer::sSingleton == NULL) {
      BtP2PServer::sSingleton = new BtP2PServer();
    }
    return BtP2PServer::sSingleton;
  }

  static BtP2PServer* sSingleton;

  ~BtP2PServer(void) {
  }

protected:
  BtP2PServer(void) : P2PServer("BT") {
  }
}; /* class BtP2PServer */

} /* namespace sc */

#endif /* !defined(_BT_P2P_SERVER_H_) */
