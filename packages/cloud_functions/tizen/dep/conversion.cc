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

#include "common/conversion.h"

#include <string>

#include "common/trace.h"  // for UNIMPLEMENTED and FATAL
#include "common/utils.h"

using firebase::Variant;
using flutter::EncodableList;
using flutter::EncodableMap;
using flutter::EncodableValue;

Variant Conversion::ToFirebaseVariant(const EncodableValue& encodable_value) {
  switch (encodable_value.index()) {
    case 0:  // std::monostate
      return Variant();
    case 1:  // bool
      return Variant(std::get<bool>(encodable_value));
    case 2:  // int32_t
      return Variant(std::get<int32_t>(encodable_value));
    case 3:  // int64_t
      return Variant(std::get<int64_t>(encodable_value));
    case 4:  // double
      return Variant(std::get<double>(encodable_value));
    case 5:  // std::string
      return Variant(std::get<std::string>(encodable_value));
    case 6:  // std::vector<uint8_t>
      return Variant(std::get<std::vector<uint8_t>>(encodable_value));
    case 7:  // std::vector<int32_t>
      return Variant(std::get<std::vector<int32_t>>(encodable_value));
    case 8:  // std::vector<int64_t>
      return Variant(std::get<std::vector<int64_t>>(encodable_value));
    case 9:  // std::vector<double>
      return Variant(std::get<std::vector<double>>(encodable_value));
    case 10:  // EncodableList
      return Conversion::ToFirebaseVariant(
          std::get<EncodableList>(encodable_value));
    case 11:  // EncodableMap
      return Conversion::ToFirebaseVariant(
          std::get<EncodableMap>(encodable_value));
    case 12:  // CustomEncodableValue
      UNIMPLEMENTED("Unknown to handle this");
      return Variant();
    case 13:  // std::vector<float>
      return Variant(std::get<std::vector<float>>(encodable_value));
    default:
      FATAL("Invalid EncodableValue type");
  }
  return Variant();
}

Variant Conversion::ToFirebaseVariant(const EncodableList& encodable_list) {
  std::vector<Variant> variant_list;
  for (const EncodableValue& encodable_value : encodable_list) {
    variant_list.push_back(Conversion::ToFirebaseVariant(encodable_value));
  }
  return Variant(variant_list);
}

Variant Conversion::ToFirebaseVariant(const EncodableMap& encodable_map) {
  std::map<Variant, Variant> variant_map;
  for (const auto& [key, value] : encodable_map) {
    variant_map.emplace(Conversion::ToFirebaseVariant(key),
                        Conversion::ToFirebaseVariant(value));
  }
  return Variant(variant_map);
}

Variant Conversion::ToFirebaseVariant(const EncodableMap* map,
                                      const char* key) {
  return Conversion::ToFirebaseVariant(GetEncodableValue(map, key));
}

EncodableValue Conversion::ToEncodableValue(const Variant& v) {
  switch (v.type()) {
    case Variant::kTypeNull:
      return EncodableValue(std::monostate());
    case Variant::kTypeInt64:
      return EncodableValue(v.int64_value());
    case Variant::kTypeDouble:
      return EncodableValue(v.double_value());
    case Variant::kTypeBool:
      return EncodableValue(v.bool_value());
    case Variant::kTypeStaticString:
      return EncodableValue(std::string(v.string_value()));
    case Variant::kTypeMutableString:
      return EncodableValue(v.mutable_string());
    case Variant::kTypeVector: {
      EncodableList list;
      for (const auto& e : v.vector()) {
        list.push_back(ToEncodableValue(e));
      }
      return EncodableValue(list);
    }
    case Variant::kTypeMap: {
      EncodableMap map;
      for (const auto& [key, value] : v.map()) {
        map[ToEncodableValue(key)] = ToEncodableValue(value);
      }
      return EncodableValue(map);
    }
    default:
      FATAL("Unsupported Variant type");
  }
  return EncodableValue();
}
