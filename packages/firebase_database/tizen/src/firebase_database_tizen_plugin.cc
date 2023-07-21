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
#include "firebase_database_tizen_plugin.h"

#include <firebase/database.h>
#include <firebase/database/mutable_data.h>
#include <firebase/future.h>
#include <firebase/log.h>
#include <firebase/variant.h>
#include <flutter/event_channel.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>

#include "common/conversion.h"
#include "common/to_string.h"
#include "common/trace.h"
#include "common/utils.h"
#include "constants.h"
#include "firebase_database_utils.h"

using firebase::Future;
using firebase::FutureStatus;
using firebase::Variant;
using firebase::database::ChildListener;
using firebase::database::DatabaseReference;
using firebase::database::DataSnapshot;
using firebase::database::Error;
using firebase::database::MutableData;
using firebase::database::Query;
using firebase::database::TransactionResult;
using flutter::BinaryMessenger;
using flutter::EncodableMap;
using flutter::EncodableValue;
using flutter::EventChannel;
using flutter::EventSink;
using flutter::MethodChannel;
using flutter::MethodResult;
using flutter::StreamHandler;
using flutter::StreamHandlerError;

using EncodableValuePair = std::pair<EncodableValue, EncodableValue>;

namespace {

static constexpr char kMethodChannelName[] =
    "plugins.flutter.io/firebase_database";

class FirebaseDatabaseTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar) {
    auto plugin = std::make_unique<FirebaseDatabaseTizenPlugin>();

    plugin->channel_ = std::make_unique<MethodChannel<EncodableValue>>(
        registrar->messenger(), kMethodChannelName,
        &flutter::StandardMethodCodec::GetInstance());

    plugin->binary_messenger_ = registrar->messenger();

    plugin->channel_->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto& call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

 private:
  void HandleMethodCall(const flutter::MethodCall<EncodableValue>& method_call,
                        std::unique_ptr<MethodResult<EncodableValue>> result) {
    const std::string& method_name = method_call.method_name();
    const auto* arguments = std::get_if<EncodableMap>(method_call.arguments());
    if (!arguments) {
      return result->Error("Invalid arguments", "Invalid argument type.");
    }

    TRACEF(DATABASE, "[TIZEN: HANDLE_METHOD_CALL] %s {\n%s\n}", method_name,
           ToString(*arguments));

#define DATABASE_METHODS(V)                                                    \
  V("FirebaseDatabase#goOnline", DatabaseGoOnline)                             \
  V("FirebaseDatabase#goOffline", DatabaseGoOffline)                           \
  V("FirebaseDatabase#purgeOutstandingWrites", DatabasePurgeOutstandingWrites) \
  V("DatabaseReference#set", DatabaseReferenceSet)                             \
  V("DatabaseReference#setWithPriority", DatabaseReferenceSetWithPriority)     \
  V("DatabaseReference#update", DatabaseReferenceUpdate)                       \
  V("DatabaseReference#setPriority", DatabaseReferenceSetPriority)             \
  V("DatabaseReference#runTransaction", DatabaseReferenceRunTransaction)       \
  V("OnDisconnect#set", OnDisconnectSet)                                       \
  V("OnDisconnect#setWithPriority", OnDisconnectSetWithPriority)               \
  V("OnDisconnect#update", OnDisconnectUpdate)                                 \
  V("OnDisconnect#cancel", OnDisconnectCancel)                                 \
  V("Query#get", QueryGet)                                                     \
  V("Query#keepSynced", QueryKeepSynced)                                       \
  V("Query#observe", QueryObserve)

#define V(Key, MethodName)                                        \
  {Key, std::bind(&FirebaseDatabaseTizenPlugin::MethodName, this, \
                  std::placeholders::_1, std::placeholders::_2)},

    static std::unordered_map<
        std::string,
        std::function<void(const EncodableMap*,
                           std::unique_ptr<MethodResult<EncodableValue>>)>>
        method_map = {DATABASE_METHODS(V)};
#undef V

    const auto& it = method_map.find(method_name);
    if (it != method_map.end()) {
      it->second(arguments, std::move(result));
    } else {
      result->NotImplemented();
    }
  }

