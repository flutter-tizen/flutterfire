// Autogenerated from Pigeon (v3.2.9), do not edit directly.
// See also: https://pub.dev/packages/pigeon

#ifndef PIGEON_MESSAGES_H_
#define PIGEON_MESSAGES_H_
#include <flutter/basic_message_channel.h>
#include <flutter/binary_messenger.h>
#include <flutter/encodable_value.h>
#include <flutter/standard_message_codec.h>

#include <map>
#include <optional>
#include <string>

/* Generated class from Pigeon. */

class FlutterError {
 public:
  FlutterError(const std::string& code) : code_(code) {}
  FlutterError(const std::string& code, const std::string& message)
      : code_(code), message_(message) {}
  FlutterError(const std::string& code, const std::string& message,
               const flutter::EncodableValue& details)
      : code_(code), message_(message), details_(details) {}

  const std::string& code() const { return code_; }
  const std::string& message() const { return message_; }
  const flutter::EncodableValue& details() const { return details_; }

 private:
  std::string code_;
  std::string message_;
  flutter::EncodableValue details_;
};

template <class T>
class ErrorOr {
 public:
  ErrorOr(const T& rhs) { new (&v_) T(rhs); }
  ErrorOr(const T&& rhs) { v_ = std::move(rhs); }
  ErrorOr(const FlutterError& rhs) { new (&v_) FlutterError(rhs); }
  ErrorOr(const FlutterError&& rhs) { v_ = std::move(rhs); }

  bool has_error() const { return std::holds_alternative<FlutterError>(v_); }
  const T& value() const { return std::get<T>(v_); };
  const FlutterError& error() const { return std::get<FlutterError>(v_); };

 private:
  friend class FirebaseCoreHostApi;
  friend class FirebaseAppHostApi;
  ErrorOr() = default;
  T TakeValue() && { return std::get<T>(std::move(v_)); }

  std::variant<T, FlutterError> v_;
};

/* Generated class from Pigeon that represents data sent in messages. */
class PigeonFirebaseOptions {
 public:
  PigeonFirebaseOptions();
  const std::string& api_key() const;
  void set_api_key(std::string_view value_arg);

  const std::string& app_id() const;
  void set_app_id(std::string_view value_arg);

  const std::string& messaging_sender_id() const;
  void set_messaging_sender_id(std::string_view value_arg);

  const std::string& project_id() const;
  void set_project_id(std::string_view value_arg);

  const std::string* auth_domain() const;
  void set_auth_domain(const std::string_view* value_arg);
  void set_auth_domain(std::string_view value_arg);

  const std::string* database_u_r_l() const;
  void set_database_u_r_l(const std::string_view* value_arg);
  void set_database_u_r_l(std::string_view value_arg);

  const std::string* storage_bucket() const;
  void set_storage_bucket(const std::string_view* value_arg);
  void set_storage_bucket(std::string_view value_arg);

  const std::string* measurement_id() const;
  void set_measurement_id(const std::string_view* value_arg);
  void set_measurement_id(std::string_view value_arg);

  const std::string* tracking_id() const;
  void set_tracking_id(const std::string_view* value_arg);
  void set_tracking_id(std::string_view value_arg);

  const std::string* deep_link_u_r_l_scheme() const;
  void set_deep_link_u_r_l_scheme(const std::string_view* value_arg);
  void set_deep_link_u_r_l_scheme(std::string_view value_arg);

  const std::string* android_client_id() const;
  void set_android_client_id(const std::string_view* value_arg);
  void set_android_client_id(std::string_view value_arg);

  const std::string* ios_client_id() const;
  void set_ios_client_id(const std::string_view* value_arg);
  void set_ios_client_id(std::string_view value_arg);

  const std::string* ios_bundle_id() const;
  void set_ios_bundle_id(const std::string_view* value_arg);
  void set_ios_bundle_id(std::string_view value_arg);

  const std::string* app_group_id() const;
  void set_app_group_id(const std::string_view* value_arg);
  void set_app_group_id(std::string_view value_arg);

 private:
  PigeonFirebaseOptions(flutter::EncodableMap map);
  flutter::EncodableMap ToEncodableMap() const;
  friend class PigeonInitializeResponse;
  friend class FirebaseCoreHostApi;
  friend class FirebaseCoreHostApiCodecSerializer;
  friend class FirebaseAppHostApi;
  friend class FirebaseAppHostApiCodecSerializer;
  std::string api_key_;
  std::string app_id_;
  std::string messaging_sender_id_;
  std::string project_id_;
  std::optional<std::string> auth_domain_;
  std::optional<std::string> database_u_r_l_;
  std::optional<std::string> storage_bucket_;
  std::optional<std::string> measurement_id_;
  std::optional<std::string> tracking_id_;
  std::optional<std::string> deep_link_u_r_l_scheme_;
  std::optional<std::string> android_client_id_;
  std::optional<std::string> ios_client_id_;
  std::optional<std::string> ios_bundle_id_;
  std::optional<std::string> app_group_id_;
};

