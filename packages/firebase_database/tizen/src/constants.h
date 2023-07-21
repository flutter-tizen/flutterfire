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

class Constants {
 public:
  static constexpr char kAborted[] = "aborted";
  static constexpr char kAppName[] = "appName";
  static constexpr char kChildKeys[] = "childKeys";
  static constexpr char kChildAdded[] = "childAdded";
  static constexpr char kChildRemove[] = "childRemoved";
  static constexpr char kChildChanged[] = "childChanged";
  static constexpr char kChildMoved[] = "childMoved";
  static constexpr char kCommitted[] = "committed";
  static constexpr char kCursor[] = "cursor";
  static constexpr char kDatabaseCacheSizeBytes[] = "cacheSizeBytes";
  static constexpr char kDatabaseEmulatorHost[] = "emulatorHost";
  static constexpr char kDatabaseEmulatorPort[] = "emulatorPort";
  static constexpr char kDatabaseLoggingEnabled[] = "loggingEnabled";
  static constexpr char kDatabasePersistenceEnabled[] = "persistenceEnabled";
  static constexpr char kDatabaseURL[] = "databaseURL";
  static constexpr char kDefalutAppName[] = "[DEFAULT]";
  static constexpr char kEndAt[] = "endAt";
  static constexpr char kEndBefore[] = "endBefore";
  static constexpr char kEventChannelNamePrefix[] = "eventChannelNamePrefix";
  static constexpr char kEventType[] = "eventType";
  static constexpr char kException[] = "exception";
  static constexpr char kKey[] = "key";
  static constexpr char kLimit[] = "limit";
  static constexpr char kLimitToFirst[] = "limitToFirst";
  static constexpr char kLimitToLast[] = "limitToLast";
  static constexpr char kModifiers[] = "modifiers";
  static constexpr char kName[] = "name";
  static constexpr char kOrderBy[] = "orderBy";
  static constexpr char kOrderByChild[] = "orderByChild";
  static constexpr char kOrderByKey[] = "orderByKey";
  static constexpr char kOrderByPriority[] = "orderByPriority";
  static constexpr char kOrderByValue[] = "orderByValue";
  static constexpr char kPath[] = "path";
  static constexpr char kPreviousChildKey[] = "previousChildKey";
  static constexpr char kPriority[] = "priority";
  static constexpr char kSnapshot[] = "snapshot";
  static constexpr char kStartAfter[] = "startAfter";
  static constexpr char kStartAt[] = "startAt";
  static constexpr char kTransactionApplyLocally[] = "transactionApplyLocally";
  static constexpr char kTransactionKey[] = "transactionKey";
  static constexpr char kType[] = "type";
  static constexpr char kValue[] = "value";
};