  template <typename T>
  static void CommonOnCompletionCallback(const Future<T>& future, void* data) {
    static_assert(
        std::is_same<T, void>::value || std::is_same<T, DataSnapshot>::value,
        "Unhandled type");

    TRACE(DATABASE, "[DONE]", future.status(), "error", future.error());
    PointerScope<MethodResult<EncodableValue>> result(data);
    if (future.status() == FutureStatus::kFutureStatusComplete) {
      if constexpr (std::is_same_v<T, void>) {
        future.error() == Error::kErrorNone
            ? result->Success()
            : result->Error(std::to_string(future.error()),
                            future.error_message());
      } else if constexpr (std::is_same_v<T, DataSnapshot>) {
        future.error() == Error::kErrorNone
            ? result->Success(
                  EncodableValue(CreateDataSnapshotPayload(future.result())))
            : result->Error(std::to_string(future.error()),
                            future.error_message());
      }
    }
  }

  void DatabaseGoOnline(const EncodableMap* arguments,
                        std::unique_ptr<MethodResult<EncodableValue>> result) {
    TRACE_SCOPE(DATABASE);
    GetDatabaseFromArguments(arguments)->GoOnline();
    result->Success();
  }

  void DatabaseGoOffline(const EncodableMap* arguments,
                         std::unique_ptr<MethodResult<EncodableValue>> result) {
    TRACE_SCOPE(DATABASE);
    GetDatabaseFromArguments(arguments)->GoOffline();
    result->Success();
  }

  void DatabasePurgeOutstandingWrites(
      const EncodableMap* arguments,
      std::unique_ptr<MethodResult<EncodableValue>> result) {
    TRACE_SCOPE(DATABASE);
    GetDatabaseFromArguments(arguments)->PurgeOutstandingWrites();
    result->Success();
  }

  void DatabaseReferenceSet(
      const EncodableMap* arguments,
      std::unique_ptr<MethodResult<EncodableValue>> result) {
    TRACE_SCOPE(DATABASE);
    GetDatabaseReferenceFromArguments(arguments)
        .SetValue(Conversion::ToFirebaseVariant(arguments, Constants::kValue))
        .OnCompletion(CommonOnCompletionCallback, result.release());
  }

  void DatabaseReferenceSetWithPriority(
      const EncodableMap* arguments,
      std::unique_ptr<MethodResult<EncodableValue>> result) {
    TRACE_SCOPE(DATABASE);
    GetDatabaseReferenceFromArguments(arguments)
        .SetValueAndPriority(
            Conversion::ToFirebaseVariant(arguments, Constants::kValue),
            Conversion::ToFirebaseVariant(arguments, Constants::kPriority))
        .OnCompletion(CommonOnCompletionCallback, result.release());
  }

  void DatabaseReferenceUpdate(
      const EncodableMap* arguments,
      std::unique_ptr<MethodResult<EncodableValue>> result) {
    TRACE_SCOPE(DATABASE);
    GetDatabaseReferenceFromArguments(arguments)
        .UpdateChildren(
            Conversion::ToFirebaseVariant(arguments, Constants::kValue))
        .OnCompletion(CommonOnCompletionCallback, result.release());
  }

  void DatabaseReferenceSetPriority(
      const EncodableMap* arguments,
      std::unique_ptr<MethodResult<EncodableValue>> result) {
    TRACE_SCOPE(DATABASE);
    GetDatabaseReferenceFromArguments(arguments)
        .SetPriority(
            Conversion::ToFirebaseVariant(arguments, Constants::kPriority))
        .OnCompletion(CommonOnCompletionCallback, result.release());
  }

