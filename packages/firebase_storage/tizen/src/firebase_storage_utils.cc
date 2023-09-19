// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "firebase_storage_utils.h"

#include <memory>
#include <sstream>
#include <string>

#include "firebase/app/src/base64.h"
#include "firebase/storage/controller.h"
#include "firebase_storage_error.h"

namespace utils {

firebase::storage::Storage* GetStorage(MethodCallArguments* args) {
  auto appName = args->GetRequiredArg<std::string>("appName");
  firebase::App* app = firebase::App::GetInstance(appName.data());
  if (!app) {
    throw FirebaseStorageError(FirebaseStorageError::Code::kAppNotFound);
  }

  firebase::storage::Storage* storage = nullptr;
  auto bucket = args->GetArg<std::string>("bucket");

  if (bucket.has_value()) {
    std::string url("gs://" + bucket.value());
    storage = firebase::storage::Storage::GetInstance(app, url.data());
  } else {
    storage = firebase::storage::Storage::GetInstance(app);
  }

  auto max_operation_retry_time = args->GetArg<double>("maxOperationRetryTime");
  if (max_operation_retry_time.has_value()) {
    storage->set_max_operation_retry_time(max_operation_retry_time.value());
  }

  auto max_download_retry_time = args->GetArg<double>("maxDownloadRetryTime");
  if (max_download_retry_time.has_value()) {
    storage->set_max_download_retry_time(max_download_retry_time.value());
  }

  auto max_upload_retry_time = args->GetArg<double>("maxUploadRetryTime");
  if (max_upload_retry_time.has_value()) {
    storage->set_max_upload_retry_time(max_upload_retry_time.value());
  }

  return storage;
}

firebase::storage::StorageReference GetStorageReference(
    MethodCallArguments* args) {
  return GetStorage(args)->GetReference(
      args->GetRequiredArg<std::string>("path"));
}

bool StringToByteData(const std::string& input, std::string* output,
                      int format) {
  switch (format) {
    case 1:  // PutStringFormat.base64
      return firebase::internal::Base64Decode(input, output);
    default:
      std::ostringstream os;
      os << "This format(" << format << ") is not supported yet.";
      throw FirebaseStorageError(FirebaseStorageError::Code::kNotSupported,
                                 os.str());
  }

  return false;
}

firebase::storage::Metadata ParseMetadata(const flutter::EncodableMap& value) {
  MethodCallArguments metadata(&value);
  firebase::storage::Metadata out;

  auto cache_control = metadata.GetArg<std::string>("cacheControl");
  if (cache_control.has_value()) {
    out.set_cache_control(cache_control.value());
  }

  auto content_disposition = metadata.GetArg<std::string>("contentDisposition");
  if (content_disposition.has_value()) {
    out.set_content_disposition(content_disposition.value());
  }

  auto content_encoding = metadata.GetArg<std::string>("contentEncoding");
  if (content_encoding.has_value()) {
    out.set_content_encoding(content_encoding.value());
  }

  auto content_language = metadata.GetArg<std::string>("contentLanguage");
  if (content_language.has_value()) {
    out.set_content_language(content_language.value());
  }

  auto content_type = metadata.GetArg<std::string>("contentType");
  if (content_type.has_value()) {
    out.set_content_type(content_type.value());
  }

  auto custom_metadata =
      metadata.GetArg<flutter::EncodableMap>("customMetadata");
  if (custom_metadata.has_value()) {
    for (const auto& [key, value] : custom_metadata.value()) {
      if (std::holds_alternative<std::string>(key) &&
          std::holds_alternative<std::string>(value)) {
        (*out.custom_metadata())[std::get<std::string>(key)] =
            std::get<std::string>(value);
      }
    }
  }

  return out;
}

flutter::EncodableValue GetMetadataValue(
    const firebase::storage::Metadata* metadata) {
  auto metadata_map = flutter::EncodableMap{
      {flutter::EncodableValue("bucket"),
       flutter::EncodableValue(metadata->bucket())},
      {flutter::EncodableValue("cacheControl"),
       flutter::EncodableValue(metadata->cache_control())},
      {flutter::EncodableValue("contentDisposition"),
       flutter::EncodableValue(metadata->content_disposition())},
      {flutter::EncodableValue("contentEncoding"),
       flutter::EncodableValue(metadata->content_encoding())},
      {flutter::EncodableValue("contentLanguage"),
       flutter::EncodableValue(metadata->content_language())},
      {flutter::EncodableValue("contentType"),
       flutter::EncodableValue(metadata->content_type())},
      {flutter::EncodableValue("fullPath"),
       flutter::EncodableValue(metadata->path())},
      {flutter::EncodableValue("generation"),
       flutter::EncodableValue(metadata->generation())},
      {flutter::EncodableValue("metadataGeneration"),
       flutter::EncodableValue(metadata->metadata_generation())},
      {flutter::EncodableValue("md5Hash"),
       flutter::EncodableValue(metadata->md5_hash())},
      {flutter::EncodableValue("metageneration"),
       flutter::EncodableValue(metadata->metadata_generation())},
      {flutter::EncodableValue("name"),
       flutter::EncodableValue(metadata->name())},
      {flutter::EncodableValue("size"),
       flutter::EncodableValue(metadata->size_bytes())},
      {flutter::EncodableValue("creationTimeMillis"),
       flutter::EncodableValue(metadata->creation_time())},
      {flutter::EncodableValue("updatedTimeMillis"),
       flutter::EncodableValue(metadata->updated_time())},
  };

  flutter::EncodableMap custom_metadata;
  for (const auto& [key, value] : *(metadata->custom_metadata())) {
    custom_metadata[flutter::EncodableValue(key)] =
        flutter::EncodableValue(value);
  }
  metadata_map[flutter::EncodableValue("customMetadata")] =
      flutter::EncodableValue(custom_metadata);

  return flutter::EncodableValue(metadata_map);
}

static flutter::EncodableMap GetTaskEventMap(const int handle,
                                             const std::string& name,
                                             const std::string& bucket) {
  return flutter::EncodableMap{
      {flutter::EncodableValue("handle"), flutter::EncodableValue(handle)},
      {flutter::EncodableValue("appName"), flutter::EncodableValue(name)},
      {flutter::EncodableValue("bucket"), flutter::EncodableValue(bucket)}};
}

static flutter::EncodableMap GetSnapshotMap(const std::string& path,
                                            const int64_t bytes_transferred,
                                            const int64_t total_bytes) {
  return flutter::EncodableMap{
      {flutter::EncodableValue("path"), flutter::EncodableValue(path)},
      {flutter::EncodableValue("bytesTransferred"),
       flutter::EncodableValue(bytes_transferred)},
      {flutter::EncodableValue("totalBytes"),
       flutter::EncodableValue(total_bytes)}};
}

flutter::EncodableValue GetTaskEventValue(const StorageTaskData& data) {
  return flutter::EncodableValue(
      GetTaskEventMap(data.handle, data.app_name, data.bucket));
}

flutter::EncodableValue GetTaskEventValue(const StorageTaskData& data,
                                          int64_t bytes_transferred,
                                          int64_t total_bytes) {
  flutter::EncodableMap map =
      GetTaskEventMap(data.handle, data.app_name, data.bucket);

  flutter::EncodableMap snapshot =
      GetSnapshotMap(data.path, bytes_transferred, total_bytes);

  map[flutter::EncodableValue("snapshot")] = flutter::EncodableValue(snapshot);

  return flutter::EncodableValue(map);
}

flutter::EncodableValue GetTaskControlEventValue(
    const bool status, const std::string& path,
    const firebase::storage::Controller* controller) {
  auto map = flutter::EncodableMap{
      {flutter::EncodableValue("status"), flutter::EncodableValue(status)},
  };
  if (status) {
    flutter::EncodableMap snapshot = GetSnapshotMap(
        path, controller->bytes_transferred(), controller->total_byte_count());

    map[flutter::EncodableValue("snapshot")] =
        flutter::EncodableValue(snapshot);
  }

  return flutter::EncodableValue(map);
}

flutter::EncodableValue GetPutTaskSuccessEventValue(
    const StorageTaskData& data, const firebase::storage::Metadata* result) {
  flutter::EncodableMap map =
      GetTaskEventMap(data.handle, data.app_name, data.bucket);

  flutter::EncodableMap metadata{
      {flutter::EncodableValue("generation"),
       flutter::EncodableValue(result->generation())},
      {flutter::EncodableValue("fullPath"),
       flutter::EncodableValue(result->GetReference().full_path())},
      {flutter::EncodableValue("cacheControl"),
       flutter::EncodableValue(result->cache_control())},
      {flutter::EncodableValue("bucket"),
       flutter::EncodableValue(result->bucket())},
      {flutter::EncodableValue("metadataGeneration"),
       flutter::EncodableValue(result->metadata_generation())},
      {flutter::EncodableValue("updatedTimeMillis"),
       flutter::EncodableValue(result->updated_time())},
      {flutter::EncodableValue("size"),
       flutter::EncodableValue(result->size_bytes())},
      {flutter::EncodableValue("md5Hash"),
       flutter::EncodableValue(result->md5_hash())},
      {flutter::EncodableValue("creationTimeMillis"),
       flutter::EncodableValue(result->creation_time())},
      {flutter::EncodableValue("contentDisposition"),
       flutter::EncodableValue(result->content_disposition())},
      {flutter::EncodableValue("contentEncoding"),
       flutter::EncodableValue(result->content_encoding())},
      {flutter::EncodableValue("contentLanguage"),
       flutter::EncodableValue(result->content_language())},
      {flutter::EncodableValue("contentType"),
       flutter::EncodableValue(result->content_type())},
  };

  flutter::EncodableMap custom_metadata;
  for (const auto& [key, value] : *(result->custom_metadata())) {
    custom_metadata[flutter::EncodableValue(key)] =
        flutter::EncodableValue(value);
  }
  metadata[flutter::EncodableValue("customMetadata")] =
      flutter::EncodableValue(custom_metadata);

  flutter::EncodableMap snapshot =
      GetSnapshotMap(data.path, result->size_bytes(), result->size_bytes());
  snapshot[flutter::EncodableValue("metadata")] =
      flutter::EncodableValue(metadata);
  map[flutter::EncodableValue("snapshot")] = flutter::EncodableValue(snapshot);

  return flutter::EncodableValue(map);
}

flutter::EncodableValue GetTaskErrorEventValue(const StorageTaskData& data,
                                               const int error_code,
                                               const char* error_message) {
  flutter::EncodableMap map =
      GetTaskEventMap(data.handle, data.app_name, data.bucket);

  flutter::EncodableMap error = {
      {flutter::EncodableValue("code"), flutter::EncodableValue(error_code)},
      {flutter::EncodableValue("message"),
       flutter::EncodableValue(error_message)}};

  map[flutter::EncodableValue("error")] = flutter::EncodableValue(error);

  return flutter::EncodableValue(map);
}

flutter::EncodableValue ParseListResult(
    const firebase::storage::ListResult& list_result) {
  flutter::EncodableMap map;

  if (!list_result.GetPageToken().empty()) {
    map[flutter::EncodableValue("nextPageToken")] =
        flutter::EncodableValue(list_result.GetPageToken());
  }

  flutter::EncodableList items;
  for (StorageReference& reference : list_result.GetItems()) {
    items.push_back(flutter::EncodableValue(reference.full_path()));
  }

  flutter::EncodableList prefixes;
  for (StorageReference& reference : list_result.GetPrefixes()) {
    prefixes.push_back(flutter::EncodableValue(reference.full_path()));
  }

  map[flutter::EncodableValue("items")] = items;
  map[flutter::EncodableValue("prefixes")] = prefixes;

  return flutter::EncodableValue(map);
}

}  // namespace utils
