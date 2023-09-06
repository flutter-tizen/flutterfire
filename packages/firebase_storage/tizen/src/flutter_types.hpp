// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_FLUTTER_ARGUMENTS_H_
#define FLUTTER_PLUGIN_FLUTTER_ARGUMENTS_H_

#include <flutter/encodable_value.h>
#include <flutter/method_channel.h>
#include <flutter/method_result.h>

#include <optional>
#include <cassert>

typedef flutter::MethodChannel<flutter::EncodableValue> FlMethodChannel;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;

class MethodCallArguments {
public:
  MethodCallArguments(const flutter::EncodableMap *arguments)
      : arguments_(arguments) {}

  template <typename T> std::optional<T> GetArg(const char *key) {
    assert(arguments_);

    auto iter = arguments_->find(flutter::EncodableValue(key));
    if (iter != arguments_->end() && !iter->second.IsNull()) {
      if (auto *value = std::get_if<T>(&iter->second)) {
        return *value;
      }
    }
    return std::nullopt;
  }

  template <typename T> T GetRequiredArg(const char *key) {
    assert(arguments_);
    
    auto value = GetArg<T>(key);
    if (value.has_value()) {
      return value.value();
    }
    std::string message =
        "No " + std::string(key) + " provided or has invalid type or value.";
    throw std::invalid_argument(message);
  }

  std::string GetKeyString() {
    assert(arguments_);

    std::string argument_keys;
    for (const auto &[key, value] : *arguments_) {
      argument_keys += std::get<std::string>(key);
      argument_keys += ", ";
    }

    argument_keys.erase(argument_keys.size() - 2);
    return argument_keys;
  }

private:
  const flutter::EncodableMap *arguments_;
};

#endif
