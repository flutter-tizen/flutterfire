// Copyright 2022, the Chromium project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';

import 'cloud_functions/cloud_functions_e2e_test.dart' as cloud_functions;
import 'firebase_core/firebase_core_e2e_test.dart' as firebase_core;
import 'firebase_database/firebase_database_e2e_test.dart' as firebase_database;
import 'firebase_storage/firebase_storage_e2e_test.dart' as firebase_storage;

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  group('FlutterFire', () {
    firebase_core.main();
    firebase_database.main();
    cloud_functions.main();
    firebase_storage.main();
  });
}
