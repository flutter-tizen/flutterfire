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

#include <firebase/variant.h>
#include <flutter/encodable_value.h>

#include <sstream>

std::ostream& operator<<(std::ostream& os, const firebase::Variant& v);
std::ostream& operator<<(std::ostream& os, const flutter::EncodableValue& v);

#ifdef FIREBASE_DATABASE
#include <firebase/database/data_snapshot.h>
#include <firebase/database/mutable_data.h>

std::ostream& operator<<(std::ostream& os,
                         const firebase::database::DataSnapshot& d);
std::ostream& operator<<(std::ostream& os,
                         const firebase::database::MutableData& d);
#endif  // FIREBASE_DATABASE

template <typename T>
static std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
  static size_t limit = std::numeric_limits<size_t>::max();

  size_t n = v.size();
  size_t end = std::min(n, limit);
  os << "[";
  for (size_t i = 0; i < end; i++) {
    os << v[i];
    if (i < end - 1) {
      os << ", ";
    }
  }
  if (end < n) {
    os << ", ...";
  }
  os << "]";
  return os;
}

template <typename K, typename V>
static std::ostream& operator<<(std::ostream& os, const std::map<K, V>& m) {
  os << "{ ";
  for (const auto& [key, value] : m) {
    os << key << ": " << value << ", ";
  }
  os << "}";
  return os;
}

template <typename T>
std::string ToString(const T& value) {
  std::stringstream ss;
  ss << value;
  return ss.str();
}
