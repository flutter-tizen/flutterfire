# FlutterFire for Tizen

FlutterFire for Tizen is a set of plugins that enable Flutter apps to use Firebase services on Tizen devices. Each plugin operates as a platform package for [FlutterFire](https://github.com/firebase/flutterfire), taking the form of a [non-endorsed federated plugin](https://docs.flutter.dev/packages-and-plugins/developing-packages#non-endorsed-federated-plugin). The actual interaction with Firebase services relies on cross-compiled [Firebase C++ SDK](https://github.com/firebase/firebase-cpp-sdk) libraries.

## Pre-Alpha Plugins

| Name                   | pub.dev           | Firebase Product                                                     | Documentation                                                     | View Source             |
|------------------------|:-----------------:|:--------------------------------------------------------------------:|:-----------------------------------------------------------------:|:-----------------------:|
| Cloud Functions        | REPLACEME         | [🔗](https://firebase.google.com/products/functions)                 | [📖](https://firebase.flutter.dev/docs/functions/overview)        | [`cloud_functions`](packages/cloud_functions)                       |
| Core                   | REPLACEME         | [🔗](https://firebase.google.com)                                    | [📖](https://firebase.flutter.dev/docs/core/usage)                | [`firebase_core`](packages/firebase_core)                           |
| Realtime Database      | REPLACEME         | [🔗](https://firebase.google.com/products/database)                  | [📖](https://firebase.flutter.dev/docs/database/overview)         | [`firebase_database`](packages/firebase_database)                   |

Please note that the plugins are now in an early stage of development, as they're based on the experimental implementation of the Firebase C++ SDK for Linux desktop ([v10.4.0](https://github.com/firebase/firebase-cpp-sdk/tree/v10.4.0)). Our plan is to initially provide development versions first, and then gradually transition to a stable version as a subsequent step following stable SDK releases.

## Supported Devices

| Name                   | API level |   TV  |   TV<br>emulator   |
|------------------------|:---------:|:-----:|:------------------:|
| Cloud Functions        | 7.0       | ✔️     | ✔️                  |
| Core                   | 7.0       | ✔️     | ✔️                  |
| Realtime Database      | 7.0       | ✔️     | ✔️                  |

## License

The licence is described separately in each package. [LICENSE](./LICENSE) contains all the licences for the entire repository.
