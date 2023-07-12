// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "firebase_core_tizen_plugin.h"

#include <firebase/app.h>
#include <firebase/log.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "log.h"
#include "messages.h"

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

  firebase::SetLogLevel(firebase::LogLevel::kLogLevelVerbose);
}

FirebaseCoreTizenPlugin::~FirebaseCoreTizenPlugin() {}

static PigeonFirebaseOptions FirebaseOptionsToPigeonOptions(
    const firebase::AppOptions& options) {
  PigeonFirebaseOptions pigeon_options;

  pigeon_options.set_api_key(options.api_key());
  pigeon_options.set_app_id(options.app_id());
  pigeon_options.set_messaging_sender_id(options.messaging_sender_id());
  pigeon_options.set_project_id(options.project_id());
  if (strlen(options.database_url()) > 0) {
    pigeon_options.set_database_u_r_l(options.database_url());
  }
  if (strlen(options.storage_bucket()) > 0) {
    pigeon_options.set_storage_bucket(options.storage_bucket());
  }
  if (strlen(options.ga_tracking_id()) > 0) {
    pigeon_options.set_tracking_id(options.ga_tracking_id());
  }

  return pigeon_options;
}

static firebase::AppOptions PigeonOptionsToFirebaseOptions(
    const PigeonFirebaseOptions& options) {
  firebase::AppOptions app_options;

  app_options.set_api_key(options.api_key().c_str());
  app_options.set_app_id(options.app_id().c_str());
  app_options.set_messaging_sender_id(options.messaging_sender_id().c_str());
  app_options.set_project_id(options.project_id().c_str());
  if (options.database_u_r_l()) {
    app_options.set_database_url(options.database_u_r_l()->c_str());
  }
  if (options.storage_bucket()) {
    app_options.set_storage_bucket(options.storage_bucket()->c_str());
  }
  if (options.tracking_id()) {
    app_options.set_ga_tracking_id(options.tracking_id()->c_str());
  }

  return app_options;
}

void FirebaseCoreTizenPlugin::InitializeApp(
    const std::string& app_name,
    const PigeonFirebaseOptions& initialize_app_request,
    std::function<void(ErrorOr<PigeonInitializeResponse> reply)> result) {
  firebase::App* app = firebase::App::GetInstance(app_name.c_str());
  if (!app) {
    app = firebase::App::Create(
        PigeonOptionsToFirebaseOptions(initialize_app_request),
        app_name.c_str());
  }

  PigeonInitializeResponse message_response;
  message_response.set_name(app->name());
  message_response.set_options(FirebaseOptionsToPigeonOptions(app->options()));

  result(message_response);
}

void FirebaseCoreTizenPlugin::InitializeCore(
    std::function<void(ErrorOr<flutter::EncodableList> reply)> result) {
  flutter::EncodableList response;

  std::vector<firebase::App*> firebase_apps = firebase::App::GetApps();
  for (auto app : firebase_apps) {
    PigeonInitializeResponse message_response;
    message_response.set_name(app->name());
    message_response.set_options(
        FirebaseOptionsToPigeonOptions(app->options()));

    response.push_back(
        flutter::EncodableValue(message_response.ToEncodableMap()));
  }

  result(response);
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
