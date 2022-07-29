/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

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
