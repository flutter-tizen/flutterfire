// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "firebase_core_tizen_plugin.h"

#include <firebase/app.h>
#include <firebase/log.h>
#include <flutter/plugin_registrar.h>

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "log.h"
#include "messages.g.h"

using namespace firebase_core_tizen;
using ::firebase::App;

namespace {

class FirebaseCoreTizenPlugin : public flutter::Plugin,
                                public FirebaseCoreHostApi,
                                public FirebaseAppHostApi {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar) {
    auto plugin = std::make_unique<FirebaseCoreTizenPlugin>(registrar);
    registrar->AddPlugin(std::move(plugin));
  }

  FirebaseCoreTizenPlugin(flutter::PluginRegistrar* registrar);
  virtual ~FirebaseCoreTizenPlugin();

  // FirebaseCoreHostApi class override
  virtual void InitializeApp(
      const std::string& app_name,
      const PigeonFirebaseOptions& initialize_app_request,
      std::function<void(ErrorOr<PigeonInitializeResponse> reply)> result)
      override;
  virtual void InitializeCore(
      std::function<void(ErrorOr<flutter::EncodableList> reply)> result)
      override;
  virtual void OptionsFromResource(
      std::function<void(ErrorOr<PigeonFirebaseOptions> reply)> result)
      override;

  // FirebaseAppHostApi class override
  virtual void SetAutomaticDataCollectionEnabled(
      const std::string& app_name, bool enabled,
      std::function<void(std::optional<FlutterError> reply)> result) override;
  virtual void SetAutomaticResourceManagementEnabled(
      const std::string& app_name, bool enabled,
      std::function<void(std::optional<FlutterError> reply)> result) override;
  virtual void Delete(
      const std::string& app_name,
      std::function<void(std::optional<FlutterError> reply)> result) override;

 private:
  flutter::PluginRegistrar* plugin_registrar_;
};

FirebaseCoreTizenPlugin::FirebaseCoreTizenPlugin(
    flutter::PluginRegistrar* registrar)
    : plugin_registrar_(registrar) {
  FirebaseCoreHostApi::SetUp(registrar->messenger(), this);
  FirebaseAppHostApi::SetUp(registrar->messenger(), this);

  firebase::SetLogLevel(firebase::LogLevel::kLogLevelWarning);
}

FirebaseCoreTizenPlugin::~FirebaseCoreTizenPlugin() {}

// Convert a Pigeon FirebaseOptions to a Firebase Options.
firebase::AppOptions PigeonFirebaseOptionsToAppOptions(
    const PigeonFirebaseOptions& pigeon_options) {
  firebase::AppOptions options;
  options.set_api_key(pigeon_options.api_key().c_str());
  options.set_app_id(pigeon_options.app_id().c_str());
  if (pigeon_options.database_u_r_l() != nullptr) {
    options.set_database_url(pigeon_options.database_u_r_l()->c_str());
  }
  if (pigeon_options.tracking_id() != nullptr) {
    options.set_ga_tracking_id(pigeon_options.tracking_id()->c_str());
  }
  options.set_messaging_sender_id(pigeon_options.messaging_sender_id().c_str());

  options.set_project_id(pigeon_options.project_id().c_str());

  if (pigeon_options.storage_bucket() != nullptr) {
    options.set_storage_bucket(pigeon_options.storage_bucket()->c_str());
  }
  return options;
}

// Convert a AppOptions to PigeonInitializeOption
PigeonFirebaseOptions optionsFromFIROptions(
    const firebase::AppOptions& options) {
  PigeonFirebaseOptions pigeon_options = PigeonFirebaseOptions();
  pigeon_options.set_api_key(options.api_key());
  pigeon_options.set_app_id(options.app_id());
  if (options.database_url() != nullptr) {
    pigeon_options.set_database_u_r_l(options.database_url());
  }
  pigeon_options.set_tracking_id(nullptr);
  pigeon_options.set_messaging_sender_id(options.messaging_sender_id());
  pigeon_options.set_project_id(options.project_id());
  if (options.storage_bucket() != nullptr) {
    pigeon_options.set_storage_bucket(options.storage_bucket());
  }
  return pigeon_options;
}

// Convert a firebase::App to PigeonInitializeResponse
PigeonInitializeResponse AppToPigeonInitializeResponse(const App& app) {
  PigeonInitializeResponse response = PigeonInitializeResponse();
  response.set_name(app.name());
  response.set_options(optionsFromFIROptions(app.options()));
  return response;
}

void FirebaseCoreTizenPlugin::InitializeApp(
    const std::string& app_name,
    const PigeonFirebaseOptions& initialize_app_request,
    std::function<void(ErrorOr<PigeonInitializeResponse> reply)> result) {
  firebase::App* app = firebase::App::GetInstance(app_name.c_str());
  if (!app) {
    app = firebase::App::Create(
        PigeonFirebaseOptionsToAppOptions(initialize_app_request),
        app_name.c_str());
  }

  // Send back the result to Flutter
  result(AppToPigeonInitializeResponse(*app));
}

void FirebaseCoreTizenPlugin::InitializeCore(
    std::function<void(ErrorOr<flutter::EncodableList> reply)> result) {
  // TODO: Missing function to get the list of currently initialized apps
  std::vector<PigeonInitializeResponse> initializedApps;
  std::vector<App*> all_apps = App::GetApps();
  for (const App* app : all_apps) {
    initializedApps.push_back(AppToPigeonInitializeResponse(*app));
  }

  flutter::EncodableList encodableList;

  for (const auto& item : initializedApps) {
    encodableList.push_back(flutter::CustomEncodableValue(item));
  }
  result(encodableList);
}

void FirebaseCoreTizenPlugin::OptionsFromResource(
    std::function<void(ErrorOr<PigeonFirebaseOptions> reply)> result) {
  // TODO: OptionsFromResource
}

void FirebaseCoreTizenPlugin::SetAutomaticDataCollectionEnabled(
    const std::string& app_name, bool enabled,
    std::function<void(std::optional<FlutterError> reply)> result) {
  // TODO: SetAutomaticDataCollectionEnabled;
  result(std::nullopt);
}

void FirebaseCoreTizenPlugin::SetAutomaticResourceManagementEnabled(
    const std::string& app_name, bool enabled,
    std::function<void(std::optional<FlutterError> reply)> result) {
  // TODO: SetAutomaticResourceManagementEnabled
  result(std::nullopt);
}

void FirebaseCoreTizenPlugin::Delete(
    const std::string& app_name,
    std::function<void(std::optional<FlutterError> reply)> result) {
  firebase::App* app = firebase::App::GetInstance(app_name.c_str());
  if (app) {
    delete app;
  }

  result(std::nullopt);
}

}  // namespace

void FirebaseCoreTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  FirebaseCoreTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
