// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <collector/kernel/process_prober.h>

#include <linux/bpf.h>

#include <bcc/BPF.h>
#include <collector/kernel/fd_reader.h>
#include <collector/kernel/probe_handler.h>
#include <collector/kernel/proc_reader.h>

ProcessProber::ProcessProber(
    ProbeHandler &probe_handler,
    ebpf::BPFModule &bpf_module,
    std::function<void(void)> periodic_cb,
    std::function<void(std::string)> check_cb)
{
  // END
  probe_handler.start_probe(bpf_module, "on_taskstats_exit", "taskstats_exit");
  probe_handler.start_probe(bpf_module, "on_cgroup_exit", "cgroup_exit");
  probe_handler.start_kretprobe(bpf_module, "onret_cgroup_exit", "cgroup_exit");

  // START
  /* probe for new process info
   * Note that other functions we considered were:
   * sys_execve, sched_fork, __sched_fork, _do_fork, sched_exec
   */
  probe_handler.start_probe(bpf_module, "on_wake_up_new_task", "wake_up_new_task");
  probe_handler.start_probe(bpf_module, "on_set_task_comm", "__set_task_comm");

  // EXISTING
  probe_handler.start_kretprobe(bpf_module, "onret_get_pid_task", "get_pid_task");
  periodic_cb();
  check_cb("process prober startup");

  // iterate over /proc/ to trigger on_get_pid_task()
  trigger_get_pid_task(periodic_cb);
  check_cb("trigger_get_pid_task()");

  // we can remove the probe for existing now
  probe_handler.cleanup_probe("r_get_pid_task");
  periodic_cb();
  check_cb("process prober cleanup()");
}

void ProcessProber::trigger_get_pid_task(std::function<void(void)> periodic_cb)
{
  ProcReader proc_reader;
  while (proc_reader.next()) {
    periodic_cb();

    if (!proc_reader.is_pid())
      continue; // skip this entry if this wasn't a pid directory

    int pid = proc_reader.get_pid();
    FDReader fd_reader(pid);
    int status = fd_reader.open_task_dir();
    if (status)
      continue; // skip this entry because task_dir couldn't be opened

    // for each thread in this group
    while (!fd_reader.next_task()) {
      // read a file from this tid so that our probe can generate a msg.
      // we don't care about the return value
      fd_reader.open_task_comm();

      periodic_cb();
    }
  }
}
