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
#ifndef FIREBASE_DATABASE_TIZEN_FIREBASE_DATABASE_UTILS_H_
#define FIREBASE_DATABASE_TIZEN_FIREBASE_DATABASE_UTILS_H_

#include <firebase/database.h>
#include <flutter/encodable_value.h>

// Database

firebase::database::Database* GetDatabaseFromArguments(
    const flutter::EncodableMap* arguments);

firebase::database::DatabaseReference GetDatabaseReferenceFromArguments(
    const flutter::EncodableMap* arguments);

firebase::database::Query GetDatabaseQueryFromArguments(
    const flutter::EncodableMap* arguments);

// Message Channel Payload

flutter::EncodableMap CreateDataSnapshotPayload(
    const firebase::database::DataSnapshot* snapshot);

flutter::EncodableMap CreateMutableDataSnapshotPayload(
    firebase::database::MutableData* snapshot);

#endif  // FIREBASE_DATABASE_TIZEN_FIREBASE_DATABASE_UTILS_H_
