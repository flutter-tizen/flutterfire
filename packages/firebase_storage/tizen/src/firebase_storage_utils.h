// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_FIREBASE_STORAGE_UTILS_H_
#define FLUTTER_PLUGIN_FIREBASE_STORAGE_UTILS_H_

#include <flutter/encodable_value.h>

#include <string>

#include "firebase/storage.h"
#include "flutter_types.hpp"

namespace utils {

struct StorageTaskData {
  int handle;
  std::string app_name;
  std::string path;
  std::string bucket;
};

firebase::storage::Storage* GetStorage(MethodCallArguments* args);

firebase::storage::StorageReference GetStorageReference(
    MethodCallArguments* args);

bool StringToByteData(const std::string& input, std::string* output,
                      int format);

firebase::storage::Metadata ParseMetadata(const flutter::EncodableMap& value);

flutter::EncodableValue GetMetadataValue(
    const firebase::storage::Metadata* metadata);

flutter::EncodableValue GetTaskEventValue(const StorageTaskData& data);
flutter::EncodableValue GetTaskEventValue(const StorageTaskData& data,
                                          const int64_t bytes_transferred,
                                          const int64_t total_bytes);

flutter::EncodableValue GetTaskControlEventValue(
    const bool status, const std::string& path,
    const firebase::storage::Controller* controller);

flutter::EncodableValue GetPutTaskSuccessEventValue(
    const StorageTaskData& data, const firebase::storage::Metadata* result);

flutter::EncodableValue GetTaskErrorEventValue(const StorageTaskData& data,
                                               const int error_code,
                                               const char* error_message);

flutter::EncodableValue ParseListResult(
    const firebase::storage::ListResult& list_result);

}  // namespace utils

#endif
