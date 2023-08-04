/*
 * Copyright (c) 2022-present Samsung Electronics Co., Ltd
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

#ifndef FIREBASE_TIZEN_DEP_COMMON_LOGGER_H_
#define FIREBASE_TIZEN_DEP_COMMON_LOGGER_H_

#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

class LogOption {
 public:
  static bool isEnabled(const std::string& pattern = "");
  static void setExternalIsEnabled(std::function<bool(const std::string&)>);

 private:
  static std::function<bool(const std::string&)> externalIsEnabled;
};

class Logger {
 public:
  class Header {
   protected:
    virtual void writeHeader(std::stringstream&) = 0;

   private:
    void write(std::stringstream& stream);
    friend class Logger;
  };

  class Output {
   public:
    virtual ~Output() = default;
    virtual void flush(std::stringstream& ss) = 0;
  };

  Logger(std::shared_ptr<Output> out = nullptr);
  Logger(const std::string& header, std::shared_ptr<Output> out = nullptr);
  Logger(Header&& header, std::shared_ptr<Output> out = nullptr);
  virtual ~Logger();

  template <class T>
  Logger& operator<<(const T& msg) {
    if (!isEnabled()) {
      return *this;
    }
    stream_ << msg;
    return *this;
  }

  template <typename T, typename... TArgs>
  Logger& log(const T& v, TArgs... args) {
    if (!isEnabled()) {
      return *this;
    }
    stream_ << v << " ";
    log(args...);
    return *this;
  }
  template <typename T>
  Logger& log(const T& v) {
    if (!isEnabled()) {
      return *this;
    }
    stream_ << v;
    return *this;
  }
  Logger& log() { return *this; }

  template <typename T, typename... Args>
  Logger& print(const char* format, T value, Args... args) {
    if (!isEnabled()) {
      return *this;
    }

    while (*format) {
      if (*format == '%' && *(++format) != '%') {
        stream_ << value;

        // handle sub-specifiers
        if ((*format == 'z')) {
          format++;
        } else if ((*format == 'l') || (*format == 'h')) {
          format++;
          if (*format == *(format + 1)) {
            format++;
          }
        }
        format++;

        print(format, args...);
        return *this;
      }
      stream_ << *format++;
    }
    assert(((void)"logical error: should not come here", false));
    return *this;
  };

  Logger& print(const char* string_without_format_specifiers = "");
  Logger& flush();
  bool isEnabled(const std::string& pattern = "") {
    return LogOption::isEnabled(pattern);
  }

 protected:
  std::stringstream stream_;
  void initialize(std::shared_ptr<Output> out = nullptr);

 private:
  std::shared_ptr<Output> output_;
};

class StdOut : public Logger::Output {
 public:
  static std::shared_ptr<StdOut> instance() {
    static std::shared_ptr<StdOut> output = std::make_shared<StdOut>();
    return output;
  }

  void flush(std::stringstream& ss) override;
};

// --- Utils ---

#ifndef __FILE_NAME__
#define __FILE_NAME__ \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#define __FUNCTION_NAME__ getPrettyFunctionName(__PRETTY_FUNCTION__)

#define __CODE_LOCATION__ \
  createCodeLocation(__PRETTY_FUNCTION__, __FILE_NAME__, __LINE__).c_str()

std::string createCodeLocation(const char* functionName, const char* filename,
                               const int line, std::string prefixPattern = "");

std::string getPrettyFunctionName(const std::string& fullname,
                                  std::string prefixPattern = "");

void writeThreadIdentifier(std::ostream& ss);

class IndentCounter {
 public:
  IndentCounter(std::string id);
  ~IndentCounter();
  static std::string getString(std::string id = "");
  static void indent(std::string id);
  static void unIndent(std::string id);

 private:
  std::string id_;
};

#endif  // FIREBASE_TIZEN_DEP_COMMON_LOGGER_H_
