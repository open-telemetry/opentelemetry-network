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

#include <platform/types.h>
#include <util/expected.h>

#include <string>
#include <system_error>

// Reads the process command-line from /proc/PID/cmdline.
Expected<std::string, std::error_code> read_proc_cmdline(u32 pid);

// Reads the process command-line from /proc/PID/cmdline.
// On ENOENT (no such file or directory) error, returns an empty string
// instead of an error code.
Expected<std::string, std::error_code> try_read_proc_cmdline(u32 pid);
