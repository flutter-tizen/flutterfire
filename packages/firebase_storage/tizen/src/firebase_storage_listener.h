// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_FIREBASE_STORAGE_LISTENER_H_
#define FLUTTER_PLUGIN_FIREBASE_STORAGE_LISTENER_H_

#include "firebase/storage.h"
#include "firebase_storage_utils.h"
#include "flutter_types.hpp"

class StorageListener : public firebase::storage::Listener {
 public:
  StorageListener(const utils::StorageTaskData& task_data,
                  const std::shared_ptr<FlMethodChannel> channel);

  void OnPaused(firebase::storage::Controller* controller) override;
  void OnProgress(firebase::storage::Controller* controller) override;

  const utils::StorageTaskData& GetStorageTaskData() { return task_data_; }

 private:
  utils::StorageTaskData task_data_;
  std::shared_ptr<FlMethodChannel> channel_;
};

#endif
