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

#include "../inc/SegmentManager.h"

#include "../inc/ProtocolManager.h"

#include "../../common/inc/DebugLog.h"

#include "../../configs/ExpConfig.h"

#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

#include <list>

using namespace sc;

/* Singleton */
SegmentManager *SegmentManager::singleton = NULL;

uint32_t SegmentManager::get_next_global_seq_no(uint32_t num_segments) {
  uint32_t res;
  mNextGlobalSeqNoLock.lock();
  res = this->mNextGlobalSeqNo;
  this->mNextGlobalSeqNo += num_segments;
  mNextGlobalSeqNoLock.unlock();
  return res;
}

void SegmentManager::serialize_segment_header(Segment *seg) {
  uint32_t net_seq_no = htonl(seg->seq_no);
  uint32_t net_flag_len = htonl(seg->flag_len);
  memcpy(seg->data, &net_seq_no, sizeof(uint32_t));
  memcpy(seg->data + 4, &net_flag_len, sizeof(uint32_t));
}

int SegmentManager::send_to_segment_manager(uint8_t *data, size_t len,
                                            bool is_control) {
  assert(data != NULL && len > 0);

  uint32_t offset = 0;
  uint32_t num_of_segments = ((len + kSegSize - 1) / kSegSize);
  assert((len + kSegSize - 1) / kSegSize < UINT32_MAX);

  /* Reserve sequence numbers to this thread */
  uint32_t allocated_seq_no = get_next_global_seq_no(num_of_segments);

  int seg_idx;
  for (seg_idx = 0; seg_idx < num_of_segments; seg_idx++) {
    uint32_t seg_len = (len - offset < kSegSize) ? len - offset : kSegSize;
    Segment *seg = this->get_free_segment();

    /* Set segment length */
    mSetSegLenBits(seg_len, seg->flag_len);

    /* Set segment sequence number */
    seg->seq_no = allocated_seq_no++;

    /* Set segment data */
    memcpy(&(seg->data[kSegHeaderSize]), data + offset, seg_len);
    offset += seg_len;

    /* Set segment MF flag and/or control segment flag */
    int flag = 0;
    if (offset < len)
      flag |= kSegFlagMF;
    if (is_control)
      flag |= kSegFlagControl;
    mSetSegFlagBits(flag, seg->flag_len);

        /* Set segment header to data */
    this->serialize_segment_header(seg);

    if (is_control) {
      this->enqueue(kSegSendControl, seg);
    } else {
      this->enqueue(kSegSendData, seg);
    }
  }

  return 0;
}

uint8_t *SegmentManager::recv_from_segment_manager(void *proc_data_handle,
                                                   bool is_control) {
  assert(proc_data_handle != NULL);

  ProtocolData *pd = reinterpret_cast<ProtocolData *>(proc_data_handle);
  uint8_t *serialized = NULL;
  uint16_t offset = 0;
  size_t data_size = 0;
  bool cont = false;
  bool dequeued = false;
  Segment *seg;

  while (dequeued == false) {
    if (is_control) {
      seg = dequeue(kSegRecvControl);
    } else {
      seg = dequeue(kSegRecvData);
    }
    if (seg) {
      dequeued = true;
    }
  }

  ProtocolManager::parse_header(&(seg->data[kSegHeaderSize]), pd);

  if (unlikely(pd->len == 0))
    return NULL;

  serialized = reinterpret_cast<uint8_t *>(calloc(pd->len, sizeof(uint8_t)));

  data_size = mGetSegLenBits(seg->flag_len) - kProtHeaderSize;
  memcpy(serialized + offset, &(seg->data[kSegHeaderSize]) + kProtHeaderSize,
         data_size);
  offset += data_size;

  cont = ((mGetSegFlagBits(seg->flag_len) & kSegFlagMF) != 0);

  free_segment(seg);

  while (cont) {
    if (is_control) {
      seg = dequeue(kSegRecvControl);
    } else {
      seg = dequeue(kSegRecvData);
    }
    data_size = mGetSegLenBits(seg->flag_len);
    memcpy(serialized + offset, &(seg->data[kSegHeaderSize]), data_size);
    cont = ((mGetSegFlagBits(seg->flag_len) & kSegFlagMF) != 0);
    offset += data_size;
    free_segment(seg);
  }

  return serialized;
}

