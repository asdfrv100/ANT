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

#ifndef _SERVER_SOCKET_H_
#define _SERVER_SOCKET_H_

#include <thread>
#include <mutex>
#include <condition_variable>

#include <stdio.h>

namespace cm {

typedef enum {
  kClosed  = 0,
  kOpening = 1,
  kOpened  = 2,
  kClosing = 3
} ServerSocketState;

class ServerSocket {
public:
  bool open(void);
  bool close(void);
  int send(const void *data_buffer, size_t data_length);
  int receive(void *data_buffer, size_t data_length);

  virtual bool open_impl(void) = 0;
  virtual bool close_impl(void) = 0;
  virtual int send_impl(const void *data_buffer, size_t data_length) = 0;
  virtual int receive_impl(void *data_buffer, size_t data_length) = 0;

  ServerSocketState get_state(void) {
    return this->mState;
  }

  ServerSocket(void) {
    this->mState = ServerSocketState::kClosed;
  }

  ~ServerSocket(void) {
  }

protected:
  void set_state(ServerSocketState new_state) {
    this->mState = new_state;
  }
  ServerSocketState mState;
}; /* class ServerSocket */

} /* namespace cm */

#endif /* !defined(_SERVER_SOCKET_H_) */
