# firebase_storage_tizen

The [Firebase Storage for Flutter](https://pub.dev/packages/firebase_storage) implementation for Tizen.

It offers experimental features for using Firebase on Flutter for Tizen. It works by wrapping cross-compiled libraries that are based on the [Firebase C++ SDK](https://github.com/firebase/firebase-cpp-sdk) for Linux.


# Usage

To use this package, you need to include `firebase_storage_tizen` as a dependency alongside `firebase_storage` in your `pubspec.yaml`. Please note that `firebase_storage_tizen` implementation is not officially endorsed for `firebase_storage`.

```yaml
dependencies:
  firebase_storage: ^11.0.10
  firebase_storage_tizen: ^0.1.0
```

Then you can import `firebase_storage` in your Dart code:

```dart
import 'package:firebase_storage/firebase_storage.dart';
```

## Required privileges

To use this plugin in a Tizen application, you may need to declare the following privileges in your `tizen-manifest.xml` file.

```xml
<privileges>
  <privilege>http://tizen.org/privilege/internet</privilege>
</privileges>
```

- `http://tizen.org/privilege/internet` allows the application to access the Internet.

For the details on Tizen privileges, please see [Tizen Docs: API Privileges](https://docs.tizen.org/application/dotnet/get-started/api-privileges).

# Limitations

The following features are currently unavailable as they're not supported by the version of Firebase C++ SDK for Linux that this plugin is currently based on.

 - useEmulator method of FirebaseStorage class.
 - listAll method of Reference class.
