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
#ifndef __LOG_H__
#define __LOG_H__

#define CLR_RESET "\033[0m"
#define CLR_DIM "\033[0;2m"
#define CLR_RED "\033[0;31m"
#define CLR_YELLOW "\033[0;33m"
#define CLR_LT_YELLOW "\033[1;33m"
#define CLR_BG_GREEN "\033[0;42m"

#if defined(__TIZEN__) || defined(TIZEN)

#include <dlog.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "CloudFunctions"

#ifndef __MODULE__
#define __MODULE__ strrchr("/" __FILE__, '/') + 1
#endif

#define LOG(prio, fmt, arg...)                                        \
  dlog_print(prio, LOG_TAG, fmt CLR_DIM " (%s:%d) " CLR_RESET, ##arg, \
             __MODULE__, __LINE__)

#define LOGD(msg, args...) LOG(DLOG_DEBUG, CLR_BG_GREEN msg CLR_RESET, ##args)
#define LOGI(msg, args...) LOG(DLOG_INFO, CLR_YELLOW msg CLR_RESET, ##args)
#define LOGW(msg, args...) LOG(DLOG_WARN, CLR_LT_YELLOW msg CLR_RESET, ##args)
#define LOGE(msg, args...) LOG(DLOG_ERROR, CLR_RED msg CLR_RESET, ##args)

#endif

#endif  // __LOG_H__
