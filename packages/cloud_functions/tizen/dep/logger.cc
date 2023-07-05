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
#include "include/common/logger.h"

#include <map>
#include <regex>
#include <set>
#include <thread>

// --- Formatter ---

std::string getPrettyFunctionName(const std::string& fullname,
                                  std::string prefixPattern) {
  std::stringstream ss;
  if (!prefixPattern.empty()) {
    ss << "(?:" << prefixPattern << ")|";
  }
  ss << R"((?::\()|([\w:~]+)\()";

  try {
    std::smatch match;
    const std::regex re(ss.str());

    std::stringstream result;
    std::string suffix = fullname;
    while (std::regex_search(suffix, match, re)) {
      result << match[1];
      suffix = match.suffix();
    }
    return result.str();
  } catch (std::regex_error& e) {
    return "";
  }
}

std::string createCodeLocation(const char* functionName, const char* filename,
                               const int line, std::string prefixPattern) {
  std::ostringstream oss;
  oss << getPrettyFunctionName(functionName, prefixPattern) << " (" << filename
      << ":" << line << ")";
  return oss.str();
}

void writeThreadIdentifier(std::ostream& os) {
  static int s_id = 0;
  static thread_local int thisThreadId = 0;

  if (thisThreadId == 0) {
    thisThreadId = ++s_id;
  }
  os << "[" << thisThreadId << "] ";
}

std::function<bool(const std::string&)> LogOption::s_externalIsEnabled;

void LogOption::setExternalIsEnabled(
    std::function<bool(const std::string&)> func) {
  s_externalIsEnabled = func;
}

// --- LogOption ---

bool LogOption::isEnabled(const std::string& pattern) {
  if (s_externalIsEnabled == nullptr) {
    return true;
  }
  return s_externalIsEnabled(pattern);
}

// --- Logger::Header ---

void Logger::Header::write(std::stringstream& stream) {
  writeThreadIdentifier(stream);
  writeHeader(stream);
}

// --- Logger ---

Logger::Logger(std::shared_ptr<Output> out) { initialize(out); }

Logger::Logger(const std::string& header, std::shared_ptr<Output> out)
    : output_(out) {
  initialize(output_);
  stream_ << header;
}

Logger::Logger(Header&& header, std::shared_ptr<Output> out) : output_(out) {
  initialize(output_);
  header.write(stream_);
}

Logger::~Logger() {
  if (!isEnabled() || output_ == nullptr) {
    return;
  }
  // stream ends with both reset-styles and endl characters.
  stream_ << "\033[0m" << std::endl;
  output_->flush(stream_);
}

void Logger::initialize(std::shared_ptr<Output> out) {
  static thread_local std::shared_ptr<StdOut> s_loggerOutput;

  if (out == nullptr) {
    if (s_loggerOutput == nullptr) {
      s_loggerOutput = std::make_shared<StdOut>();
    }
    output_ = s_loggerOutput;
  } else {
    output_ = out;
  }
}

Logger& Logger::print(const char* string_without_format_specifiers) {
  if (output_ == nullptr) {
    return *this;
  }

  while (*string_without_format_specifiers) {
    if (*string_without_format_specifiers == '%' &&
        *(++string_without_format_specifiers) != '%') {
      assert(((void)"runtime error: invalid format-string", false));
    }
    stream_ << *string_without_format_specifiers++;
  }
  return *this;
}

Logger& Logger::flush() {
  if (isEnabled() && output_) {
    output_->flush(stream_);
  }
  stream_.str("");
  return *this;
}
// --- Output ---

void StdOut::flush(std::stringstream& stream) { std::cout << stream.str(); }

// --- Utils ---

thread_local int s_indentCount = 0;
thread_local int s_deltaCount = 0;

void IndentCounter::indent(std::string id) {
  if (!LogOption::isEnabled(id)) {
    return;
  }
  s_deltaCount++;
}

void IndentCounter::unIndent(std::string id) {
  if (!LogOption::isEnabled(id)) {
    return;
  }
  s_deltaCount--;
}

IndentCounter::IndentCounter(std::string id) {
  id_ = id;

  if (!LogOption::isEnabled(id)) {
    return;
  }
  s_indentCount++;
}

IndentCounter::~IndentCounter() {
  if (!LogOption::isEnabled(id_)) {
    return;
  }
  s_indentCount--;
}

std::string IndentCounter::getString(std::string id) {
  assert(s_indentCount >= 0);

  std::ostringstream oss;
  int indentCount = s_indentCount + s_deltaCount;

  if (s_deltaCount > 0) {
    oss << s_deltaCount << " ";
  }

  for (int i = 1; i < std::min(30, indentCount); ++i) {
    oss << "  ";
  }

  return oss.str();
}
