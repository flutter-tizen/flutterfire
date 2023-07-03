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
#include "common/trace.h"

#include <cassert>  // assert
#include <iomanip>  // setfill and setw

#define TYPE_LENGTH_LIMIT 5
#define TRACE_ID_LENGTH_LIMIT 10
#define COLOR_RESET "\033[0m"
#define COLOR_DIM "\033[0;2m"
#define LOG_PREFIX_PATTERN ".*TizenPlugin"

std::string Trace::Option::s_tag_;

#if defined(__TIZEN__) || defined(TIZEN)

#include <dlog.h>

class PriorityLog {
 public:
  class Info : public Logger::Output {
   public:
    void flush(std::stringstream& stream) override {
      dlog_print(DLOG_INFO, Trace::Option::tag(), stream.str().c_str());
    }
  };

  class Warn : public Logger::Output {
   public:
    void flush(std::stringstream& stream) override {
      dlog_print(DLOG_WARN, Trace::Option::tag(), stream.str().c_str());
    }
  };

  class Error : public Logger::Output {
   public:
    void flush(std::stringstream& stream) override {
      dlog_print(DLOG_ERROR, Trace::Option::tag(), stream.str().c_str());
    }
  };
};

class CustomOutput : public PriorityLog::Info {
 public:
  static std::shared_ptr<CustomOutput> instance() {
    static std::shared_ptr<CustomOutput> output =
        std::make_shared<CustomOutput>();
    return output;
  }
};

// -- Fatal --
static std::string trim(std::string const& str,
                        std::string const& whitespace = " \r\n\t\v\f") {
  if (str.length() == 0) return "";
  auto start = str.find_first_not_of(whitespace);
  auto end = str.find_last_not_of(whitespace);
  return str.substr(start, end - start + 1);
}

void Trace_Fatal(const char* functionName, const char* filename, const int line,
                 const std::string message) {
  std::stringstream stream;
  if (message.length() > 0) stream << trim(message) << " ";
  stream << createCodeLocation(functionName, filename, line);
  PriorityLog::Error().flush(stream);
  assert(false);
}

#else

class CustomOutput : public Logger::Output {
 public:
  void flush(std::stringstream& ss) override {
    // NOTE: We use stdout for now since there is no macro to print
    // a raw string only. e.g) STARFISH_LOG_INFO("%s", "blahblah");
    std::cout << ss.str();
  };

  static std::shared_ptr<CustomOutput> instance() {
    static std::shared_ptr<CustomOutput> output =
        std::make_shared<CustomOutput>();
    return output;
  }
};
#endif

static void writeHeader(std::ostream& ss, const std::string& tag,
                        const std::string& id) {
  ss << COLOR_DIM;
  ss << std::left << std::setfill(' ') << "("
     << std::setw(TRACE_ID_LENGTH_LIMIT)
     << std::string(id).substr(0, TRACE_ID_LENGTH_LIMIT) << ") ";
}

Trace::Trace(std::string id, const char* functionName, const char* filename,
             const int line) {
  if (!LogOption::isEnabled(id)) {
    return;
  }

  writeHeader(stream_, "TRACE", id);
  stream_ << IndentCounter::getString(id)
          << createCodeLocation(functionName, filename, line,
                                LOG_PREFIX_PATTERN)
          << " " << COLOR_RESET;
  initialize(CustomOutput::instance());
}

Trace::Trace(std::string id) {
  if (!LogOption::isEnabled(id)) {
    return;
  }

  writeHeader(stream_, "INFO", id);
  initialize(CustomOutput::instance());
}
