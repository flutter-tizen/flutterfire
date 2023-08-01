# firebase_storage_tizen

A new Flutter plugin project.

## Getting Started

This project is a starting point for a Flutter
[plug-in package](https://flutter.dev/developing-packages/),
a specialized package that includes platform-specific implementation code for
Android and/or iOS.

For help getting started with Flutter development, view the
[online documentation](https://flutter.dev/docs), which offers tutorials,
samples, guidance on mobile development, and a full API reference.

The plugin project was generated without specifying the `--platforms` flag, no platforms are currently supported.
To add platforms, run `flutter create -t plugin --platforms <platforms> .` in this directory.
You can also find a detailed instruction on how to add platforms in the `pubspec.yaml` at https://flutter.dev/docs/development/packages-and-plugins/developing-packages#plugin-platforms.

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
