#include "cloud_functions_plugin.h"

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <firebase/functions.h>
#include <firebase/functions/callable_reference.h>
#include <firebase/functions/callable_result.h>
#include <firebase/functions/common.h>
#include <firebase/log.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <system_info.h>

#include <functional>
#include <memory>
#include <string>
#include <unordered_set>

#include "common/conversion.h"
#include "common/to_string.h"
#include "common/trace.h"
#include "common/utils.h"
#include "log.h"
#include "queue.hpp"

using flutter::BinaryMessenger;
using flutter::EncodableMap;
using flutter::EncodableValue;
using flutter::MethodChannel;
using flutter::MethodResult;
using EncodableValuePair = std::pair<EncodableValue, EncodableValue>;

using firebase::App;
using firebase::Future;
using firebase::FutureStatus;
using firebase::Variant;
using firebase::functions::Error;
using firebase::functions::Functions;
using firebase::functions::HttpsCallableReference;
using firebase::functions::HttpsCallableResult;

namespace {

constexpr const char* METHOD_CHANNEL_NAME =
    "plugins.flutter.io/firebase_functions";

#include <string>

enum CloudFunctionsErrorCode {
  OK = 0,
  Cancelled = 1,
  Unknown = 2,
  InvalidArgument = 3,
  DeadlineExceeded = 4,
  NotFound = 5,
  AlreadyExists = 6,
  PermissionDenied = 7,
  ResourceExhausted = 8,
  FailedPrecondition = 9,
  Aborted = 10,
  OutOfRange = 11,
  Unimplemented = 12,
  Internal = 13,
  Unavailable = 14,
  DataLoss = 15,
  Unauthenticated = 16
};

static const char* ToErrorCodeString(const int code) {
  switch (code) {
    case CloudFunctionsErrorCode::OK:
      return "ok";
    case CloudFunctionsErrorCode::Cancelled:
      return "cancelled";
    case CloudFunctionsErrorCode::Unknown:
      return "unknown";
    case CloudFunctionsErrorCode::InvalidArgument:
      return "invalid-argument";
    case CloudFunctionsErrorCode::DeadlineExceeded:
      return "deadline-exceeded";
    case CloudFunctionsErrorCode::NotFound:
      return "not-found";
    case CloudFunctionsErrorCode::AlreadyExists:
      return "already-exists";
    case CloudFunctionsErrorCode::PermissionDenied:
      return "permission-denied";
    case CloudFunctionsErrorCode::ResourceExhausted:
      return "resource-exhausted";
    case CloudFunctionsErrorCode::FailedPrecondition:
      return "failed-precondition";
    case CloudFunctionsErrorCode::Aborted:
      return "aborted";
    case CloudFunctionsErrorCode::OutOfRange:
      return "out-of-range";
    case CloudFunctionsErrorCode::Unimplemented:
      return "unimplemented";
    case CloudFunctionsErrorCode::Internal:
      return "internal";
    case CloudFunctionsErrorCode::Unavailable:
      return "unavailable";
    case CloudFunctionsErrorCode::DataLoss:
      return "data-loss";
    case CloudFunctionsErrorCode::Unauthenticated:
      return "unauthenticated";
    default:
      return "unknown";
  }
}

class CloudFunctionsPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar) {
    auto channel = std::make_unique<MethodChannel<EncodableValue>>(
        registrar->messenger(), METHOD_CHANNEL_NAME,
        &flutter::StandardMethodCodec::GetInstance());

    LogOption::setExternalIsEnabled(
        [](const std::string& id) -> bool { return true; });
    Trace::Option::setTag(LOG_TAG);

    firebase::SetLogLevel(firebase::LogLevel::kLogLevelVerbose);

    auto plugin = std::make_unique<CloudFunctionsPlugin>();

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto& call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  CloudFunctionsPlugin() { LOGD("Plugin created"); }

  virtual ~CloudFunctionsPlugin() { LOGD("Plugin destroyed"); }

  struct AsyncTask {
    std::function<void()> handler{nullptr};
  };

 private:
  Queue<AsyncTask> async_task_queue_;

