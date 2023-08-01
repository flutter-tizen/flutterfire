# Example

A test app for cloud function plugin (e2e).

## Prerequisites

1. Configure [the Firebase Options](https://pub.dev/documentation/firebase_core_platform_interface/latest/firebase_core_platform_interface/FirebaseOptions-class.html) below.

```dart
  // {plugin_example_path}/lib/firebase_options.dart

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
$ cd tools/emulator/functions
```

Configure your connection address and projectId.

```dart
// example/lib/firebase_options.dart
static String get emulatorHost {
  return 'XX.XXX.XXX.XXX';
}
```

```json
// tools/emulator/firebase.json
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
// tools/emulator/.firebaserc
{
  "projects": {
    "default": "PLACEHOLDER"
  }
}
```

3. Run the Firebase emulator for cloud functions.

```shell
$ cd tools/emulator/functions

# `npm install` is required at the first time.

$ npm run serve

┌───────────┬─────────────────────┬──────────────────────────────────────┐
│ Emulator  │ Host:Port           │ View in Emulator UI                  │
├───────────┼─────────────────────┼──────────────────────────────────────┤
│ Functions │ XX.XXX.XXX.XXX:5001 │ http://XX.XXX.XXX.XXX:4000/functions │
└───────────┴─────────────────────┴──────────────────────────────────────┘
```
