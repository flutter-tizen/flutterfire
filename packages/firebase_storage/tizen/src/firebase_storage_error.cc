// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "firebase_storage_error.h"

std::string FirebaseStorageError::GetCodeString() const {
  switch (code_) {
    case Code::kObjectNotFound:
      return "object-not-found";
    case Code::kBucketNotFound:
      return "bucket-not-found";
    case Code::kProjectNotFound:
      return "project-not-found";
    case Code::kQuotaExceeded:
      return "quota-exceeded";
    case Code::kUnauthenticated:
      return "unauthenticated";
    case Code::kUnauthorized:
      return "unauthorized";
    case Code::kRetryLimitExceeded:
      return "retry-limit-exceeded";
    case Code::kNonMatchingChecksum:
      return "invalid-checksum";
    case Code::kDownloadSizeExceeded:
      return "download-size-exceeded";
    case Code::kCancelled:
      return "canceled";
    case Code::kInvalidArgument:
      return "invalid-argument";
    case Code::kAppNotFound:
      return "app-not-found";
    case Code::kNotSupported:
      return "not-supported";
    case Code::kInvalidString:
      return "invalid-string";
    case Code::kSystemError:
      return "system-error";
    case Code::KTaskNotFound:
      return "task-not-found";
    case Code::kUnknown:
    default:
      return "unknown";
  }
}

std::string FirebaseStorageError::GetErrorString() const {
  switch (code_) {
    case Code::kUnknown:
    case Code::kObjectNotFound:
    case Code::kBucketNotFound:
    case Code::kProjectNotFound:
    case Code::kQuotaExceeded:
    case Code::kUnauthenticated:
    case Code::kUnauthorized:
    case Code::kRetryLimitExceeded:
    case Code::kNonMatchingChecksum:
    case Code::kDownloadSizeExceeded:
    case Code::kCancelled:
      return firebase::storage::GetErrorMessage(
          firebase::storage::Error(code_));
    case Code::kInvalidArgument:
      return "Invalid argument.";
    case Code::kAppNotFound:
      return "No app is configured with that name.";
    case Code::kNotSupported:
      return "The feature is not supported.";
    case Code::kInvalidString:
      return "The string contains invalid characters.";
    case Code::kSystemError:
      return "A system error occurred.";
    case Code::KTaskNotFound:
      return "A task does not exist.";
    default:
      return "An unknown error occurred.";
  }
}
