// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "firebase_storage_task.h"

#include <flutter/method_channel.h>

#include <memory>
#include <string>
#include <thread>

#include "firebase/app.h"
#include "firebase/storage.h"
#include "firebase_storage_error.h"
#include "log.h"

StorageTaskHandler& StorageTaskHandler::GetInstance() {
  static StorageTaskHandler instance;
  return instance;
}

void StorageTaskHandler::AddTask(int handle,
                                 std::unique_ptr<StorageTask> task) {
  std::lock_guard<std::mutex> lock(mutex_);
  tasks_[handle] = std::move(task);
}

std::optional<StorageTask*> StorageTaskHandler::GetTask(int handle) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = tasks_.find(handle);
  if (it == tasks_.end()) {
    return std::nullopt;
  }
  return (it->second).get();
}

void StorageTaskHandler::RemoveTask(int handle) {
  std::lock_guard<std::mutex> lock(mutex_);
  tasks_.erase(handle);
}

StorageTask::StorageTask(Type type,
                         const std::shared_ptr<FlMethodChannel> channel,
                         std::unique_ptr<MethodCallArguments>&& args)
    : type_(type), channel_(std::move(channel)), method_args_(std::move(args)) {
  auto path = method_args_->GetRequiredArg<std::string>("path");

  storage_reference_ =
      utils::GetStorage(method_args_.get())->GetReference(path);
  utils::StorageTaskData task_data{
      method_args_->GetRequiredArg<int>("handle"),
      method_args_->GetRequiredArg<std::string>("appName"), path,
      storage_reference_.bucket()};
  listener_ = std::make_unique<StorageListener>(task_data, channel);
}

const char* StorageTask::GetTaskName() {
  switch (type_) {
    case kPutData:
      return "Put-Data";
    case kPutString:
      return "Put-String";
    case kPutFile:
      return "Put-File";
    case kWriteToFile:
      return "Write-To-File";
    case kNone:
    default:
      return "None";
  }
}

void StorageTask::Complete() {
  StorageTaskHandler::GetInstance().RemoveTask(GetHandle());
}

void StorageTask::Success(const flutter::EncodableValue& result) {
  channel_->InvokeMethod("Task#onSuccess",
                         std::make_unique<flutter::EncodableValue>(result));
}

void StorageTask::Fail(const flutter::EncodableValue& result,
                       const char* error_message) {
  LOG_ERROR("Fail %s: %s", GetTaskName(), error_message);

  channel_->InvokeMethod("Task#onFailure",
                         std::make_unique<flutter::EncodableValue>(result));
}

void StoragePutTask::Run() {
  RunTaskImpl().OnCompletion(
      [](const firebase::Future<firebase::storage::Metadata>& metadata,
         void* userdata) {
        auto task = static_cast<StoragePutTask*>(userdata);
        if (metadata.error() == firebase::storage::Error::kErrorNone) {
          task->Success(utils::GetPutTaskSuccessEventValue(
              task->GetStorageTaskData(), metadata.result()));
        } else {
          task->Fail(utils::GetTaskErrorEventValue(task->GetStorageTaskData(),
                                                   metadata.error(),
                                                   metadata.error_message()),
                     metadata.error_message());
        }

        task->Complete();
      },
      this);
}

firebase::Future<firebase::storage::Metadata>
StoragePutDataTask::RunTaskImpl() {
  buffer_ = method_args_->GetRequiredArg<std::vector<uint8_t>>("data");

  auto metadata_value = method_args_->GetArg<flutter::EncodableMap>("metadata");
  if (metadata_value.has_value()) {
    return storage_reference_.PutBytes(
        buffer_.data(), buffer_.size(),
        utils::ParseMetadata(metadata_value.value()), GetListener(),
        GetController());
  } else {
    return storage_reference_.PutBytes(buffer_.data(), buffer_.size(),
                                       GetListener(), GetController());
  }
}

firebase::Future<firebase::storage::Metadata>
StoragePutStringTask::RunTaskImpl() {
  auto format = method_args_->GetRequiredArg<int>("format");
  auto data = method_args_->GetRequiredArg<std::string>("data");

  if (!utils::StringToByteData(data, &buffer_, format)) {
    throw FirebaseStorageError(FirebaseStorageError::Code::kInvalidString,
                               "Fail to decode the input string.");
  }

  auto metadata_value = method_args_->GetArg<flutter::EncodableMap>("metadata");
  if (metadata_value.has_value()) {
    return storage_reference_.PutBytes(
        buffer_.data(), buffer_.size(),
        utils::ParseMetadata(metadata_value.value()), GetListener(),
        GetController());
  } else {
    return storage_reference_.PutBytes(buffer_.data(), buffer_.size(),
                                       GetListener(), GetController());
  }
}

firebase::Future<firebase::storage::Metadata>
StoragePutFileTask::RunTaskImpl() {
  auto file_path = method_args_->GetRequiredArg<std::string>("filePath");

  auto metadata_value = method_args_->GetArg<flutter::EncodableMap>("metadata");
  if (metadata_value.has_value()) {
    return storage_reference_.PutFile(
        file_path.data(), utils::ParseMetadata(metadata_value.value()),
        GetListener(), GetController());
  } else {
    return storage_reference_.PutFile(file_path.data(), GetListener(),
                                      GetController());
  }
}

void StorageWriteToFileTask::Run() {
  auto file_path = method_args_->GetRequiredArg<std::string>("filePath");

  storage_reference_.GetFile(file_path.data(), GetListener(), GetController())
      .OnCompletion(
          [](const firebase::Future<std::size_t>& metadata, void* userdata) {
            auto task = static_cast<StorageWriteToFileTask*>(userdata);

            if (metadata.error() == firebase::storage::Error::kErrorNone) {
              task->Success(utils::GetTaskEventValue(task->GetStorageTaskData(),
                                                     *metadata.result(),
                                                     *metadata.result()));
            } else {
              task->Fail(utils::GetTaskErrorEventValue(
                             task->GetStorageTaskData(), metadata.error(),
                             metadata.error_message()),
                         metadata.error_message());
            }

            task->Complete();
          },
          this);
}