/*
 * This function is the end of the sending logic.
 * It enqeueus the data to the sending queue in order with the sequence number.
 */
void SegmentManager::enqueue(SegQueueType type, Segment *seg) {
  assert(type < kSegMaxQueueType);
  // Get lock for the queue
  std::unique_lock<std::mutex> lck(this->mQueueLock[type]);
  bool segment_enqueued = false;

  if (type == kSegSendControl || type == kSegSendData) {
    this->mSendRequest.add(SEGMENT_DATA_SIZE);
  }

  if (seg->seq_no == this->mNextSeqNo[type]) {
    /*
     * If the sequence number is the right next one,
     * it executes enqueuing logic normally.
     */
    this->mNextSeqNo[type]++;
    this->mQueues[type].push_back(seg);
    this->mQueueLength[type].increase();
    segment_enqueued = true;
  } else {
    /*
     * If the sequence number is not the next expected one,
     * it enqueues its segments to the pending queue, not normal queue.
     */
    if (seg->seq_no <= this->mNextSeqNo[type]) {
      LOG_ERR("Sequence # Error!: %d > %d (Queue #%d)", seg->seq_no,
              this->mNextSeqNo[type], (int)type);
    }
    assert(seg->seq_no > this->mNextSeqNo[type]);

    std::list<Segment *>::iterator curr_it = this->mPendingQueues[type].begin();

    /*
     * If we received a unsequential segment, put it into pending queue.
     * Pending queue should retain segments in order of sequence number.
     */
    while (curr_it != this->mPendingQueues[type].end()) {
      Segment *walker = *curr_it;
      assert(walker->seq_no != seg->seq_no);
      if (walker->seq_no > seg->seq_no)
        break;
      curr_it++;
    }
    this->mPendingQueues[type].insert(curr_it, seg);
  }

  /*
   * Finally, we put all the consequent segments into type queue.
   * If no segment in the pending queue matches to the next seq_no,
   * Then this process is just skipped.
   */
  std::list<Segment *>::iterator curr_it = this->mPendingQueues[type].begin();
  while (curr_it != this->mPendingQueues[type].end() &&
         (*curr_it)->seq_no == this->mNextSeqNo[type]) {
    this->mNextSeqNo[type]++;
    this->mQueues[type].push_back(*curr_it);
    this->mQueueLength[type].increase();
    segment_enqueued = true;

    std::list<Segment *>::iterator to_erase = curr_it++;
    this->mPendingQueues[type].erase(to_erase);
  }

  if (segment_enqueued)
    this->mCondEnqueued[type].notify_all();
}

/*
 * Dequeue the segment from the queue.
 * Note that this function is used for sending & receiving queue.
 */
