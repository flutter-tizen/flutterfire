/*
 * Copyright (c) 2023-present Samsung Electronics Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "cloud_functions_plugin.h"

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
#include "queue.hpp"

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

constexpr const char* kMethodChannelName =
    "plugins.flutter.io/firebase_functions";

enum CloudFunctionsErrorCode {
  kOK = 0,
  kCancelled = 1,
  kUnknown = 2,
  kInvalidArgument = 3,
  kDeadlineExceeded = 4,
  kNotFound = 5,
  kAlreadyExists = 6,
  kPermissionDenied = 7,
  kResourceExhausted = 8,
  kFailedPrecondition = 9,
  kAborted = 10,
  kOutOfRange = 11,
  kUnimplemented = 12,
  kInternal = 13,
  kUnavailable = 14,
  kDataLoss = 15,
  kUnauthenticated = 16,
};

static const char* ToErrorCodeString(const int code) {
  switch (code) {
    case CloudFunctionsErrorCode::kOK:
      return "ok";
    case CloudFunctionsErrorCode::kCancelled:
      return "cancelled";
    case CloudFunctionsErrorCode::kUnknown:
      return "unknown";
    case CloudFunctionsErrorCode::kInvalidArgument:
      return "invalid-argument";
    case CloudFunctionsErrorCode::kDeadlineExceeded:
      return "deadline-exceeded";
    case CloudFunctionsErrorCode::kNotFound:
      return "not-found";
    case CloudFunctionsErrorCode::kAlreadyExists:
      return "already-exists";
    case CloudFunctionsErrorCode::kPermissionDenied:
      return "permission-denied";
    case CloudFunctionsErrorCode::kResourceExhausted:
      return "resource-exhausted";
    case CloudFunctionsErrorCode::kFailedPrecondition:
      return "failed-precondition";
    case CloudFunctionsErrorCode::kAborted:
      return "aborted";
    case CloudFunctionsErrorCode::kOutOfRange:
      return "out-of-range";
    case CloudFunctionsErrorCode::kUnimplemented:
      return "unimplemented";
    case CloudFunctionsErrorCode::kInternal:
      return "internal";
    case CloudFunctionsErrorCode::kUnavailable:
      return "unavailable";
    case CloudFunctionsErrorCode::kDataLoss:
      return "data-loss";
    case CloudFunctionsErrorCode::kUnauthenticated:
      return "unauthenticated";
    default:
      return "unknown-code";
  }
}

class CloudFunctionsPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar) {
    auto channel = std::make_unique<MethodChannel<EncodableValue>>(
        registrar->messenger(), kMethodChannelName,
        &flutter::StandardMethodCodec::GetInstance());
    auto plugin = std::make_unique<CloudFunctionsPlugin>();

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto& call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  struct AsyncTask {
    std::function<void()> handler{nullptr};
  };

 private:
  void HandleMethodCall(const flutter::MethodCall<EncodableValue>& method_call,
                        std::unique_ptr<MethodResult<EncodableValue>> result) {
    TRACE_SCOPE0(FUNCTION);

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
      return result->Error("invalid-argument", "Invalid argument type.");
    }

    TRACEF(FUNCTION, "[TIZEN: HANDLE_METHOD_CALL] %s {\n%s\n}", method_name,
           ToString(*arguments));

    CHECK(method_name == "FirebaseFunctions#call");

    const auto app_name = GetOptionalValue<std::string>(arguments, "appName")
                              .value_or("[DEFAULT]");
    const auto region =
        GetOptionalValue<std::string>(arguments, "region").value();

    App* app = App::GetInstance(app_name.c_str());
    if (app == nullptr) {
      return result->Error("unavailable", "No app matched found.");
    }
    Functions* functions = Functions::GetInstance(app, region.c_str());
    if (functions == nullptr) {
      return result->Error("invalid-argument",
                           "Can't create functions with the given app.");
    }
    const auto origin = GetOptionalValue<std::string>(arguments, "origin");
    const auto timeout = GetOptionalValue<int64_t>(arguments, "timeout");

    if (origin) {
      functions->UseFunctionsEmulator(origin.value().c_str());
    }

    if (timeout) {
      TRACE(FUNCTION, "[!] timeout isn't supported.");
    }

    // 2. Call the function requested.

    struct Param {
      Param(std::unique_ptr<MethodResult<EncodableValue>> _result,
            std::function<void()> _post_task)
          : result(std::move(_result)), post_task(std::move(_post_task)) {}
      std::unique_ptr<MethodResult<EncodableValue>> result;
      std::function<void()> post_task;
    };

    TRACE(FUNCTION, "reference.Call");

    auto function_name =
        GetOptionalValue<std::string>(arguments, "functionName");

    CHECK(function_name);

    std::shared_ptr<HttpsCallableReference> reference(
        new HttpsCallableReference(
            functions->GetHttpsCallable(function_name.value().c_str())));

    EncodableValue parameters = GetEncodableValue(arguments, "parameters");

    reference->Call(Conversion::ToFirebaseVariant(parameters))
        .OnCompletion(
            [](const Future<HttpsCallableResult>& future, void* data) {
              PointerScope<Param> param(data);
              if (future.status() == FutureStatus::kFutureStatusComplete) {
                TRACE(FUNCTION, "FutureStatus::kFutureStatusComplete");
                if (future.error() == Error::kErrorNone) {
                  TRACE_SCOPE0(FUNCTION, "FutureStatus::Success");
                  param->result->Success(
                      Conversion::ToEncodableValue(future.result()->data()));
                } else {
                  TRACE_SCOPE0(
                      FUNCTION, "FutureStatus::Error", "code:", future.error(),
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
                  TRACE(FUNCTION, "details", details);

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

    TRACE(FUNCTION, "/reference.Call");
  }

  Queue<AsyncTask> async_task_queue_;
};

}  // namespace

void CloudFunctionsPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  CloudFunctionsPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
