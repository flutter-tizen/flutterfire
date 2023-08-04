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

#include "firebase_database_utils.h"

#include <map>
#include <sstream>
#include <string>
#include <unordered_map>

#include "common/conversion.h"
#include "common/to_string.h"
#include "common/trace.h"
#include "common/utils.h"
#include "constants.h"

using firebase::App;
using firebase::InitResult;
using firebase::LogLevel;
using firebase::database::Database;
using firebase::database::DatabaseReference;
using firebase::database::DataSnapshot;
using firebase::database::MutableData;
using firebase::database::Query;
using flutter::EncodableList;
using flutter::EncodableMap;
using flutter::EncodableValue;

using EncodableValuePair = std::pair<EncodableValue, EncodableValue>;

static std::unordered_map<std::string, Database*> cached_database_map;

Database* GetDatabaseFromArguments(const EncodableMap* args) {
  CHECK_NOT_NULL(args);
  const std::string app_name =
      GetOptionalValue<std::string>(args, Constants::kAppName)
          .value_or(Constants::kDefalutAppName);
  const std::string database_url =
      GetOptionalValue<std::string>(args, Constants::kDatabaseURL).value_or("");
  const std::string instance_key = app_name + database_url;

  const auto& it = cached_database_map.find(instance_key);
  if (it != cached_database_map.end()) {
    return it->second;
  }

  App* app = App::GetInstance(app_name.c_str());
  CHECK_NOT_NULL(app);

  Database* database = nullptr;
  InitResult result;

  if (database_url.length() == 0) {
    database = Database::GetInstance(app, &result);
  } else {
    database = Database::GetInstance(app, database_url.c_str(), &result);
  }
  CHECK_NOT_NULL(database);

  if (result != InitResult::kInitResultSuccess) {
    TRACE(DATABASE, "[FAIL] Database::GetInstance with", result);
  }

  database->set_persistence_enabled(
      GetOptionalValue<bool>(args, Constants::kDatabasePersistenceEnabled)
          .value_or(false));

  if (GetOptionalValue<int32_t>(args, Constants::kDatabaseCacheSizeBytes)) {
    TRACE(DATABASE, "[!] cacheSizeBytes isn't supported.");
  }

  auto log_level = LogLevel::kLogLevelError;
  if (GetOptionalValue<bool>(args, Constants::kDatabaseLoggingEnabled)
          .value_or(false)) {
    log_level = LogLevel::kLogLevelWarning;
  }
  database->set_log_level(log_level);

  if (GetOptionalValue<int32_t>(args, Constants::kDatabaseEmulatorHost) ||
      GetOptionalValue<int32_t>(args, Constants::kDatabaseEmulatorPort)) {
    TRACE(DATABASE, "[!] Emulator isn't supported.");
  }

  cached_database_map.emplace(instance_key, database);

  return database;
}

DatabaseReference GetDatabaseReferenceFromArguments(
    const EncodableMap* arguments) {
  CHECK_NOT_NULL(arguments);
  Database* database = GetDatabaseFromArguments(arguments);

  auto path_maybe = GetOptionalValue<std::string>(arguments, Constants::kPath);
  CHECK(path_maybe);
  auto path = path_maybe.value();
  TRACE(DATABASE, "path:", path);
  return database->GetReference(path.c_str());
}

static Query ApplyOrderModifier(Query& query, const EncodableMap& modifier) {
  const auto name =
      GetOptionalValue<std::string>(&modifier, Constants::kName).value();

  if (name == Constants::kOrderByChild) {
    return query.OrderByChild(
        GetOptionalValue<std::string>(&modifier, Constants::kPath).value());
  } else if (name == Constants::kOrderByKey) {
    return query.OrderByKey();
  } else if (name == Constants::kOrderByValue) {
    return query.OrderByValue();
  } else if (name == Constants::kOrderByPriority) {
    return query.OrderByPriority();
  } else {
    TRACE(DATABASE, "[!] Unknown modifier");
  }
  return query;
}