  void HandleMethodCall(const flutter::MethodCall<EncodableValue>& method_call,
                        std::unique_ptr<MethodResult<EncodableValue>> result) {
    TRACE_SCOPE0(PLUGIN);

    // 0. Handle remaining AsyncTasks.

    while (async_task_queue_.empty() == false) {
      AsyncTask task;
      async_task_queue_.pop(task, 0);
      if (task.handler) {
        task.handler();
      }
    }

    // 1. Parse method channel arguments.

    const std::string& method_name = method_call.method_name();
    const auto* arguments = std::get_if<EncodableMap>(method_call.arguments());
    if (!arguments) {
      return result->Error("Invalid arguments", "Invalid argument type.");
    }

    LOGI("[TIZEN: HANDLE_METHOD_CALL] %s {\n%s\n}", method_name.c_str(),
         ToString(*arguments).c_str());

    CHECK(method_name == "FirebaseFunctions#call");

    const auto app_name = GetOptionalValue<std::string>(arguments, "appName")
                              .value_or("[DEFAULT]");
    const auto region =
        GetOptionalValue<std::string>(arguments, "region").value();

    App* app = App::GetInstance(app_name.c_str());
    if (app == nullptr) {
      return result->Error("-1", "No app matched found.");
    }
    Functions* functions = Functions::GetInstance(app, region.c_str());
    if (functions == nullptr) {
      return result->Error("-1", "Can't create functions with the given app.");
    }
    const auto origin = GetOptionalValue<std::string>(arguments, "origin");
    const auto timeout = GetOptionalValue<int64_t>(arguments, "timeout");

    if (origin) {
      functions->UseFunctionsEmulator(origin.value().c_str());
    }

    if (timeout) {
      LOGW("timeout isn't supported.");
    }

    // 2. Call the function requested.

    struct Param {
      Param(std::unique_ptr<MethodResult<EncodableValue>> _result,
            std::function<void()> _post_task)
          : result(std::move(_result)), post_task(std::move(_post_task)) {}
      std::unique_ptr<MethodResult<EncodableValue>> result;
      std::function<void()> post_task;
    };

    TRACE(PLUGIN, "reference.Call");

    std::shared_ptr<HttpsCallableReference> reference(
        new HttpsCallableReference(functions->GetHttpsCallable(
            GetOptionalValue<std::string>(arguments, "functionName")
                .value()
                .c_str())));

    EncodableValue parameters = GetEncodableValue(arguments, "parameters");

    reference->Call(Conversion::ToFirebaseVariant(parameters))
        .OnCompletion(
            [](const Future<HttpsCallableResult>& future, void* data) {
              PointerScope<Param> param(data);
              if (future.status() == FutureStatus::kFutureStatusComplete) {
                TRACE(PLUGIN, "FutureStatus::kFutureStatusComplete");
                if (future.error() == Error::kErrorNone) {
                  TRACE_SCOPE0(PLUGIN, "FutureStatus::Success");
                  param->result->Success(
                      Conversion::ToEncodableValue(future.result()->data()));
                } else {
                  TRACE_SCOPE0(
                      PLUGIN, "FutureStatus::Error", "code:", future.error(),
                      "code_string:", ToErrorCodeString(future.error()),
                      "error_message:", future.error_message());

                  EncodableMap m;
                  m.insert(EncodableValuePair(
                      "code", ToErrorCodeString(future.error())));
                  m.insert(
                      EncodableValuePair("message", future.error_message()));
                  // FIXME: details below should be parsed. (Refs:
                  // https://github.com/firebase/flutterfire/blob/1e4783d86640509a9bae16c923ed5ccb6f917dd3/packages/cloud_functions/cloud_functions_platform_interface/lib/src/method_channel/utils/exception.dart#L37C55-L37C69)
                  auto details = EncodableMap{
                      {EncodableValue("additionalData"), EncodableValue(m)},
                  };
                  TRACE(PLUGIN, "details", details);

                  param->result->Error(std::to_string(future.error()),
                                       future.error_message(),
                                       EncodableValue(details));
                }
              }
              param->post_task();
            },
            new Param(std::move(result), [this, reference]() {
              // This is for holding the reference: shared_ptr<>.
              async_task_queue_.push({.handler = [reference]() {}});
            }));

    TRACE(PLUGIN, "/reference.Call");
  }
};

}  // namespace

void CloudFunctionsPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  CloudFunctionsPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
