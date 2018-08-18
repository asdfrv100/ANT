/* Copyright 2017-2018 All Rights Reserved.
 *  Gyeonghwan Hong (redcarrottt@gmail.com)
 *  Eunsoo Park (esevan.park@gmail.com)
 *  Injung Hwang (sinban04@gmail.com)
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

#include <Core.h>

#include <DebugLog.h>
#include <NetworkSwitcher.h>
#include <ProtocolManager.h>
#include <SegmentManager.h>

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

/**
 * < When to Free Buffer >
 * [*] Sending
 * When ProtocolManager::serialize() method is invoked, memory will be
 * allocated. ProtocolManager is in charge of allocating this memory.
 * SegmentManager is in charge of freeing this memory.
 * [*] Receiving
 * SegmentManager invokes ProtocolManager::parse() with allocated serialized
 * vector. SegmentManager is in charge of allocating this memory. After copying
 * to application memory, serialized vector should be freed. ProtocolManager is
 * in charge of freeing this memory.
 */
namespace sc {

Core *Core::singleton = NULL;

StartCoreTransaction *StartCoreTransaction::sOngoing = NULL;
StopCoreTransaction *StopCoreTransaction::sOngoing = NULL;

// Start core: connect initial adapters
void Core::start() {
  if (this->get_state() != CMState::kCMStateIdle) {
    LOG_ERR("Core has already started.");
    this->done_start(false);
    return;
  } else if (this->mControlAdapter == NULL) {
    LOG_ERR("No control adapter is registered!");
    this->done_start(false);
    return;
  } else if (this->mDataAdapters.empty()) {
    LOG_ERR("No data adapter is registered!");
    this->done_start(false);
    return;
  }

  // Connect control adapter & first data adapter
  this->set_state(CMState::kCMStateStarting);
  StartCoreTransaction::run(this);
}

void Core::done_start(bool is_success) {
  if (is_success) {
    LOG_VERB("Succeed to start core!");
    this->set_state(CMState::kCMStateReady);
  } else {
    LOG_ERR("Failed to start core!");
    this->set_state(CMState::kCMStateIdle);
    return;
  }

  start_sc_done(is_success);
}

// Stop core: disconnect all the adapters
void Core::stop() {
  CMState state = this->get_state();
  if (state == CMState::kCMStateStarting ||
      state == CMState::kCMStateStopping) {
    LOG_ERR("Cannot stop core during starting/stopping!");
    this->done_stop(false);
    return;
  } else if (state == CMState::kCMStateIdle) {
    LOG_ERR("Core is already idle state!");
    this->done_stop(false);
    return;
  } else if (this->mControlAdapter == NULL) {
    LOG_ERR("No control adapter is registered!");
    this->done_stop(false);
    return;
  } else if (this->mDataAdapters.empty()) {
    LOG_ERR("No data adapter is registered!");
    this->done_stop(false);
    return;
  }

  // Disconnect all the adapters
  this->set_state(CMState::kCMStateStopping);
  StopCoreTransaction::run(this);
}

void Core::done_stop(bool is_success) {
  if (is_success) {
    LOG_VERB("Succeed to stop core!");
    this->set_state(CMState::kCMStateIdle);
  } else {
    LOG_ERR("Failed to stop core!");
    this->set_state(CMState::kCMStateReady);
  }

  stop_sc_done(is_success);
}

// Register control adpater
void Core::register_control_adapter(ServerAdapter *control_adapter) {
  if (this->get_state() != CMState::kCMStateIdle) {
    LOG_ERR("You can register control adapter on only idle state!");
    return;
  }
  std::unique_lock<std::mutex> lck(this->mControlAdapterLock);
  if (this->mControlAdapter != NULL) {
    LOG_ERR("Another control adapter has already been registered!");
    return;
  }
  control_adapter->enable_receiver_thread(Core::receive_control_message_loop);
  this->mControlAdapter = control_adapter;
}

// Register data adpater
void Core::register_data_adapter(ServerAdapter *data_adapter) {
  if (this->get_state() != CMState::kCMStateIdle) {
    LOG_ERR("You can register data adapter on only idle state!");
    return;
  }
  std::unique_lock<std::mutex> lck(this->mDataAdaptersLock);
  data_adapter->enable_receiver_thread(NULL);
  data_adapter->enable_sender_thread();
  this->mDataAdapters.push_back(data_adapter);
}

int Core::send(const void *dataBuffer, uint32_t dataLength) {
  CMState state = this->get_state();
  if (state != CMState::kCMStateReady) {
    LOG_ERR("Core is not started yet, so you cannot send the data");
    return -1;
  }
  uint32_t curr_offset = 0;
  int sent_bytes;
  ProtocolData pd;
  uint8_t *serialized_vector;
  uint32_t packet_size;

  // Attach the protocol header to the payload
  ProtocolManager::data_to_protocol_data((const uint8_t *)dataBuffer,
                                         dataLength, &pd);
  // The serialized_vector buffer is allocated in here
  packet_size = ProtocolManager::serialize(&pd, (const uint8_t *)dataBuffer, curr_offset,
                                 dataLength, &serialized_vector);
  assert(serialized_vector != NULL && packet_size > 0);

  // Hand over the data to the Protocol Manager
  sent_bytes = ProtocolManager::send_packet(serialized_vector, packet_size);
  if (unlikely(sent_bytes < 0)) {
    LOG_ERR("Sending stopped(%u/%u) by %d", curr_offset, dataLength,
            sent_bytes);
  }
  return sent_bytes;
}

int Core::receive(void **pDataBuffer) {
  CMState state = this->get_state();
  if (state != CMState::kCMStateReady) {
    LOG_ERR("Core is not started yet, so you cannot receive data");
    return -1;
  }
  int ret;
  uint32_t packet_size;
  uint8_t *packet;

  packet_size = ProtocolManager::recv_packet(&packet);
  *pDataBuffer = packet;

  return packet_size;
}

void Core::send_control_message(const void *dataBuffer, size_t dataLength) {
  CMState state = this->get_state();
  if (state != CMState::kCMStateReady) {
    LOG_ERR("Core is not started yet, so you cannot send the data");
    return;
  }
  ServerAdapter *control_adapter = this->get_control_adapter();
  control_adapter->send(dataBuffer, dataLength);
}

void Core::send_request_connect(uint16_t adapter_id) {
  CMState state = this->get_state();
  if (state != CMState::kCMStateReady) {
    LOG_ERR("Core is not started yet, so you cannot send the data");
    return;
  }

  uint8_t request_code = kCtrlReqConnect;
  uint16_t net_adapter_id = htons(adapter_id);

  this->send_control_message(&request_code, 1);
  this->send_control_message(&net_adapter_id, 2);
}

void Core::send_noti_private_data(uint16_t adapter_id, char *private_data_buf,
                                  uint32_t private_data_len) {
  CMState state = this->get_state();
  if (state != CMState::kCMStateReady) {
    LOG_ERR("Core is not started yet, so you cannot send the data");
    return;
  }
  uint8_t request_code = kCtrlReqPriv;
  uint16_t net_adapter_id = htons(adapter_id);
  uint32_t net_private_data_len = htonl(private_data_len);

  this->send_control_message(&request_code, 1);
  this->send_control_message(&net_adapter_id, 2);
  this->send_control_message(&net_private_data_len, 4);
  this->send_control_message(private_data_buf, private_data_len);
}

void Core::receive_control_message_loop(ServerAdapter *adapter) {
  assert(adapter != NULL);
  LOG_VERB("Ready to receive control message");

  char data[512] = {
      0,
  };
  int res = 0;

  Core *core = Core::get_instance();
  while (true) {
    // Receive 1Byte: Control Request Code
    res = adapter->receive(data, 1);
    if (res <= 0) {
      LOG_VERB("Control adapter could be closed");
      sleep(1);
      continue;
    }

    char req_code = data[0];
    /*  If the control message is 'connect adapter', */
    if (req_code == CtrlReq::kCtrlReqConnect) {
      // Receive 2Byte: Adapter ID
      res = adapter->receive(data, 2);
      if (res <= 0) {
        LOG_DEBUG("Control adapter has been closed");
        break;
      }

      // convert adapter_id to n_adapter_id
      uint16_t n_adapter_id;
      uint16_t adapter_id;

      memcpy(&n_adapter_id, data, 2);
      adapter_id = ntohs(n_adapter_id);

      LOG_DEBUG("Control Request: 'Connect Adapter Request' (%d)", (int)adapter_id);
      NetworkSwitcher::get_instance()->connect_adapter(adapter_id);
    } else if (req_code == CtrlReq::kCtrlReqPriv) {
      LOG_VERB("Private data arrived");
      uint16_t n_adapter_id;
      uint16_t adapter_id;
      uint32_t nlen;
      uint32_t len;

      // Receive 2Byte: Adapter ID
      res = adapter->receive(&n_adapter_id, 2);
      if (res <= 0)
        break;
      adapter_id = ntohs(n_adapter_id);

      // Receive 4Byte: Private Data Length
      res = adapter->receive(&nlen, 4);
      if (res <= 0)
        break;
      len = ntohl(nlen);
      assert(len <= 512);

      // Receive nByte: Private Data
      res = adapter->receive(data, len);
      if (res > 0) {
        for (std::vector<ControlMessageListener *>::iterator it =
                 core->mControlMessageListeners.begin();
             it != core->mControlMessageListeners.end(); it++) {
          ControlMessageListener *listener = *it;
          listener->on_receive_control_message(adapter_id, data, len);
        }
      } else {
        LOG_DEBUG("Control adapter closed");
        break;
      }
    }
  } // End while

  // If control message loop is crashed, reconnect control adapter.
  NetworkSwitcher::get_instance()->reconnect_control_adapter();
}

// Transactions
// ----------------------------------------------------------------------------------

// Start Core
bool StartCoreTransaction::run(Core *caller) {
  if (sOngoing == NULL) {
    sOngoing = new StartCoreTransaction(caller);
    sOngoing->start();
    return true;
  } else {
    LOG_WARN("Already starting core");
    caller->done_start(false);
    return false;
  }
}
bool StartCoreTransaction::start() {
  // Connect control adpater
  std::unique_lock<std::mutex> lck(this->mCaller->mControlAdapterLock);
  bool res = this->mCaller->mControlAdapter->connect(
      StartCoreTransaction::connect_control_adapter_callback, false);
  if (!res) {
    LOG_ERR("Connecting control adapter is failed");
    this->done(false);
  }
  return res;
}

void StartCoreTransaction::connect_control_adapter_callback(bool is_success) {
  if (!is_success) {
    LOG_ERR("Connecting control adapter is failed");
    sOngoing->done(false);
    return;
  }

  // Connect first data adapter
  std::unique_lock<std::mutex> lck(sOngoing->mCaller->mDataAdaptersLock);
  bool res = true;
  if(sOngoing->mCaller->mDataAdapters.empty()) {
    res = false;
  } else {
    res = sOngoing->mCaller->mDataAdapters.front()->connect(
        StartCoreTransaction::connect_first_data_adapter_callback, false);
  }

  if (!res) {
    LOG_ERR("Connecting first data adapter is failed");
    sOngoing->done(false);
  }
}

void StartCoreTransaction::connect_first_data_adapter_callback(
    bool is_success) {
  if (is_success) {
    // Done transaction
    sOngoing->done(true);
  } else {
    LOG_ERR("Connecting first data adapter is failed");
    sOngoing->done(false);
  }
}

void StartCoreTransaction::done(bool is_success) {
    this->mCaller->done_start(is_success);
    sOngoing = NULL;
  }

// Stop Core
bool StopCoreTransaction::run(Core *caller) {
  if (sOngoing == NULL) {
    sOngoing = new StopCoreTransaction(caller);
    sOngoing->start();
    return true;
  } else {
    LOG_WARN("Already stopping core");
    caller->done_stop(false);
    return false;
  }
}
bool StopCoreTransaction::start() {
  {
    std::unique_lock<std::mutex> lck(this->mDataAdaptersCountLock);
    this->mDataAdaptersCount = this->mCaller->get_data_adapter_count();
  }

  // Disconnect control adpater
  bool res;
  {
    std::unique_lock<std::mutex> lck(this->mCaller->mControlAdapterLock);
    res = this->mCaller->mControlAdapter->disconnect(
        StopCoreTransaction::disconnect_control_adapter_callback);
  }

  if (!res) {
    LOG_ERR("Connecting control adapter is failed");
    this->done(false);
  }
  return res;
}

void StopCoreTransaction::disconnect_control_adapter_callback(bool is_success) {
  if (!is_success) {
    LOG_ERR("Connecting control adapter is failed");
    sOngoing->done(false);
    return;
  }

  // Disconnect all data adapters
  {
    std::unique_lock<std::mutex> lck(sOngoing->mCaller->mDataAdaptersLock);
    for (std::vector<ServerAdapter *>::iterator it =
             sOngoing->mCaller->mDataAdapters.begin();
         it != sOngoing->mCaller->mDataAdapters.end(); it++) {
      ServerAdapter *data_adapter = *it;
      bool res = data_adapter->disconnect(
          StopCoreTransaction::disconnect_data_adapter_callback);
      if (!res) {
        LOG_ERR("Disconnecting data adapter is failed");
      }
    }
  }
}

void StopCoreTransaction::disconnect_data_adapter_callback(bool is_success) {
  if (!is_success) {
    LOG_ERR("Connecting data adapter is failed 2");
    sOngoing->done(false);
    return;
  }

  bool done_disconnect_all = false;
  {
    std::unique_lock<std::mutex> lck(sOngoing->mDataAdaptersCountLock);
    sOngoing->mDataAdaptersCount--;
    if (sOngoing->mDataAdaptersCount == 0) {
      done_disconnect_all = true;
    }
  }

  // Done transaction
  if (done_disconnect_all) {
    sOngoing->done(true);
  } else {
    sOngoing->done(false);
  }
}

void StopCoreTransaction::done(bool is_success) {
    this->mCaller->done_stop(is_success);
    sOngoing = NULL;
  }

} /* namespace sc */