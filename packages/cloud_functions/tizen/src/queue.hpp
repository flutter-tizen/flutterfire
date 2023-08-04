/*
 * Copyright (c) 2023-present Samsung Electronics Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CLOUD_FUNCTIONS_TIZEN_QUEUE_HPP_
#define CLOUD_FUNCTIONS_TIZEN_QUEUE_HPP_

#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
class Queue {
 public:
  Queue() = default;
  Queue(const Queue&) = delete;
  Queue(const Queue&&) = delete;
  Queue& operator=(const Queue&) = delete;
  Queue& operator=(const Queue&&) = delete;

  void push(const T& item) {
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.push(item);
    lock.unlock();
    cv_.notify_one();
  }

  bool empty() {
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.empty();
  }

  bool pop(T& item, const unsigned int millisecondWaitingPeriod) {
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.empty()) {
      auto timeout = std::chrono::milliseconds(millisecondWaitingPeriod);
      if (cv_.wait_for(lock, timeout) == std::cv_status::timeout) {
        return false;
      }
    }
    item = queue_.front();
    queue_.pop();
    return true;
  }

 private:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable cv_;
};

#endif  // CLOUD_FUNCTIONS_TIZEN_QUEUE_HPP_