  void DatabaseReferenceRunTransaction(
      const EncodableMap* arguments,
      std::unique_ptr<MethodResult<EncodableValue>> result) {
    // Local class TransactionMethodResult
    //
    class TransactionMethodResult : public MethodResult<> {
     public:
      void SuccessInternal(const EncodableValue* should_load) override {
        TRACE(DATABASE, "[TRANSACTION/DART]", ToString(*should_load));

        std::lock_guard<std::mutex> lock(mutex_);

        if (auto* map = std::get_if<EncodableMap>(should_load)) {
          wrapped_data_->aborted =
              GetOptionalValue<bool>(map, Constants::kAborted)
                  .value_or(wrapped_data_->aborted);

          wrapped_data_->exception =
              GetOptionalValue<bool>(map, Constants::kException)
                  .value_or(wrapped_data_->exception);

          CHECK_NOT_NULL(wrapped_data_->current_data);
          wrapped_data_->current_data->set_value(
              Conversion::ToFirebaseVariant(map, Constants::kValue));
        } else {
          FATAL("The given result should be structed as EncodableMap");
        }
        cv_.notify_one();
      }

      void ErrorInternal(const std::string& error_code,
                         const std::string& error_message,
                         const EncodableValue* error_details) override {
        TRACE(DATABASE, error_code, error_message);
        wrapped_data_->aborted = true;
        cv_.notify_one();
      }

      void NotImplementedInternal() override {
        TRACE(DATABASE);
        wrapped_data_->aborted = true;
        cv_.notify_one();
      }

      struct WrappedData {
        WrappedData(MutableData* data) : current_data(data) {
          CHECK_NOT_NULL(current_data);
        }
        MutableData* current_data;
        bool aborted{false};
        bool exception{false};
      };

      TransactionMethodResult(std::mutex& mutex, std::condition_variable& cv,
                              std::shared_ptr<WrappedData> wrapped_data)
          : mutex_(mutex), cv_(cv), wrapped_data_(wrapped_data) {
        CHECK_NOT_NULL(wrapped_data_);
      }

     private:
      std::mutex& mutex_;
      std::condition_variable& cv_;
      std::shared_ptr<WrappedData> wrapped_data_;
    };

    TRACE_SCOPE(DATABASE);
    const auto transaction_key =
        Conversion::ToFirebaseVariant(arguments, Constants::kTransactionKey)
            .int64_value();
    const auto is_transaction_apply_locally =
        Conversion::ToFirebaseVariant(arguments,
                                      Constants::kTransactionApplyLocally)
            .bool_value();

    TRACE(DATABASE, "transactionKey", transaction_key);
    TRACE(DATABASE, "transactionApplyLocally", is_transaction_apply_locally);

    GetDatabaseReferenceFromArguments(arguments)
        .RunTransaction(
            [this, transaction_key](MutableData* data) {
              TRACE(DATABASE, "[TRANSACTION/FB]");

              std::mutex mutex;
              std::condition_variable cv;

              auto wrapped =
                  std::make_shared<TransactionMethodResult::WrappedData>(data);
              auto result =
                  std::make_unique<TransactionMethodResult>(mutex, cv, wrapped);

              EncodableMap arguments = CreateMutableDataSnapshotPayload(data);
              arguments.insert(EncodableValuePair(Constants::kTransactionKey,
                                                  transaction_key));

              TRACE(DATABASE, ToString(arguments));

              std::unique_lock<std::mutex> lock(mutex);
              channel_->InvokeMethod(
                  "FirebaseDatabase#callTransactionHandler",
                  std::make_unique<EncodableValue>(arguments),
                  std::move(result));
              cv.wait(lock);

              if (wrapped->aborted || wrapped->exception) {
                return TransactionResult::kTransactionResultAbort;
              } else {
                return TransactionResult::kTransactionResultSuccess;
              }
            },
            is_transaction_apply_locally)
        .OnCompletion(
            [](const Future<DataSnapshot>& future, void* data) {
              TRACE_SCOPE0(DATABASE, "[DONE]", future.status(), "error",
                           future.error());
              PointerScope<MethodResult<EncodableValue>> result(data);
              if (future.status() == FutureStatus::kFutureStatusComplete) {
                const auto error = future.error();
                if (error == Error::kErrorNone) {
                  EncodableMap payload =
                      CreateDataSnapshotPayload(future.result());
                  payload.insert(
                      EncodableValuePair(Constants::kCommitted,
                                         Conversion::ToEncodableValue(true)));
                  result->Success(EncodableValue(payload));
                } else {
                  // TODO(daeyeon): verify if the following is a firebase issue.
                  //
                  // In the src/include/firebase/database/database_reference.h,
                  // it says that the code, 'kErrorTransactionAbortedByUser',
                  // means that a transaction was aborted because its
                  // transaction function returned 'kTransactionResultAbort',
                  // and the old value of the DataSnapshot will be returned.
                  //
                  // However, in src/desktop/core/repo.cc:672, the Complete()
                  // function is called with 'kErrorWriteCanceled' instead of
                  // 'kErrorTransactionAbortedByUser'. This is our question
                  // mark. Here we need to check for 'kErrorWriteCanceled' also.
                  //
                  // Seeing the code passed in other platform implementations,
                  // we can see that 'kErrorTransactionAbortedByUser' is passed
                  // as mentioned in the documentation.
                  // - src/android/util_android.cc:182
                  // - src/ios/database_reference_ios.mm:165

                  if (error == Error::kErrorTransactionAbortedByUser ||
                      error == Error::kErrorWriteCanceled) {
                    EncodableMap payload =
                        CreateDataSnapshotPayload(future.result());
                    payload.insert(EncodableValuePair(
                        Constants::kCommitted,
                        Conversion::ToEncodableValue(false)));
                    result->Success(EncodableValue(payload));
                  } else {
                    result->Error(std::to_string(error),
                                  future.error_message());
                  }
                }
              }
            },
            result.release());
  }

