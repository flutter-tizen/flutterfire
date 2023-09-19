// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "firebase_storage_listener.h"

#include <flutter/method_channel.h>

#include <memory>

StorageListener::StorageListener(const utils::StorageTaskData& task_data,
                                 const std::shared_ptr<FlMethodChannel> channel)
    : task_data_(task_data), channel_(std::move(channel)) {}

void StorageListener::OnPaused(firebase::storage::Controller* controller) {
  channel_->InvokeMethod(
      "Task#onPaused",
      std::make_unique<flutter::EncodableValue>(
          utils::GetTaskEventValue(task_data_, controller->bytes_transferred(),
                                   controller->total_byte_count())));
}

void StorageListener::OnProgress(firebase::storage::Controller* controller) {
  channel_->InvokeMethod(
      "Task#onProgress",
      std::make_unique<flutter::EncodableValue>(
          utils::GetTaskEventValue(task_data_, controller->bytes_transferred(),
                                   controller->total_byte_count())));
}
