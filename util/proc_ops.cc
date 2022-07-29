// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <util/proc_ops.h>

#include <util/string_view.h>

#include <absl/container/flat_hash_map.h>

namespace {

template <typename T> void parse_proc_stat_field(views::NumberView<T> &out, std::string_view &data)
{
  using namespace views;
  trim_run(data, WHITESPACE);
  out = trim_up_to(data, WHITESPACE, SeekBehavior::CONSUME);
}

void parse_proc_stat_field(std::string_view &out, std::string_view &data)
{
  using namespace views;
  trim_up_to(data, '(', SeekBehavior::CONSUME);
  out = trim_up_to_last(data, ')', SeekBehavior::CONSUME);
}

void parse_proc_stat_field(ProcessState &out, std::string_view &data)
{
  using namespace views;
  trim_run(data, WHITESPACE);
  if (data.empty()) {
    out = ProcessState::unknown;
  } else {
    out = sanitize_enum(static_cast<ProcessState>(data.front()));
    data.remove_prefix(1);
  }
}

std::string_view parse_label(std::string_view &data)
{
  using namespace views;
  trim_run(data, WHITESPACE);
  return trim_up_to(data, ':', SeekBehavior::CONSUME);
}

} // namespace

static bool is_valid_value(ProcessState value)
{
  return true;
}

static bool is_valid_value(std::string_view value)
{
  return !value.empty();
}

template <typename T> static bool is_valid_value(views::NumberView<T> value)
{
  return !value.empty();
}

ProcStatView::ProcStatView(std::string_view data) : data_(data)
{
#define PROC_STAT_VIEW_PARSE_FIELDS(Type, Name, ...) parse_proc_stat_field(Name, data);
  PROC_STAT_VIEW_IMPL(PROC_STAT_VIEW_PARSE_FIELDS)
#undef PROC_STAT_VIEW_PARSE_FIELDS
}

bool ProcStatView::internal_validity_check() const
{
  return true
#define VERIFY_FIELDS(Type, Name, ...) &&is_valid_value(Name)
      PROC_STAT_VIEW_IMPL(VERIFY_FIELDS)
#undef VERIFY_FIELDS
          ;
}

namespace {
auto const proc_status_view_parse_map = [] {
  // set up parsing map for labels
  using namespace views;

  using parser_fn = void (*)(ProcStatusView &, std::string_view &);
  absl::flat_hash_map<std::string_view, parser_fn> map;

#define PROC_STATUS_VIEW_PARSE_FIELDS(Type, Name, Label, ...)                                                                  \
  map[Label] = [](ProcStatusView &out, std::string_view &data) {                                                               \
    trim_run(data, NON_EOL_WHITESPACE);                                                                                        \
    auto const view = trim_up_to(data, EOL, SeekBehavior::CONSUME);                                                            \
    out.Name = view;                                                                                                           \
  };
  PROC_STATUS_VIEW_IMPL(PROC_STATUS_VIEW_PARSE_FIELDS)
#undef PROC_STATUS_VIEW_PARSE_FIELDS

  return map;
}();
} // namespace

ProcStatusView::ProcStatusView(std::string_view data) : data_(data)
{
  using namespace views;

  while (!data.empty()) {
    auto const label = parse_label(data);
    auto const parser = proc_status_view_parse_map.find(label);
    if (parser == proc_status_view_parse_map.end()) {
      trim_up_to(data, EOL, SeekBehavior::CONSUME);
    } else {
      parser->second(*this, data);
    }
  }
}

bool ProcStatusView::internal_validity_check() const
{
  return true
#define VERIFY_FIELDS(Type, Name, ...) &&!Name.empty()
      PROC_STATUS_VIEW_IMPL(VERIFY_FIELDS)
#undef VERIFY_FIELDS
          ;
}

namespace {
auto const proc_io_view_parse_map = [] {
  // set up parsing map for labels
  using namespace views;

  using parser_fn = void (*)(ProcIoView &, std::string_view &);
  absl::flat_hash_map<std::string_view, parser_fn> map;

#define PROC_IO_VIEW_PARSE_FIELDS(Type, Name, Label, ...)                                                                      \
  map[Label] = [](ProcIoView &out, std::string_view &data) {                                                                   \
    trim_run(data, NON_EOL_WHITESPACE);                                                                                        \
    auto const view = trim_up_to(data, EOL, SeekBehavior::CONSUME);                                                            \
    out.Name = view;                                                                                                           \
  };
  PROC_IO_VIEW_IMPL(PROC_IO_VIEW_PARSE_FIELDS)
#undef PROC_IO_VIEW_PARSE_FIELDS

  return map;
}();
} // namespace

ProcIoView::ProcIoView(std::string_view data) : data_(data)
{
  using namespace views;

  while (!data.empty()) {
    auto const label = parse_label(data);
    auto const parser = proc_io_view_parse_map.find(label);
    if (parser == proc_io_view_parse_map.end()) {
      trim_up_to(data, EOL, SeekBehavior::CONSUME);
    } else {
      parser->second(*this, data);
    }
  }
}

bool ProcIoView::internal_validity_check() const
{
  return true
#define VERIFY_FIELDS(Type, Name, ...) &&!Name.empty()
      PROC_IO_VIEW_IMPL(VERIFY_FIELDS)
#undef VERIFY_FIELDS
          ;
}
