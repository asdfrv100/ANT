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
#include <Util.h>

#include <DebugLog.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>

using namespace cm;

int Util::run_client(const char *path, char * const params[], char *res_buf, size_t len) {
  int fd[2];
  int pid;
  int bk;
  static int initialized;

  if (pipe(fd) < 0) {
    LOG_ERR("pipe open error");
    return errno;
  }
  dup2(1, bk);

  if ((pid = fork()) < 0) {
    LOG_ERR("fork error");
    return errno;
  } else if (pid > 0) { //LOG_VERB("Forked PID : %d", pid);
    /* Parent process */
    close(fd[1]);

    char buf[1024];
    int read_bytes = read(fd[0], buf, 1024);

    if (read_bytes < 0) {
      LOG_ERR("%d read error", pid);
      return errno;
    }

    memcpy(res_buf, buf, read_bytes < len ? read_bytes:len);
    dup2(bk, 1);

    close(fd[0]);

    return pid;
  } else {
    /* Child process */
    close(fd[0]);
    dup2(fd[1], 1);

    execv(HCICONFIG_PATH, params);
    return 0;
  }
}
