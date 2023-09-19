#include "firebase_storage_tizen_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <functional>
#include <memory>
#include <string>

#include "firebase_storage_error.h"
#include "firebase_storage_task.h"
#include "flutter_types.hpp"
#include "log.h"

namespace {

class StorageReferenceWork {
 public:
  StorageReferenceWork(std::unique_ptr<MethodCallArguments>&& args,
                       std::unique_ptr<FlMethodResult>&& result)
      : reference_(utils::GetStorageReference(args.get())),
        method_call_args_(std::move(args)),
        method_result_(std::move(result)) {}

  firebase::storage::StorageReference* GetStorageReference() {
    return &reference_;
  }

  MethodCallArguments* GetMethodCallArguments() {
    return method_call_args_.get();
  }

  void Success(const flutter::EncodableValue& result) {
    method_result_->Success(result);
  }

  void Success() { method_result_->Success(); }

  void Fail(int error_code) {
    FirebaseStorageError error(error_code);
    auto code = error.GetCodeString();
    auto message = error.GetMessage();

    method_result_->Error(
        code, message,
        flutter::EncodableValue(flutter::EncodableMap{
            {flutter::EncodableValue("code"), flutter::EncodableValue(code)},
            {flutter::EncodableValue("message"),
             flutter::EncodableValue(message)}}));
  }

 private:
  firebase::storage::StorageReference reference_;
  std::unique_ptr<MethodCallArguments> method_call_args_;
  std::unique_ptr<FlMethodResult> method_result_;
};

class FirebaseStorageTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar) {
    auto channel = std::make_shared<FlMethodChannel>(
        registrar->messenger(), "plugins.flutter.io/firebase_storage",
        &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<FirebaseStorageTizenPlugin>(channel);

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto& call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  FirebaseStorageTizenPlugin(std::shared_ptr<FlMethodChannel> channel)
      : channel_(std::move(channel)) {}

  virtual ~FirebaseStorageTizenPlugin() {}

 private:
  std::shared_ptr<FlMethodChannel> channel_;

  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<FlMethodResult> result) {
    auto method_call_arguments =
        std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!method_call_arguments) {
      FirebaseStorageError error(FirebaseStorageError::Code::kInvalidArgument);
      result->Error(error.GetCodeString(), "No arguments provided.");
      return;
    }

    auto args = std::make_unique<MethodCallArguments>(method_call_arguments);
    const auto& method_name = method_call.method_name();

