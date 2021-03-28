---
description: >-
  How process and cgroup information is sent from eBPF to the kernel collector
  user-space
---

# Processes and cgroups

## Processes

### Messages

* pid\_info
* pid\_close
* pid\_set\_comm

The _pid\_info_ message is sent whenever a new process is detected. That can be during startup, when processes are enumerated by visiting the _proc_ filesystem, or during the steady state when a new process is first scheduled to run.

The _pid\_close_ message is sent when a process exits.

The _pid\_set\_comm_ message is sent when the command-line is set for the process, e.g. during the _exec_ system call.

All messages identify the process through the `u32 pid` field \(process ID\).

## Cgroups

Cgroups are a kernel feature that is, among other uses, utilized for implementing containerization \(e.g. Docker\). Cgroups are also used by systemd.

### Messages

* css\_populate\_dir
* cgroup\_clone\_children\_read
* cgroup\_attach\_task
* kill\_css

The _css\_populate\_dir_ message is sent when a new cgroup directory is created in the cgroup filesystem hierarchy \(usually mounted on /sys/fs/cgroup\).

The _cgroup\_clone\_children\_read_ message is sent during startup, when existing cgroups are enumerated.

The _cgroup\_attach\_task_ message is sent when a process enters a cgroup.

The _kill\_css_ message is sent when a cgroup is destroyed.

All messages identify the cgroup through the `u64 cgroup` field. A cgroup's parent is identified through the `u64 cgroup_parent` field \(cgroups are hierarchical\).

