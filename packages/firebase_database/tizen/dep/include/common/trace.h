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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either xess or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include "logger.h"

class Trace : public Logger {
 public:
  Trace(std::string id);
  Trace(std::string id, const char* functionName, const char* filename,
        const int line);
  template <typename T, typename... TArgs>
  Trace(std::string id, const char* functionName, const char* filename,
        const int line, const T& v, TArgs... args)
      : Trace(id, functionName, filename, line) {
    if (!isEnabled(id)) {
      return;
    }
    stream_ << v << " ";
    log(args...);
  }

  class Option {
   public:
    static void setTag(const char* tagName) { tag_ = tagName; }
    static const char* tag() { return tag_.c_str(); }

   private:
    static std::string tag_;
  };
};

#if defined(NDEBUG)

#define TRACE(id, ...)
#define TRACE0(id, ...)
#define TRACEF(id, ...)
#define TRACEF0(id, ...)
#define TRACE_SCOPE(id, ...)
#define TRACE_SCOPE0(id, ...)

#else

#define TRACE(id, ...) \
  Trace(#id, __PRETTY_FUNCTION__, __FILE_NAME__, __LINE__).log(__VA_ARGS__)

#define TRACE0(id, ...) Trace(#id).log(__VA_ARGS__)

#define TRACEF(id, ...) \
  Trace(#id, __PRETTY_FUNCTION__, __FILE_NAME__, __LINE__).print(__VA_ARGS__)

#define TRACEF0(id, ...) Trace(#id).print(__VA_ARGS__)

#define TRACE_SCOPE(id, ...)    \
  IndentCounter __counter(#id); \
  TRACE(id, __VA_ARGS__)

#define TRACE_SCOPE0(id, ...)                                       \
  IndentCounter __counter(#id);                                     \
  TRACE(id, __VA_ARGS__);                                           \
  Trace __outter(#id, __PRETTY_FUNCTION__, __FILE_NAME__, __LINE__, \
                 "/" __VA_ARGS__)

#endif

#ifdef __GNUC__
#define LIKELY(condition) __builtin_expect(!!(condition), 1)
#define UNLIKELY(condition) __builtin_expect(!!(condition), 0)
#else
#define LIKELY(condition) (condition)
#define UNLIKELY(condition) (condition)
#endif

void Trace_Fatal(const char* functionName, const char* filename, const int line,
                 const std::string message);

#define FATAL(...) \
  Trace_Fatal(__PRETTY_FUNCTION__, __FILE_NAME__, __LINE__, __VA_ARGS__)
#define CHECK_FAILED_HANDLER(message) FATAL("Check failed: " message)
#define CHECK_WITH_MSG(condition, message) \
  do {                                     \
    if (UNLIKELY(!(condition))) {          \
      CHECK_FAILED_HANDLER(message);       \
    }                                      \
  } while (false)
#define CHECK(condition) CHECK_WITH_MSG(condition, #condition)
#define CHECK_NULL(val) CHECK((val) == nullptr)
#define CHECK_NOT_NULL(val) CHECK((val) != nullptr)
#define UNIMPLEMENTED(...) CHECK_WITH_MSG(false, "[UNIMPLEMENTED] " __VA_ARGS__)