    try {
      if (method_name == "Storage#useEmulator") {
        result->NotImplemented();
      } else if (method_name == "Reference#delete") {
        ReferenceDelete(std::make_shared<StorageReferenceWork>(
            std::move(args), std::move(result)));
      } else if (method_name == "Reference#getDownloadURL") {
        ReferenceGetDownloadURL(std::make_shared<StorageReferenceWork>(
            std::move(args), std::move(result)));
      } else if (method_name == "Reference#getMetadata") {
        ReferenceGetMetadata(std::make_shared<StorageReferenceWork>(
            std::move(args), std::move(result)));
      } else if (method_name == "Reference#getData") {
        ReferenceGetData(std::make_shared<StorageReferenceWork>(
            std::move(args), std::move(result)));
      } else if (method_name == "Reference#list") {
        ReferenceList(std::make_shared<StorageReferenceWork>(
            std::move(args), std::move(result)));
      } else if (method_name == "Reference#listAll") {
        result->NotImplemented();
      } else if (method_name == "Reference#updateMetadata") {
        ReferenceUpdateMetadata(std::make_shared<StorageReferenceWork>(
            std::move(args), std::move(result)));
      } else if (method_name == "Task#startPutData") {
        StorageTask::Create<StoragePutDataTask>(channel_, std::move(args))
            ->Run();
        result->Success();
      } else if (method_name == "Task#startPutString") {
        StorageTask::Create<StoragePutStringTask>(channel_, std::move(args))
            ->Run();
        result->Success();
      } else if (method_name == "Task#startPutFile") {
        StorageTask::Create<StoragePutFileTask>(channel_, std::move(args))
            ->Run();
        result->Success();
      } else if (method_name == "Task#pause") {
        TaskStorageControl(
            std::move(args), std::move(result),
            [](StorageTask* task) { return task->GetController()->Pause(); });
      } else if (method_name == "Task#resume") {
        TaskStorageControl(
            std::move(args), std::move(result),
            [](StorageTask* task) { return task->GetController()->Resume(); });
      } else if (method_name == "Task#cancel") {
        TaskStorageControl(
            std::move(args), std::move(result), [](StorageTask* task) {
              auto result = task->GetController()->Cancel();

              if (result) {
                task->GetMethodChannel()->InvokeMethod(
                    "Task#onCanceled",
                    std::make_unique<flutter::EncodableValue>(
                        utils::GetTaskEventValue(task->GetStorageTaskData())));
              }

              return result;
            });
      } else if (method_name == "Task#writeToFile") {
        StorageTask::Create<StorageWriteToFileTask>(channel_, std::move(args))
            ->Run();
        result->Success();
      } else {
        result->NotImplemented();
      }
    } catch (const std::invalid_argument& error) {
      result->Error("invalid-argument", error.what());
    } catch (const FirebaseStorageError& error) {
      result->Error(error.GetCodeString(), error.GetMessage());
    }
  }

  void ReferenceDelete(const std::shared_ptr<StorageReferenceWork>& work) {
    work->GetStorageReference()->Delete().OnCompletion(
        [work](const firebase::Future<void>& metadata) {
          if (metadata.error() == firebase::storage::Error::kErrorNone) {
            work->Success();
          } else {
            work->Fail(metadata.error());
          }
        });
  }

  void ReferenceGetDownloadURL(
      const std::shared_ptr<StorageReferenceWork>& work) {
    work->GetStorageReference()->GetDownloadUrl().OnCompletion(
        [work](const firebase::Future<std::string>& metadata) {
          if (metadata.error() == firebase::storage::Error::kErrorNone) {
            work->Success(flutter::EncodableValue(flutter::EncodableMap{
                {flutter::EncodableValue("downloadURL"),
                 flutter::EncodableValue(*metadata.result())}}));
          } else {
            work->Fail(metadata.error());
          }
        });
  }

  void ReferenceGetMetadata(const std::shared_ptr<StorageReferenceWork>& work) {
    work->GetStorageReference()->GetMetadata().OnCompletion(
        [work](const firebase::Future<firebase::storage::Metadata>& metadata) {
          if (metadata.error() == firebase::storage::Error::kErrorNone) {
            work->Success(utils::GetMetadataValue(metadata.result()));
          } else {
            work->Fail(metadata.error());
          }
        });
  }

  void ReferenceGetData(const std::shared_ptr<StorageReferenceWork>& work) {
    auto max_size =
        work->GetMethodCallArguments()->GetRequiredArg<int>("maxSize");
    auto buffer = std::make_shared<std::vector<uint8_t>>(max_size);

    work->GetStorageReference()
        ->GetBytes(buffer.get()->data(), max_size)
        .OnCompletion(
            [work, buffer, max_size](const firebase::Future<size_t>& metadata) {
              auto buffer_size = *metadata.result();
              assert(buffer_size <= max_size);

              if (metadata.error() == firebase::storage::Error::kErrorNone) {
                auto buffer_data = buffer.get();
                buffer_data->resize(buffer_size);

                work->Success(flutter::EncodableValue(*buffer_data));
              } else {
                work->Fail(metadata.error());
              }
            });
  }

  void ReferenceList(const std::shared_ptr<StorageReferenceWork>& work) {
    auto options_map =
        work->GetMethodCallArguments()->GetRequiredArg<flutter::EncodableMap>(
            "options");

    MethodCallArguments options(&options_map);

    int max_result = options.GetRequiredArg<int>("maxResults");
    auto page_token = options.GetArg<std::string>("pageToken").value_or("");
    work->GetStorageReference()
        ->List(max_result, page_token)
        .OnCompletion([work](const firebase::Future<ListResult>& metadata) {
          if (metadata.error() == firebase::storage::Error::kErrorNone) {
            work->Success(utils::ParseListResult(*metadata.result()));
          } else {
            work->Fail(metadata.error());
          }
        });
  }

  void ReferenceUpdateMetadata(
      const std::shared_ptr<StorageReferenceWork>& work) {
    auto metadata =
        work->GetMethodCallArguments()->GetRequiredArg<flutter::EncodableMap>(
            "metadata");

    work->GetStorageReference()
        ->UpdateMetadata(utils::ParseMetadata(metadata))
        .OnCompletion(
            [work](
                const firebase::Future<firebase::storage::Metadata>& metadata) {
              if (metadata.error() == firebase::storage::Error::kErrorNone) {
                work->Success(utils::GetMetadataValue(metadata.result()));
              } else {
                work->Fail(metadata.error());
              }
            });
  }

  void TaskStorageControl(std::unique_ptr<MethodCallArguments>&& args,
                          std::unique_ptr<FlMethodResult>&& result,
                          std::function<bool(StorageTask*)> control_callback) {
    int handle = args->GetRequiredArg<int>("handle");

    auto task_value = StorageTaskHandler::GetInstance().GetTask(handle);
    if (!task_value.has_value()) {
      FirebaseStorageError error(FirebaseStorageError::Code::KTaskNotFound);
      result->Error(error.GetCodeString(), error.GetMessage());
      return;
    }

    auto task = *task_value;
    bool status = control_callback(task);
    result->Success(utils::GetTaskControlEventValue(status, task->GetPath(),
                                                    task->GetController()));
  }
};

}  // namespace

void FirebaseStorageTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  FirebaseStorageTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