static Query ApplyCursorModifier(Query& query, const EncodableMap& modifier) {
  const auto name =
      GetOptionalValue<std::string>(&modifier, Constants::kName).value();

  if (name == Constants::kStartAt) {
    return query.StartAt(
        Conversion::ToFirebaseVariant(&modifier, Constants::kValue));
  } else if (name == Constants::kEndAt) {
    return query.EndAt(
        Conversion::ToFirebaseVariant(&modifier, Constants::kValue));
  } else if (name == Constants::kStartAfter) {
    TRACE(DATABASE, "[!] 'startAfter' isn't supported");
  } else if (name == Constants::kEndBefore) {
    TRACE(DATABASE, "[!] 'endBefore' isn't supported");
  } else {
    TRACE(DATABASE, "[!] Unknown modifier or unimplemented");
  }
  return query;
}

static Query ApplyLimitModifier(Query& query, const EncodableMap& modifier) {
  const auto name =
      GetOptionalValue<std::string>(&modifier, Constants::kName).value();
  const auto limit = static_cast<size_t>(
      GetEncodableValue(&modifier, Constants::kLimit).LongValue());

  if (name == Constants::kLimitToFirst) {
    return query.LimitToFirst(limit);
  } else if (name == Constants::kLimitToLast) {
    return query.LimitToLast(limit);
  } else {
    TRACE(DATABASE, "[!] Unknown modifier or unimplemented");
  }
  return query;
}

Query GetDatabaseQueryFromArguments(const EncodableMap* arguments) {
  TRACE_SCOPE0(DATABASE);
  Query query = GetDatabaseReferenceFromArguments(arguments);

  auto modifiers_maybe =
      GetOptionalValue<EncodableList>(arguments, Constants::kModifiers);

  CHECK(modifiers_maybe);

  auto modifiers = modifiers_maybe.value();
  TRACE(DATABASE, "modifiers size:", modifiers.size());

  for (const EncodableValue& value : modifiers) {
    auto modifier = std::get<EncodableMap>(value);
    auto type =
        GetOptionalValue<std::string>(&modifier, Constants::kType).value();
    if (type == Constants::kOrderBy) {
      query = ApplyOrderModifier(query, modifier);
    } else if (type == Constants::kCursor) {
      query = ApplyCursorModifier(query, modifier);
    } else if (type == Constants::kLimit) {
      query = ApplyLimitModifier(query, modifier);
    }
  }

  return query;
}

EncodableMap CreateDataSnapshotPayload(const DataSnapshot* snapshot) {
  CHECK_NOT_NULL(snapshot);

  TRACE_SCOPE(DATABASE, ToString(*snapshot));

  EncodableMap map;
  map.insert(EncodableValuePair(Constants::kKey, snapshot->key_string()));
  map.insert(EncodableValuePair(
      Constants::kValue, Conversion::ToEncodableValue(snapshot->value())));
  map.insert(
      EncodableValuePair(Constants::kPriority,
                         Conversion::ToEncodableValue(snapshot->priority())));

  if (snapshot->has_children()) {
    std::vector<std::string> childKeys;
    for (const auto& child : snapshot->children()) {
      childKeys.push_back(child.key_string());
    }
    map.insert(EncodableValuePair(Constants::kChildKeys,
                                  Conversion::ToEncodableValue(childKeys)));
  }

  return EncodableMap{
      {EncodableValue(Constants::kSnapshot), EncodableValue(map)}};
}

EncodableMap CreateMutableDataSnapshotPayload(MutableData* snapshot) {
  CHECK_NOT_NULL(snapshot);

  TRACE_SCOPE(DATABASE, ToString(*snapshot));

  EncodableMap map;
  map.insert(EncodableValuePair(Constants::kKey, snapshot->key_string()));
  map.insert(EncodableValuePair(
      Constants::kValue, Conversion::ToEncodableValue(snapshot->value())));
  map.insert(
      EncodableValuePair(Constants::kPriority,
                         Conversion::ToEncodableValue(snapshot->priority())));

  if (snapshot->children_count() > 0) {
    std::vector<std::string> childKeys;
    for (const auto& child : snapshot->children()) {
      childKeys.push_back(child.key_string());
    }
    map.insert(EncodableValuePair(Constants::kChildKeys,
                                  Conversion::ToEncodableValue(childKeys)));
  }

  return EncodableMap{
      {EncodableValue(Constants::kSnapshot), EncodableValue(map)}};
}