/* Generated class from Pigeon that represents data sent in messages. */
class PigeonInitializeResponse {
 public:
  PigeonInitializeResponse();
  const std::string& name() const;
  void set_name(std::string_view value_arg);

  const PigeonFirebaseOptions& options() const;
  void set_options(const PigeonFirebaseOptions& value_arg);

  const bool* is_automatic_data_collection_enabled() const;
  void set_is_automatic_data_collection_enabled(const bool* value_arg);
  void set_is_automatic_data_collection_enabled(bool value_arg);

  const flutter::EncodableMap& plugin_constants() const;
  void set_plugin_constants(const flutter::EncodableMap& value_arg);

  flutter::EncodableMap ToEncodableMap() const;

 private:
  PigeonInitializeResponse(flutter::EncodableMap map);
  friend class FirebaseCoreHostApi;
  friend class FirebaseCoreHostApiCodecSerializer;
  friend class FirebaseAppHostApi;
  friend class FirebaseAppHostApiCodecSerializer;
  std::string name_;
  PigeonFirebaseOptions options_;
  std::optional<bool> is_automatic_data_collection_enabled_;
  flutter::EncodableMap plugin_constants_;
};

class FirebaseCoreHostApiCodecSerializer
    : public flutter::StandardCodecSerializer {
 public:
  inline static FirebaseCoreHostApiCodecSerializer& GetInstance() {
    static FirebaseCoreHostApiCodecSerializer sInstance;
    return sInstance;
  }

  FirebaseCoreHostApiCodecSerializer();

 public:
  void WriteValue(const flutter::EncodableValue& value,
                  flutter::ByteStreamWriter* stream) const override;

 protected:
  flutter::EncodableValue ReadValueOfType(
      uint8_t type, flutter::ByteStreamReader* stream) const override;
};

/* Generated class from Pigeon that represents a handler of messages from
 * Flutter. */
class FirebaseCoreHostApi {
 public:
  FirebaseCoreHostApi(const FirebaseCoreHostApi&) = delete;
  FirebaseCoreHostApi& operator=(const FirebaseCoreHostApi&) = delete;
  virtual ~FirebaseCoreHostApi(){};
  virtual void InitializeApp(
      const std::string& app_name,
      const PigeonFirebaseOptions& initialize_app_request,
      std::function<void(ErrorOr<PigeonInitializeResponse> reply)> result) = 0;
  virtual void InitializeCore(
      std::function<void(ErrorOr<flutter::EncodableList> reply)> result) = 0;
  virtual void OptionsFromResource(
      std::function<void(ErrorOr<PigeonFirebaseOptions> reply)> result) = 0;

  /** The codec used by FirebaseCoreHostApi. */
  static const flutter::StandardMessageCodec& GetCodec();
  /** Sets up an instance of `FirebaseCoreHostApi` to handle messages through
   * the `binary_messenger`. */
  static void SetUp(flutter::BinaryMessenger* binary_messenger,
                    FirebaseCoreHostApi* api);
  static flutter::EncodableMap WrapError(std::string_view error_message);
  static flutter::EncodableMap WrapError(const FlutterError& error);

 protected:
  FirebaseCoreHostApi() = default;
};
class FirebaseAppHostApiCodecSerializer
    : public flutter::StandardCodecSerializer {
 public:
  inline static FirebaseAppHostApiCodecSerializer& GetInstance() {
    static FirebaseAppHostApiCodecSerializer sInstance;
    return sInstance;
  }

  FirebaseAppHostApiCodecSerializer();
};

/* Generated class from Pigeon that represents a handler of messages from
 * Flutter. */
class FirebaseAppHostApi {
 public:
  FirebaseAppHostApi(const FirebaseAppHostApi&) = delete;
  FirebaseAppHostApi& operator=(const FirebaseAppHostApi&) = delete;
  virtual ~FirebaseAppHostApi(){};
  virtual void SetAutomaticDataCollectionEnabled(
      const std::string& app_name, bool enabled,
      std::function<void(std::optional<FlutterError> reply)> result) = 0;
  virtual void SetAutomaticResourceManagementEnabled(
      const std::string& app_name, bool enabled,
      std::function<void(std::optional<FlutterError> reply)> result) = 0;
  virtual void Delete(
      const std::string& app_name,
      std::function<void(std::optional<FlutterError> reply)> result) = 0;

  /** The codec used by FirebaseAppHostApi. */
  static const flutter::StandardMessageCodec& GetCodec();
  /** Sets up an instance of `FirebaseAppHostApi` to handle messages through the
   * `binary_messenger`. */
  static void SetUp(flutter::BinaryMessenger* binary_messenger,
                    FirebaseAppHostApi* api);
  static flutter::EncodableMap WrapError(std::string_view error_message);
  static flutter::EncodableMap WrapError(const FlutterError& error);

 protected:
  FirebaseAppHostApi() = default;
};
#endif  // PIGEON_MESSAGES_H_
