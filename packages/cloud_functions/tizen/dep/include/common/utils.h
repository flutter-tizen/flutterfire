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
#pragma once

#include <flutter/encodable_value.h>

#include <optional>
#include <string>
#include <variant>

template <typename T>
class PointerScope {
 public:
  PointerScope() = delete;
  PointerScope(void* p) { value_ = reinterpret_cast<T*>(p); }
  virtual ~PointerScope() { delete value_; }
  T* operator->() { return value_; }
  T* operator*() { return value_; }

 private:
  T* value_;
};

// EncodableMap

template <typename T>
std::optional<T> GetOptionalValue(const flutter::EncodableMap* map,
                                  const char* key) {
  const auto& iter = map->find(flutter::EncodableValue(key));
  if (iter != map->end() && !iter->second.IsNull()) {
    if (auto* value = std::get_if<T>(&iter->second)) {
      return *value;
    }
  }
  return std::nullopt;
}

flutter::EncodableValue GetEncodableValue(const flutter::EncodableMap* map,
                                          const char* key);
