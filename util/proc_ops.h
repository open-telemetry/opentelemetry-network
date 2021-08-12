//
// Copyright 2021 Splunk Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include <util/expected.h>
#include <util/process_state.h>
#include <util/string_view.h>

#include <string_view>

#define PROC_VIEWS_COMMON_CODE(ClassName)                                                                                      \
public:                                                                                                                        \
  ClassName() = default;                                                                                                       \
  ClassName(std::string_view data);                                                                                            \
                                                                                                                               \
  explicit operator bool() const { return !data_.empty() && internal_validity_check(); }                                       \
  bool operator!() const { return !static_cast<bool>(*this); }                                                                 \
  bool valid() const { return static_cast<bool>(*this); }                                                                      \
  std::string_view view() const { return data_; }                                                                              \
                                                                                                                               \
private:                                                                                                                       \
  bool internal_validity_check() const;                                                                                        \
  std::string_view data_;

#include <util/proc_stat_view.inl>
struct ProcStatView {
#define PROC_STAT_VIEW_DECLARE_FIELDS(Type, Name, ...) Type Name;
  PROC_STAT_VIEW_IMPL(PROC_STAT_VIEW_DECLARE_FIELDS)
#undef PROC_STAT_VIEW_DECLARE_FIELDS

  PROC_VIEWS_COMMON_CODE(ProcStatView);
};

#include <util/proc_status_view.inl>
struct ProcStatusView {
#define PROC_STATUS_VIEW_DECLARE_FIELDS(Type, Name, ...) Type Name;
  PROC_STATUS_VIEW_IMPL(PROC_STATUS_VIEW_DECLARE_FIELDS)
#undef PROC_STATUS_VIEW_DECLARE_FIELDS

  PROC_VIEWS_COMMON_CODE(ProcStatusView);
};

#include <util/proc_io_view.inl>
struct ProcIoView {
#define PROC_IO_VIEW_DECLARE_FIELDS(Type, Name, ...) Type Name;
  PROC_IO_VIEW_IMPL(PROC_IO_VIEW_DECLARE_FIELDS)
#undef PROC_IO_VIEW_DECLARE_FIELDS

  PROC_VIEWS_COMMON_CODE(ProcIoView);
};
