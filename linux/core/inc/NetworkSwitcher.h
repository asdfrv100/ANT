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

#ifndef __NETWORK_SWITCHER_H__
#define __NETWORK_SWITCHER_H__

#include "Core.h"
#include "ServerAdapter.h"

#include "../../common/inc/DebugLog.h"

#include "../../configs/ExpConfig.h"

#include <mutex>
#include <thread>
#include <vector>

namespace sc {
class Core;
class NetworkSwitcher;

/* Transctions in NetworkSwitcher */
class SwitchAdapterTransaction {
  /*
   * Switch Adapter Transaction: Order
   * 1. Entry
   *    - NetworkSwitcher.switch_adapters()
   *    - SwitchAdapterTransaction.start()
   * 2-a. Connect/WakeUp Next Adapter
   *    - next_adapter.connect()
   *       or next_adapter.wake_up()
   * 2-b. Callback (for connect request)
   *    - SwitchAdapterTransaction.connect_callback()
   * 3-a. Disconnect/Sleep Prev Adapter
   *    - prev_adapter.disconnect()
   * 3-b. Callback (for disconnect request)
   *    - SwitchAdapterTransaction.disconnect_callback()
   * 4. NetworkSwitcher.done_switch()
   */
public:
  static bool run(int prev_index, int next_index);
  void start(void);
  static void connect_next_adapter_callback(bool is_success);
  static void disconnect_prev_adapter_callback(bool is_success);

protected:
  void done(bool is_success);

  SwitchAdapterTransaction(int prev_index, int next_index) {
    this->mPrevIndex = prev_index;
    this->mNextIndex = next_index;
  }
  static SwitchAdapterTransaction *sOngoing;

  int mPrevIndex;
  int mNextIndex;
}; /* class SwitchAdapterTransaction */

class ConnectRequestTransaction {
public:
  static bool run(int adapter_id);
  bool start(void);
  static void connect_callback(bool is_success);

protected:
  void done();

  ConnectRequestTransaction(int adapter_id) { this->mAdapterId = adapter_id; }
  static ConnectRequestTransaction *sOngoing;

  int mAdapterId;
}; /* class ConnectRequestTransaction */

class ReconnectAdapterTransaction {
public:
  static bool run(ServerAdapter *targetAdapter);
  bool start();
  static void disconnect_callback(bool is_success);
  static void connect_callback(bool is_success);

protected:
  void done(bool require_restart);

  ReconnectAdapterTransaction(ServerAdapter *targetAdapter) {
    this->mTargetAdapter = targetAdapter;
  }
  static ReconnectAdapterTransaction *sOngoing;

  ServerAdapter *mTargetAdapter;
}; /* class ReconnectAdapterTransaction */

/* Netweork Switcher State */
typedef enum {
  kNSStateReady = 0,
  kNSStateSwitching = 1,
} NSState;

/* Network Switcher */
class NetworkSwitcher {
public:
  /* APIs called by peer through Core */
  void connect_adapter_by_peer(int adapter_id);
  void disconnect_adapter_by_peer(int adapter_id);
  void sleep_adapter_by_peer(int adapter_id);
  void wake_up_adapter_by_peer(int adapter_id);

  /* APIs called by receiver/sender loop */
  void reconnect_adapter(ServerAdapter *adapter,
                         bool retry_if_already_switching);

  /* APIs called by NetworkMonitor */
  bool switch_adapters(int prev_index, int next_index);

private:
  /* Notification of switch done event */
  void done_switch();

public:
  /* State getter */
  NSState get_state(void) {
    std::unique_lock<std::mutex> lck(this->mStateLock);
    return this->mState;
  }

private:
  /* State setter */
  void set_state(NSState new_state) {
    std::unique_lock<std::mutex> lck(this->mStateLock);
    this->mState = new_state;
  }

  /* State */
  NSState mState;
  std::mutex mStateLock;

public:
  /* Singleton */
  static NetworkSwitcher *singleton(void) {
    if (NetworkSwitcher::singleton == NULL) {
      NetworkSwitcher::singleton = new NetworkSwitcher();
    }
    return NetworkSwitcher::singleton;
  }

private:
  /* Singleton */
  static NetworkSwitcher *singleton;
  NetworkSwitcher() { this->set_state(NSState::kNSStateReady); }

public:
  /* Its private members can be accessed by auxiliary classes */
  friend SwitchAdapterTransaction;
  friend ConnectRequestTransaction;
  friend ReconnectAdapterTransaction;
}; /* class NetworkSwitcher */
} /* namespace sc */

#endif /* !defined(__NETWORK_SWITCHER_H__) */