Segment *SegmentManager::dequeue(SegQueueType type) {
#ifdef EXP_MEASURE_INTERVAL_SEND_QUEUE
  struct timeval times[4];
  if (type == kSegSend)
    gettimeofday(&times[0], NULL);
#endif
  assert(type < kSegMaxQueueType);
  std::unique_lock<std::mutex> lck(this->mQueueLock[type]);

  /* If queue is empty, wait until some segment is enqueued */
  int queueLength = this->mQueueLength[type].get_value();

#ifdef EXP_MEASURE_INTERVAL_SEND_QUEUE
  if (type == kSegSend) {
    gettimeofday(&times[1], NULL);
    if (this->mSendCount % 500 == 0) {
      LOG_DEBUG("Try %d: Queue Length %d", this->mSendCount, queueLength);
    }
  }
#endif

  if (queueLength == 0) {
#ifdef VERBOSE_SEGMENT_QUEUE_WAITING
    if (type == kSegSend) {
      LOG_DEBUG("sending queue is empty. wait for another");
    } else {
      LOG_DEBUG("receiving queue is empty. wait for another");
    }
#endif

#ifdef EXP_MEASURE_INTERVAL_SEND_QUEUE
    if (type == kSegSend)
      LOG_DEBUG("Try %d: Queue Length 0 %d %d %d", this->mSendCount,
                queueLength, this->mQueueLength[kSegSend].get_value(),
                this->mQueues[type].size());
#endif

    this->mCondEnqueued[type].wait(lck);
  }

#ifdef EXP_MEASURE_INTERVAL_SEND_QUEUE
  if (type == kSegSend)
    gettimeofday(&times[2], NULL);
#endif

  /* Dequeue from queue */
  Segment *ret = this->mQueues[type].front();
  if (ret == NULL) {
    LOG_DEBUG("Queue[%s] is NULL(empty)", type == 0 ? "send" : "recv");
    return NULL;
  }
  this->mQueues[type].pop_front();
  this->mQueueLength[type].decrease();

#ifdef EXP_MEASURE_INTERVAL_SEND_QUEUE
  if (type == kSegSend)
    gettimeofday(&times[3], NULL);
#endif

#ifdef EXP_MEASURE_INTERVAL_SEND_QUEUE
  if (type == kSegSend) {
    for (int i = 0; i < 3; i++) {
      this->mIntervals[i] += (times[i + 1].tv_sec - times[i].tv_sec) * 1000 +
                             (times[i + 1].tv_usec - times[i].tv_usec) / 1000;
    }
    this->mSendCount++;
    if (this->mSendCount % 500 == 0) {
      LOG_DEBUG("Send Queue: %d / %d / %d", this->mIntervals[0],
                this->mIntervals[1], this->mIntervals[2]);
      for (int i = 0; i < 3; i++) {
        this->mIntervals[i] = 0;
      }
    }
  }
#endif

  return ret;
}

/*  This function returns a free segment from the list of free segments.
 *  If there is no one available, allocate new one
 */
Segment *SegmentManager::get_free_segment(void) {
  std::unique_lock<std::mutex> lck(this->mFreeSegmentListLock);
  Segment *ret = NULL;
  if (this->mFreeSegmentListSize == 0) {
    ret = reinterpret_cast<Segment *>(calloc(1, sizeof(Segment)));
  } else {
    ret = this->mFreeList.front();
    this->mFreeList.pop_front();
    this->mFreeSegmentListSize--;
  }

  ret->seq_no = 0;
  ret->flag_len = 0;
  assert(ret != NULL);
  return ret;
}

void SegmentManager::release_segment_from_free_list(uint32_t threshold) {
  while (this->mFreeSegmentListSize > threshold) {
    Segment *to_free = this->mFreeList.front();
    this->mFreeList.pop_front();
    free(to_free);
    this->mFreeSegmentListSize--;
  }
}

/*  This function releases the segment which is not further used.
 */
void SegmentManager::free_segment(Segment *seg) {
  std::unique_lock<std::mutex> lck(this->mFreeSegmentListLock);
  this->mFreeList.push_front(seg);
  this->mFreeSegmentListSize++;

  // If the free list is big enough, release the half elements of the list
  if (unlikely(this->mFreeSegmentListSize > kSegFreeThreshold)) {
    release_segment_from_free_list(kSegFreeThreshold / 2);
  }
}

void SegmentManager::free_segment_all(void) {
  std::unique_lock<std::mutex> lck(this->mFreeSegmentListLock);
  release_segment_from_free_list(0);
}

void SegmentManager::failed_sending(Segment *seg) {
  std::unique_lock<std::mutex> lck(this->mSendFailQueueLock);
  this->mSendFailQueue.push_back(seg);
  this->mFailedSendingQueueLength.add(1);
}

Segment *SegmentManager::get_failed_sending(void) {
  std::unique_lock<std::mutex> lck(this->mSendFailQueueLock);
  if (this->mSendFailQueue.size() == 0)
    return NULL;

  Segment *res = this->mSendFailQueue.front();
  this->mSendFailQueue.pop_front();

  return res;
}