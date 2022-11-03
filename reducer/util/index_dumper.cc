// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <reducer/util/index_dumper.h>

#include <platform/types.h>

std::string IndexDumper::dump_dir_{};

void IndexDumper::set_dump_dir(std::string_view dir)
{
  dump_dir_ = dir;
}

std::chrono::seconds IndexDumper::cooldown_{0};

void IndexDumper::set_cooldown(std::chrono::seconds cooldown)
{
  cooldown_ = cooldown;
}
