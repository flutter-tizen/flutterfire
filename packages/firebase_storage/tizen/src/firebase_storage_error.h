// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_FIREBASE_STORAGE_ERROR_H_
#define FLUTTER_PLUGIN_FIREBASE_STORAGE_ERROR_H_

#include <firebase/storage/common.h>

#include <string>

using namespace firebase::storage;

class FirebaseStorageError {
 public:
  enum class Code {
    kUnknown = Error::kErrorUnknown,
    kObjectNotFound = Error::kErrorObjectNotFound,
    kBucketNotFound = Error::kErrorBucketNotFound,
    kProjectNotFound = Error::kErrorProjectNotFound,
    kQuotaExceeded = Error::kErrorQuotaExceeded,
    kUnauthenticated = Error::kErrorUnauthenticated,
    kUnauthorized = Error::kErrorUnauthorized,
    kRetryLimitExceeded = Error::kErrorRetryLimitExceeded,
    kNonMatchingChecksum = Error::kErrorNonMatchingChecksum,
    kDownloadSizeExceeded = Error::kErrorDownloadSizeExceeded,
    kCancelled = Error::kErrorCancelled,
    kInvalidArgument = 20,
    kAppNotFound,
    kNotSupported,
    kInvalidString,
    kSystemError,
    KTaskNotFound,
  };

  FirebaseStorageError(Code code, const std::string& message)
      : code_(code), message_(message) {}

  FirebaseStorageError(Code code) : code_(code), message_(GetErrorString()) {}

  FirebaseStorageError(int code)
      : code_(FirebaseStorageError::Code(code)), message_(GetErrorString()) {}

  FirebaseStorageError(const FirebaseStorageError& other) {
    this->code_ = other.code_;
    this->message_ = other.message_;
  }

  ~FirebaseStorageError() = default;

  FirebaseStorageError& operator=(const FirebaseStorageError& other) {
    this->code_ = other.code_;
    this->message_ = other.message_;
    return *this;
  }

  Code GetCode() const { return code_; }
  std::string GetMessage() const { return message_; }
  std::string GetCodeString() const;

 private:
  Code code_;
  std::string message_;

  std::string GetErrorString() const;
};

#endif  // FLUTTER_PLUGIN_FIREBASE_STORAGE_ERROR_H_
