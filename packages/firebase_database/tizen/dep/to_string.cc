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

#include "include/common/to_string.h"

using firebase::Variant;
using flutter::EncodableList;
using flutter::EncodableMap;
using flutter::EncodableValue;

std::ostream& operator<<(std::ostream& os, const Variant& v) {
  switch (v.type()) {
    case Variant::kTypeNull:
      os << "null";
      break;
    case Variant::kTypeInt64:
      os << v.int64_value();
      break;
    case Variant::kTypeDouble:
      os << v.double_value();
      break;
    case Variant::kTypeBool:
      os << std::boolalpha << v.bool_value() << std::noboolalpha;
      break;
    case Variant::kTypeStaticString:
      os << v.string_value();
      break;
    case Variant::kTypeMutableString:
      os << v.mutable_string();
      break;
    case Variant::kTypeVector:
      os << v.vector();
      break;
    case Variant::kTypeMap:
      os << v.map();
      break;
    default:
      os << "<unknown Variant type>";
      break;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const EncodableValue& v) {
  switch (v.index()) {
    case 0:  // std::monostate
      os << "null";
      break;
    case 1:  // bool
      os << std::boolalpha << std::get<bool>(v) << std::noboolalpha;
      break;
    case 2:  // int32_t
      os << std::get<int32_t>(v);
      break;
    case 3:  // int64_t
      os << std::get<int64_t>(v);
      break;
    case 4:  // double
      os << std::get<double>(v);
      break;
    case 5:  // std::string
      os << "\"" << std::get<std::string>(v) << "\"";
      break;
    case 6:  // std::vector<uint8_t>
      os << std::get<std::vector<uint8_t>>(v);
      break;
    case 7:  // std::vector<int32_t>
      os << std::get<std::vector<int32_t>>(v);
      break;
    case 8:  // std::vector<int64_t>
      os << std::get<std::vector<int64_t>>(v);
      break;
    case 9:  // std::vector<double>
      os << std::get<std::vector<double>>(v);
      break;
    case 10: {  // EncodableList
      os << std::get<EncodableList>(v);
      break;
    }
    case 11: {  // EncodableMap
      os << std::get<EncodableMap>(v);
      break;
    }
    case 12:  // CustomEncodableValue
      os << "<unsupported type>";
      break;
    case 13:  // std::vector<float>
      os << std::get<std::vector<float>>(v);
      break;
    default:
      os << "<unknown EncodableValue type>";
      break;
  }
  return os;
}

#ifdef FIREBASE_DATABASE

using firebase::database::DataSnapshot;
using firebase::database::MutableData;

std::ostream& operator<<(std::ostream& os, const DataSnapshot& snapshot) {
  static thread_local std::string indent;

  os << indent << "{\n";
  indent += "  ";

  os << indent << "key: " << snapshot.key_string() << ", \n"
     << indent << "value: " << snapshot.value() << ", \n"
     << indent << "priority: " << snapshot.priority() << ", \n"
     << indent << "children_count: " << snapshot.children_count() << ", \n";

  for (const auto& s : snapshot.children()) {
    os << s;
  }

  indent = indent.substr(0, indent.length() - 2);
  os << indent << "},\n";
  return os;
}

std::ostream& operator<<(std::ostream& os, const MutableData& data) {
  static thread_local std::string indent;

  os << indent << "{\n";
  indent += "  ";

  // Functions like children_count() or priority() is not marked as const. This
  // seems to be a firebase mistake. So we convert it to a const reference.
  MutableData& mutable_data = const_cast<MutableData&>(data);

  os << indent << "key: " << mutable_data.key_string() << ", \n"
     << indent << "value: " << mutable_data.value() << ", \n"
     << indent << "priority: " << mutable_data.priority() << ", \n"
     << indent << "children_count: " << mutable_data.children_count() << ", \n";

  for (auto& d : mutable_data.children()) {
    os << d;
  }

  indent = indent.substr(0, indent.length() - 2);
  os << indent << "},\n";
  return os;
}

#endif  // FIREBASE_DATABASE