  void OnDisconnectSet(const EncodableMap* arguments,
                       std::unique_ptr<MethodResult<EncodableValue>> result) {
    TRACE_SCOPE(DATABASE);
    GetDatabaseReferenceFromArguments(arguments)
        .OnDisconnect()
        ->SetValue(Conversion::ToFirebaseVariant(
            GetEncodableValue(arguments, Constants::kValue)))
        .OnCompletion(CommonOnCompletionCallback, result.release());
  }

  void OnDisconnectSetWithPriority(
      const EncodableMap* arguments,
      std::unique_ptr<MethodResult<EncodableValue>> result) {
    TRACE_SCOPE(DATABASE);
    GetDatabaseReferenceFromArguments(arguments)
        .OnDisconnect()
        ->SetValueAndPriority(
            Conversion::ToFirebaseVariant(arguments, Constants::kValue),
            Conversion::ToFirebaseVariant(arguments, Constants::kPriority))
        .OnCompletion(CommonOnCompletionCallback, result.release());
  }

  void OnDisconnectUpdate(
      const EncodableMap* arguments,
      std::unique_ptr<MethodResult<EncodableValue>> result) {
    TRACE_SCOPE(DATABASE);
    GetDatabaseReferenceFromArguments(arguments)
        .OnDisconnect()
        ->UpdateChildren(
            Conversion::ToFirebaseVariant(arguments, Constants::kValue))
        .OnCompletion(CommonOnCompletionCallback, result.release());
  }

  void OnDisconnectCancel(
      const EncodableMap* arguments,
      std::unique_ptr<MethodResult<EncodableValue>> result) {
    TRACE_SCOPE(DATABASE);
    GetDatabaseReferenceFromArguments(arguments)
        .OnDisconnect()
        ->Cancel()
        .OnCompletion(CommonOnCompletionCallback, result.release());
  }

  void QueryGet(const EncodableMap* arguments,
                std::unique_ptr<MethodResult<EncodableValue>> result) {
    TRACE_SCOPE(DATABASE);
    GetDatabaseQueryFromArguments(arguments).GetValue().OnCompletion(
        CommonOnCompletionCallback, result.release());
  }

  void QueryKeepSynced(const EncodableMap* arguments,
                       std::unique_ptr<MethodResult<EncodableValue>> result) {
    TRACE_SCOPE(DATABASE);
    auto keep_sync =
        GetOptionalValue<bool>(arguments, Constants::kValue).value();
    GetDatabaseQueryFromArguments(arguments).SetKeepSynchronized(keep_sync);
    result->Success();
  }

