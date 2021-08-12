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

#include <linux/bpf.h>

#include <bcc/BPF.h>
#include <functional>
#include <memory>
#include <platform/types.h>

/* forward declarations */
class ProbeHandler;

/**
 * Adds BPF probes for nat translations of connections via the kernel's
 * conntrack tables
 */
class NatProber {
public:
  /**
   * C'tor
   *
   * @param probe_handler: a ProbeHandler where new probes can be registered
   * @param bpf_module: the module from the bpf source code
   * @param periodic_cb: a callback to be called every once in a while, to
   *   allow user to e.g., flush rings
   */
  NatProber(ProbeHandler &probe_handler, ebpf::BPFModule &bpf_module, std::function<void(void)> periodic_cb);

private:
  /**
   * Queries the kernel for conntrack info
   */
  int query_kernel();

  std::function<void(void)> periodic_cb_;
};
