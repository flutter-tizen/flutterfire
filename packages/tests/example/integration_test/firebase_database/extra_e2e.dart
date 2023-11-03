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
import 'package:firebase_database/firebase_database.dart';
import 'package:flutter_test/flutter_test.dart';

void setupExtraTests() {
  group('Database Extra', () {
    test('PurgeOutstandingWrites()', () async {
      FirebaseDatabase.instance.purgeOutstandingWrites();
    });
  });

  group('Query Extra', () {
    late DatabaseReference ref;

    setUp(() async {
      ref = FirebaseDatabase.instance.ref('tests');

      // Wipe the database before each test
      await ref.remove();
    });

    test('keepSynced()', () async {
      ref.keepSynced(true);
    });
  });
}
