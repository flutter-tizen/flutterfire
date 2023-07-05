# FlutterFire end-to-end testing application.

All package e2e tests belong in this application.

# How to run

```shell
$ flutter test integration_test/e2e_test.dart

# Or

$ flutter drive \
  --driver test_driver/integration_test.dart \
  --target integration_test/e2e_test.dart
```

# Prerequisites

1. Configure [the Firebase Options](https://pub.dev/documentation/firebase_core_platform_interface/latest/firebase_core_platform_interface/FirebaseOptions-class.html) below.

```dart
  // tests/lib/firebase_options.dart

  static const FirebaseOptions tizen = FirebaseOptions(
    apiKey: 'PLACEHOLDER',
    appId: 'PLACEHOLDER',
    messagingSenderId: 'PLACEHOLDER',
    projectId: 'PLACEHOLDER',
    databaseURL: 'PLACEHOLDER',
    storageBucket: 'PLACEHOLDER',
  );
```

2. Configure the Firebase emulator.

To test Cloud Functions, use the emulator. Run the command below to go to the path where you can run a test app on the emulator.

```shell
$ cd tests/emulator
```

Configure your connection address and projectId.

```dart
// tests/lib/firebase_options.dart
static String get emulatorHost {
  return 'XX.XXX.XXX.XXX';
}
```

```json
// tests/emulator/firebase.json
{
  "emulators": {
    "functions": {
      "port": 5001,
      "host": "XX.XXX.XXX.XXX"
    }
    ...
  }
}
```

```json
// tests/emulator/.firebaserc
{
  "projects": {
    "default": "PLACEHOLDER"
  }
}
```

3. Run the Firebase emulator for cloud functions.

```shell
# `firebase login` is required at the first time.

$ firebase emulators:start

┌───────────┬─────────────────────┬──────────────────────────────────────┐
│ Emulator  │ Host:Port           │ View in Emulator UI                  │
├───────────┼─────────────────────┼──────────────────────────────────────┤
│ Functions │ XX.XXX.XXX.XXX:5001 │ http://XX.XXX.XXX.XXX:4000/functions │
├───────────┼─────────────────────┼──────────────────────────────────────┤
│ Database  │ XX.XXX.XXX.XXX:9000 │ http://XX.XXX.XXX.XXX:4000/database  │
└───────────┴─────────────────────┴──────────────────────────────────────┘
```
