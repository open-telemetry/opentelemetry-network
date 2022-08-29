# Running multiple agents

To avoid kprobe leaks, kernel collectors by default set each kprobe event name to the same value prepending `ebpf_net_` to it \(see `/sys/kernel/debug/tracing/kprobe_events`\). While letting agents reuse the name after the crash, such choice also prevents running multiple agents on the same host.

To be able to run multiple agents on the same machine, one can set the `FLOWMILL_AGENT_PROBE_SUFFIX` environment variable to an arbitrary string. This will append the string to the kprobe event name making it unique among the agents. For example, one can set `FLOWMILL_AGENT_PROBE_SUFFIX=agent1` and `FLOWMILL_AGENT_PROBE_SUFFIX=agent2` environment variables for the two agents to prevent the conflict. In case of an agent crash we may still leak the probes. However, the impact is limited only to this small set \(as opposed to appending process PID, where the number of leaked probes can be arbitrarily large\).