  void QueryObserve(const EncodableMap* arguments,
                    std::unique_ptr<MethodResult<EncodableValue>> result) {
    // Local class FirebaseChildListener
    //
    class FirebaseChildListener : public ChildListener {
     public:
      using ObserveHandler = std::function<void(
          const std::string& event_type, const DataSnapshot& snapshot,
          const char* previous_sibling_key)>;

      using CancelHandler =
          std::function<void(const Error& error, const char* error_message)>;

      FirebaseChildListener(
          std::string event_channel_name,
          std::shared_ptr<EventChannel<EncodableValue>> channel) {
        event_channel_name_ = event_channel_name;
        channel_ = channel;
      }

      ~FirebaseChildListener() { SetHandler(nullptr, nullptr); }

      void OnChildAdded(const DataSnapshot& snapshot,
                        const char* previous_sibling_key) override {
        TRACE_SCOPE(FB_LISTEN);
        NotifyObserveEvent(Constants::kChildAdded, snapshot,
                           previous_sibling_key);
      }

      void OnChildChanged(const DataSnapshot& snapshot,
                          const char* previous_sibling_key) override {
        TRACE_SCOPE(FB_LISTEN);
        NotifyObserveEvent(Constants::kChildChanged, snapshot,
                           previous_sibling_key);
      }

      void OnChildMoved(const DataSnapshot& snapshot,
                        const char* previous_sibling_key) override {
        TRACE_SCOPE(FB_LISTEN);
        NotifyObserveEvent(Constants::kChildMoved, snapshot,
                           previous_sibling_key);
      }

      void OnChildRemoved(const DataSnapshot& snapshot) override {
        TRACE_SCOPE(FB_LISTEN);
        NotifyObserveEvent(Constants::kChildRemove, snapshot, nullptr);
      }

      void OnCancelled(const Error& error, const char* error_message) override {
        TRACE_SCOPE(FB_LISTEN);
        NotifyCancelEvent(error, error_message);
      }

      void NotifyObserveEvent(const std::string& event_type,
                              const DataSnapshot& snapshot,
                              const char* previous_sibling_key) {
        TRACE_SCOPE(FB_LISTEN);
        if (observe_handler_) {
          std::lock_guard<std::mutex> lock(mutex_);
          observe_handler_(event_type, snapshot, previous_sibling_key);
        }
      }

      void NotifyCancelEvent(const Error& error, const char* error_message) {
        TRACE_SCOPE(FB_LISTEN);
        if (cancel_handler_) {
          std::lock_guard<std::mutex> lock(mutex_);
          cancel_handler_(error, error_message);
        }
      }

      void SetHandler(ObserveHandler observe_handler,
                      CancelHandler cancel_handler) {
        TRACE_SCOPE(FB_LISTEN);
        std::lock_guard<std::mutex> lock(mutex_);
        observe_handler_ = observe_handler;
        cancel_handler_ = cancel_handler;
      }

     private:
      std::mutex mutex_;
      std::string event_channel_name_;
      std::shared_ptr<EventChannel<EncodableValue>> channel_;
      ObserveHandler observe_handler_{nullptr};
      CancelHandler cancel_handler_{nullptr};
    };

    // Local class FlutterStreamHandler
    //
    // The class methods are called whenever the stream is listened to or
    // canceled in the Flutter.
    class FlutterStreamHandler : public StreamHandler<EncodableValue> {
     public:
      FlutterStreamHandler(
          std::shared_ptr<Query> query,
          std::shared_ptr<EventChannel<EncodableValue>> channel,
          FirebaseChildListener* childListener)
          : query_(query), channel_(channel), childListener_(childListener) {}

      ~FlutterStreamHandler() {
        if (childListener_) {
          TRACE(FT_STREAM,
                "[!] delete childListener_, but it's supposed to be deleted "
                "inside OnCancelInternal()");
          query_->RemoveChildListener(childListener_);
          delete childListener_;
        }
      }

     protected:
      std::unique_ptr<StreamHandlerError<EncodableValue>> OnListenInternal(
          const EncodableValue* arguments,
          std::unique_ptr<EventSink<EncodableValue>>&& events) override {
        TRACE_SCOPE(FT_STREAM, ToString(*arguments));

        // NOTE(daeyeon): Any event_channel_name bound to this handler is
        // unique, and it is expected that this function will be called only
        // once for the event_channel_name. Therefore, we assume that the sink
        // should always be nullptr at this point.
        CHECK_NULL(events_);
        CHECK_NOT_NULL(childListener_);

        events_ = std::move(events);

        // The arguments should be an EncodableMap including "eventType":
        // https://github.com/firebase/flutterfire/blob/9be203f10dd20c6bc9bd0588ea8062e326de767c/packages/firebase_database/firebase_database_platform_interface/lib/src/method_channel/method_channel_query.dart#L58
        CHECK(std::holds_alternative<EncodableMap>(*arguments));

        auto map = std::get<EncodableMap>(*arguments);
        event_type_ =
            GetOptionalValue<std::string>(&map, Constants::kEventType).value();

        // Register a handler to send an event
        childListener_->SetHandler(
            [this](const std::string& event_type, const DataSnapshot& snapshot,
                   const char* previous_sibling_key) {
              TRACE_SCOPE(FT_STREAM, "listen:", event_type_,
                          "receive:", event_type, "key:",
                          previous_sibling_key == nullptr
                              ? "{}"
                              : previous_sibling_key);

              if (event_type_ == event_type) {
                EncodableMap payload = CreateDataSnapshotPayload(&snapshot);
                payload.insert(
                    EncodableValuePair(Constants::kEventType, event_type));
                if (previous_sibling_key) {
                  payload.insert(EncodableValuePair(
                      Constants::kPreviousChildKey, previous_sibling_key));
                }
                events_->Success(EncodableValue(payload));
              }
            },
            [this](const Error& error, const char* error_message) {
              TRACE_SCOPE(FT_STREAM, "error_message", error_message);
              events_->Error(std::to_string(error), error_message);
            });
        return nullptr;
      }

      std::unique_ptr<StreamHandlerError<EncodableValue>> OnCancelInternal(
          const flutter::EncodableValue* arguments) override {
        TRACE_SCOPE(FT_STREAM, ToString(*arguments));

        // Here, release all the associated resources.

        // Release events
        events_.reset();

        // Release the listener referring to query_.
        query_->RemoveChildListener(childListener_);
        delete childListener_;
        childListener_ = nullptr;

        // Calling this will release this instance itself.
        channel_->SetStreamHandler(nullptr);

        return nullptr;
      }

     private:
      std::shared_ptr<Query> query_;
      std::shared_ptr<EventChannel<EncodableValue>> channel_;
      std::string event_type_;
      std::unique_ptr<EventSink<EncodableValue>> events_;
      FirebaseChildListener* childListener_;
    };

    TRACE_SCOPE(DATABASE);
    const auto prefix = GetOptionalValue<std::string>(
                            arguments, Constants::kEventChannelNamePrefix)
                            .value();
    const std::string event_channel_name =
        prefix + "#" + std::to_string(++listener_count_);

    TRACE(DATABASE, "event_channel_name:", event_channel_name);

    Query query = GetDatabaseQueryFromArguments(arguments);

    // Create an event channel
    auto channel = std::make_shared<EventChannel<EncodableValue>>(
        binary_messenger_, event_channel_name,
        &flutter::StandardMethodCodec::GetInstance());

    // Create a listener to this query. This is supposed to be deleted in
    // FirebaseChildListener::OnCancelInternal.
    auto listener = new FirebaseChildListener(event_channel_name, channel);
    query.AddChildListener(listener);

    // Create a stream handler
    auto stream_handler = std::make_unique<FlutterStreamHandler>(
        std::make_shared<Query>(query), channel, listener);

    // Register a stream handler on this channel
    channel->SetStreamHandler(std::move(stream_handler));

    result->Success(EncodableValue(event_channel_name));
  }

 private:
  std::unique_ptr<MethodChannel<EncodableValue>> channel_;
  int listener_count_{0};
  BinaryMessenger* binary_messenger_{nullptr};
};

}  // namespace

void FirebaseDatabaseTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  FirebaseDatabaseTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
