# firebase_database_tizen

The [Firebase Database for Flutter](https://pub.dev/packages/firebase_database) implementation for Tizen.

It offers experimental features for using Firebase on Flutter for Tizen. It works by wrapping cross-compiled libraries that are based on the [Firebase C++ SDK](https://github.com/firebase/firebase-cpp-sdk) for Linux.

# Usage

To use this package, you need to include `firebase_database_tizen` as a dependency alongside `firebase_database` in your `pubspec.yaml`. Please note that `firebase_database_tizen` implementation is not officially endorsed for `firebase_database`.

```yaml
dependencies:
  firebase_database: 10.0.9
  firebase_database_tizen: ^0.1.0
```

Then you can import `firebase_database` in your Dart code:

```dart
import 'package:firebase_database/firebase_database.dart';
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

- Using Firebase Local Emulator Suite.
- Using `FirebaseDatabase#setPersistenceCacheSizeBytes()` for the on-disk data.
- Using `Query#startAfter()` or `Query#endBefore()` method to add a cursor to a query.
