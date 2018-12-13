/* Copyright (c) 2017 SKKU ESLAB, and contributors. All rights reserved.
 *
 * Contributor: Gyeonghwan Hong<redcarrottt@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstring>
#include <sys/types.h>
#include <unistd.h>

#include "ANTdbugLog.h"
#include "MessageRouter.h"

#define VERB_ROUTING 0

void MessageRouter::addRoutingEntry(std::string uriString, Channel *channel) {
  Channel *oldChannel = findExactChannelLocked(uriString);
  if (oldChannel != NULL) {
    ANT_DBG_VERB("Routing Entry (%s -> %s) exists (Try to add entry %s -> %s)",
                 uriString.c_str(), oldChannel->getName().c_str(),
                 uriString.c_str(), channel->getName().c_str());
    return;
  }
  pthread_mutex_lock(&this->mMasterRoutingTableMutex);
  std::pair<std::string, Channel *> newEntry(uriString, channel);
  this->mMasterRoutingTable.insert(newEntry);
  pthread_mutex_unlock(&this->mMasterRoutingTableMutex);

  ANT_DBG_VERB("Routing Entry (%s -> %s) added", uriString.c_str(),
               channel->getName().c_str());
}

void MessageRouter::removeRoutingEntry(std::string uriString) {
  pthread_mutex_lock(&this->mMasterRoutingTableMutex);

  bool found = false;
  Channel *targetChannel = NULL;
  std::map<std::string, Channel *>::iterator rtIter;
  for (rtIter = this->mMasterRoutingTable.begin();
       rtIter != this->mMasterRoutingTable.end(); ++rtIter) {
    std::string entryUri(rtIter->first);
    if (entryUri.compare(uriString) == 0) {
      // Remove from routing table
      this->mMasterRoutingTable.erase(rtIter);

      targetChannel = rtIter->second;
      found = true;
      break;
    }
  }

  if (found == false) {
    ANT_DBG_WARN("Cannot find entry for URI %s", uriString.c_str());
  }

  ANT_DBG_VERB("Entry (%s -> %s) is removed from MessageRouterTable",
               uriString.c_str(), targetChannel->getName().c_str());

  pthread_mutex_unlock(&this->mMasterRoutingTableMutex);
}

void MessageRouter::printRoutingTable() {
  pthread_mutex_lock(&this->mMasterRoutingTableMutex);

  std::map<std::string, Channel *>::iterator rtIter;
  for (rtIter = this->mMasterRoutingTable.begin();
       rtIter != this->mMasterRoutingTable.end(); ++rtIter) {
    std::string entryUri(rtIter->first);
    Channel *entryChannel = rtIter->second;
#if VERB_ROUTING == 1
    ANT_DBG_WARN("Entry %s: %s", entryUri.c_str(),
                 entryChannel->getName().c_str());
#endif
  }

  pthread_mutex_unlock(&this->mMasterRoutingTableMutex);
}

void MessageRouter::routeMessage(Channel *originalChannel,
                                 BaseMessage *message) {
  std::string uriString = message->getUri().c_str();

  // Find all the target entry of given URI
  Channel *targetChannel = this->findBestChannelLocked(uriString);

  // Prevent message flooding
  // If original channel and target channel are same, message may be flooded.
  if (originalChannel == targetChannel) {
    return;
  }

#if VERB_ROUTING == 1
  ANT_DBG_WARN(
      "ROUTE: from %s to %s (in pid %d)\n%s\n=======",
      (originalChannel == NULL) ? "NULL" : originalChannel->getName().c_str(),
      (targetChannel == NULL) ? "NULL" : targetChannel->getName().c_str(),
      (getpid()), message->toJSONString());
#endif

  // If the message did not routed at all, make a warning message
  if (targetChannel != NULL) {
    targetChannel->routeMessage(message);
  }
}

Channel *MessageRouter::findBestChannelLocked(std::string uriString) {
  std::string givenURI(uriString);

  // Find all the target entry of given URI
  pthread_mutex_lock(&this->mMasterRoutingTableMutex);
  Channel *targetChannel = NULL;
  std::string targetUriString("");
  std::map<std::string, Channel *>::iterator rtIter;
  for (rtIter = this->mMasterRoutingTable.begin();
       rtIter != this->mMasterRoutingTable.end(); ++rtIter) {
    std::string entryUri(rtIter->first);
    Channel *entryChannel = rtIter->second;

    size_t foundPos = givenURI.find(entryUri);
    // Select the best matching target
    // At least, the pattern should be matched from the beginning of URI
    if (foundPos == 0) {
      // If target is not determined, keep it as a target.
      // If the matching length of this entry is longer than present target,
      // change the target.
      if ((targetUriString.empty()) ||
          (entryUri.size() < targetUriString.size())) {
        targetChannel = entryChannel;
        targetUriString.assign(entryUri);
      }
    }
  }
  pthread_mutex_unlock(&this->mMasterRoutingTableMutex);
  return targetChannel;
}

Channel *MessageRouter::findExactChannelLocked(std::string uriString) {
  // Find all the target entry of given URI
  pthread_mutex_lock(&this->mMasterRoutingTableMutex);
  Channel *targetChannel = NULL;
  std::map<std::string, Channel *>::iterator rtIter;
  for (rtIter = this->mMasterRoutingTable.begin();
       rtIter != this->mMasterRoutingTable.end(); ++rtIter) {
    std::string entryUri(rtIter->first);
    Channel *entryChannel = rtIter->second;

    if (entryUri.compare(uriString) == 0) {
      pthread_mutex_unlock(&this->mMasterRoutingTableMutex);
      return entryChannel;
    }
  }
  pthread_mutex_unlock(&this->mMasterRoutingTableMutex);
  return NULL;
}
