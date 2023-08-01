// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_FIREBASE_STORAGE_TASK_H_
#define FLUTTER_PLUGIN_FIREBASE_STORAGE_TASK_H_

#include <flutter/encodable_value.h>

#include <mutex>
#include <optional>

#include "firebase_storage_listener.h"
#include "firebase_storage_utils.h"
#include "flutter_types.hpp"

class StorageTask;
class StoragePutBytesTask;

class StorageTaskHandler {
 public:
  static StorageTaskHandler& GetInstance();

  void AddTask(int handle, std::unique_ptr<StorageTask> task);

  std::optional<StorageTask*> GetTask(int handle);

  void RemoveTask(int handle);

 private:
  StorageTaskHandler() = default;
  StorageTaskHandler(const StorageTaskHandler&) = delete;
  void operator=(const StorageTaskHandler&) = delete;

  std::unordered_map<int, std::unique_ptr<StorageTask>> tasks_;
  std::mutex mutex_;
};

class StorageTask {
 public:
  enum Type {
    kNone,
    kPutData,
    kPutString,
    kPutFile,
    kWriteToFile,
  };

  virtual ~StorageTask() = default;

  template <class T>
  static T* Create(const std::shared_ptr<FlMethodChannel> channel,
                   std::unique_ptr<MethodCallArguments>&& args) {
    auto task = std::make_unique<T>(channel, std::move(args));
    auto instance = task.get();
    StorageTaskHandler::GetInstance().AddTask(task->GetHandle(),
                                              std::move(task));
    return instance;
  }

  const int GetHandle() { return listener_->GetStorageTaskData().handle; }

  std::string GetAppName() { return listener_->GetStorageTaskData().app_name; }

  std::string GetPath() { return listener_->GetStorageTaskData().path; }

  std::string GetBucket() { return listener_->GetStorageTaskData().bucket; }

  const utils::StorageTaskData& GetStorageTaskData() {
    return listener_->GetStorageTaskData();
  }

  firebase::storage::Controller* GetController() { return &controller_; }

  StorageListener* GetListener() { return listener_.get(); }

  const char* GetTaskName();

  FlMethodChannel* GetMethodChannel() { return channel_.get(); }

  void Complete();

  void Success(const flutter::EncodableValue& result);

  void Fail(const flutter::EncodableValue& result, const char* error_message);

  virtual void Run() = 0;

 protected:
  StorageTask(Type type, const std::shared_ptr<FlMethodChannel> channel,
              std::unique_ptr<MethodCallArguments>&& args);

  Type type_;
  std::shared_ptr<FlMethodChannel> channel_;
  std::unique_ptr<MethodCallArguments> method_args_;

  firebase::storage::StorageReference storage_reference_;
  firebase::storage::Controller controller_;
  std::unique_ptr<StorageListener> listener_;
};

class StoragePutTask : public StorageTask {
 public:
  StoragePutTask(Type type, const std::shared_ptr<FlMethodChannel> channel,
                 std::unique_ptr<MethodCallArguments>&& args)
      : StorageTask(type, channel, std::move(args)) {}

  void Run() override;
  virtual firebase::Future<firebase::storage::Metadata> RunTaskImpl() = 0;
};

class StoragePutDataTask final : public StoragePutTask {
 public:
  StoragePutDataTask(const std::shared_ptr<FlMethodChannel> channel,
                     std::unique_ptr<MethodCallArguments>&& args)
      : StoragePutTask(kPutData, channel, std::move(args)) {}

  virtual ~StoragePutDataTask() = default;

  firebase::Future<firebase::storage::Metadata> RunTaskImpl() override;

 private:
  std::vector<uint8_t> buffer_;
};

class StoragePutStringTask final : public StoragePutTask {
 public:
  StoragePutStringTask(const std::shared_ptr<FlMethodChannel> channel,
                       std::unique_ptr<MethodCallArguments>&& args)
      : StoragePutTask(kPutString, channel, std::move(args)) {}

  virtual ~StoragePutStringTask() = default;

  firebase::Future<firebase::storage::Metadata> RunTaskImpl() override;

 private:
  std::string buffer_;
};

class StoragePutFileTask final : public StoragePutTask {
 public:
  StoragePutFileTask(const std::shared_ptr<FlMethodChannel> channel,
                     std::unique_ptr<MethodCallArguments>&& args)
      : StoragePutTask(kPutFile, channel, std::move(args)) {}

  virtual ~StoragePutFileTask() = default;

  firebase::Future<firebase::storage::Metadata> RunTaskImpl() override;
};

class StorageWriteToFileTask final : public StorageTask {
 public:
  StorageWriteToFileTask(const std::shared_ptr<FlMethodChannel> channel,
                         std::unique_ptr<MethodCallArguments>&& args)
      : StorageTask(kWriteToFile, channel, std::move(args)) {}

  void Run() override;
};

#endif
