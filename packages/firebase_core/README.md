# firebase_core_tizen

The [Firebase Core for Flutter](https://pub.dev/packages/firebase_core) implementation for Tizen.

It offers experimental features for using Firebase on Flutter for Tizen. It works by wrapping cross-compiled libraries that are based on the [Firebase C++ SDK](https://github.com/firebase/firebase-cpp-sdk) for Linux.

# Usage

To use this package, you need to include `firebase_core_tizen` as a dependency alongside `firebase_core` in your `pubspec.yaml`. Please note that `firebase_core_tizen` implementation is not officially endorsed for `firebase_core`.

```yaml
dependencies:
  firebase_core: ^2.17.0
  firebase_core_tizen: ^1.0.1
```

Then you can import `firebase_core` in your Dart code:

```dart
import 'package:firebase_core/firebase_core.dart';
```

# Limitations

The following features are currently unavailable as they're not supported by the version of Firebase C++ SDK for Linux that this plugin is currently based on.

- Read options from native resources on Tizen.
- Using `SetAutomaticDataCollectionEnabled` on FirebaseApp instance.
- Using `SetAutomaticResourceManagementEnabled` on FirebaseApp instance.

