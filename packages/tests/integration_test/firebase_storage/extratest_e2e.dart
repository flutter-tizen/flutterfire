// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert';
import 'dart:io';

import 'package:firebase_storage/firebase_storage.dart';
import 'package:flutter_test/flutter_test.dart';

import './test_utils.dart';

const String kOkString = 'ok';
final List<int> okStringList = utf8.encode(kOkString);

void setupExtraTests() {
  group('$Reference', () {
    late FirebaseStorage storage;

    setUpAll(() async {
      storage = FirebaseStorage.instance;
    });

    test('getData', () async {
      final Reference ref = storage.ref('flutter-tests/ok.txt');

      final bytes = await ref.getData();

      List<int> okStringList = utf8.encode(kOkString);
      expect(bytes?.length, okStringList.length);
      expect(bytes, okStringList);
    });

    test('getMetadata', () async {
      final Reference ref = storage.ref('flutter-tests/ok.txt');

      final result = await ref.getMetadata();
      expect(result, isA<FullMetadata>());
      expect(result.name, 'ok.txt');
    });
  });

  group('ControlTask', () {
    late FirebaseStorage storage;

    setUpAll(() async {
      storage = FirebaseStorage.instance;
    });

    Future<void> _testPauseTask(Task task) async {
      List<TaskSnapshot> snapshots = [];
      bool hasError = false;
      expect(task.snapshot.state, TaskState.running);

      task.snapshotEvents.listen(
        (TaskSnapshot snapshot) {
          snapshots.add(snapshot);
        },
        onError: (error) {
          hasError = true;
        },
        cancelOnError: true,
      );

      await task.snapshotEvents.first;
      await Future.delayed(const Duration(milliseconds: 100));

      bool? paused = await task.pause();
      expect(paused, isTrue);
      expect(task.snapshot.state, TaskState.paused);

      await Future.delayed(const Duration(milliseconds: 100));
      bool? resumed = await task.resume();
      expect(resumed, isTrue);
      expect(task.snapshot.state, TaskState.running);

      TaskSnapshot snapshot = await task;
      expect(task.snapshot.state, TaskState.success);
      expect(snapshot.state, TaskState.success);

      expect(snapshot.totalBytes, snapshot.bytesTransferred);

      expect(hasError, false);
    }

    test('upload pause and resume', () async {
      final File file = await createFile('test-file.txt');
      final Reference ref = storage.ref('flutter-tests').child('test-file.txt');

      await _testPauseTask(ref.putFile(file));
    });

    test('download pause and resume', () async {
      final Directory systemTempDir = Directory.systemTemp;
      final File tempFile = File('${systemTempDir.path}/temp-test-file.txt');
      if (tempFile.existsSync()) await tempFile.delete();

      final Reference ref = storage.ref('flutter-tests').child('test-file.txt');

      await _testPauseTask(ref.writeToFile(tempFile));
    });
  });
}
