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

#ifndef _RFCOMM_SERVER_SOCKET_H_
#define _RFCOMM_SERVER_SOCKET_H_

#include <ServerSocket.h>

#include <Counter.h>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

namespace cm {

class RfcommServerSocket : ServerSocket {
public:
  virtual bool open_impl(void);
  virtual bool close_impl(void);
  virtual int send_impl(const void *data_buffer, size_t data_length);
  virtual int receive_impl(void *data_buffer, size_t data_length);

  uuid_t get_service_uuid(void) {
    return this->mServiceUUID;
  }

  RfcommServerSocket(const char* service_uuid) {
    char buffer[100];
    snprintf(buffer, 100, "%s", service_uuid);
    this->str2uuid(buffer, &(this->mServiceUUID));
  }

  ~RfcommServerSocket(void) {
  }

protected:
  uuid_t mServiceUUID;
  int mPort;

  sdp_session_t* mSdpSession;
  int mServerSocket;
  int mClientSocket;

private:
  int bt_dynamic_bind_rc(void);
  int bt_register_service(void);
  int str2uuid(char *str, uuid_t *uuid);
}; /* class RfcommServerSocket */

} /* namespace cm */

#endif /* !defined(_RFCOMM_SERVER_SOCKET_H_) */
