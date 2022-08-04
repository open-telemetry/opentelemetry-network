# DNS and HTTP collection

## DNS collection

### DNS Collection and Enrichment

This document describes how DNS collection and enrichment works.

### Agent

#### Kernel space

Collector's eBFP code gathers DNS data by hooking into kernel UDP send functions \(for reference, see where `perf_check_and_submit_dns` function is called from\). Captured UDP send buffer data \(`skb->data`\) are relayed to the user space through perf ring messages. The maximum captured UDP data size is limited to 512 bytes \(maximum DNS UDP packet message size specified in the RFC\). We currently do not support DNS over HTTP.

#### User space

Once data is received by the user space, it is parsed using third party code in the `collector/kernel/dns` directory. If the DNS request is detected, it is stored in the `DnsRequests` cache object \(see `BufferedPoller::dns_requests_`\). If data represents a DNS response, the corresponding request is located in the cache. The DNS response time is calculated using previously stored request time stamp, and the `dns_response` message is sent to the pipeline server ingest. This message includes host name and a set of IPv4 and IPv6 associated addresses. All requests are timed out and removed from the collector cache object after 10 seconds \(see `BufferedPoller::process_dns_timeouts`\).

## HTTP collection

### HTTP Detection

There are currently two implementations of HTTP code detection:

* in kernel BPF HTTP detection
* user land detection \(userland-tcp\)

The main difference is that for user land TCP the raw data is passed to the user space and then processed. For in-kernel based version the detection is performed in kernel space by BPF code. Userland code can be enabled by passing `--enable-userland-tcp` flag to the kernel collector.

The agent collects data by attaching to `tcp_sendmsg` and `tcp_recvmsg` system calls. Data is gathered directly from packets in the `skb` kernel structure. For user land TCP the data is passed to the user space for further processing. If supported http protocol is detected, the http status code is collected. For requests the timestamp is recorded to compute request latency on response.

The collected http response code is sent to the pipeline server. It is the actual status number. Pipeline server performs subsequent aggregation into `2xx`, `4xx`, `other`, and `5xx` groups.

### Collector internal messages

The following message is used to communicate between kernel space and user space in the Kernel Collector:

```text
`http_response` {
     sk - socket id
     pid - process id (TGID) of the process
     code - the actual http code
     latency_ns - response latency
     client_server - whether the side is client or a server
}
```

### Server ingest message

The following message is used to send data to the pipeline server ingest:

```text
`http_response` {
     sk - socket id
     pid - process id (TGID) of the process
     code - the actual http code
     latency_ns - response latency
     client_server - whether the side is client or a server
}
```

