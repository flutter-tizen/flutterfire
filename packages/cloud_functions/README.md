# cloud_functions_tizen

The [Firebase Cloud Functions for Flutter](https://pub.dev/packages/cloud_functions) implementation for Tizen.

It offers experimental features for using Firebase on Flutter for Tizen. It works by wrapping cross-compiled libraries that are based on the [Firebase C++ SDK](https://github.com/firebase/firebase-cpp-sdk) for Linux.

# Usage

To use this package, you need to include `cloud_functions_tizen` as a dependency alongside `cloud_functions` in your `pubspec.yaml`. Please note that `cloud_functions_tizen` implementation is not officially endorsed for `cloud_functions`.

```yaml
dependencies:
  cloud_functions: 4.0.7
  cloud_functions_tizen: ^0.1.0
```

Then you can import `cloud_functions` in your Dart code:

```dart
import 'package:cloud_functions/cloud_functions.dart';
```

# Limitations

The following features are currently unavailable as they're not supported by the version of Firebase C++ SDK for Linux that this plugin is currently based on.

- Using `HttpsCallableOptions#timeout` for an HttpsCallable instance's options.

# Known bugs

- The code and details of `FirebaseFunctionsException` aren't properly provided.
