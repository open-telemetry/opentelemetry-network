# TCP and UDP

## TCP and UDP

### TCP

#### Messages

* new\_sock\_created
* reset\_tcp\_counters
* close\_sock\_info
* set\_state\_ipv4
* set\_state\_ipv6
* rtt\_estimator
* tcp\_syn\_timeout
* tcp\_reset
* http\_response
* tcp\_data

Messages _new\_sock\_created_ and _reset\_tcp\_counters_ are sent when a new TCP socket is created. The _close\_sock\_info_ message is sent when a TCP socket is closed.

Messages _set\_state\_ipv4_ and _set\_state\_ipv6_ are sent when a socket is assigned a IP address and port number.

The _rtti\_estimator_ message conveys socket stats, as reported by the kernel.

The _tcp\_syn\_timeout_ message is sent when a socket experiences a timeout in SYN\_RECV or SYN\_SENT state. The _tcp\_reset_ message is sent when a TCP RST was sent or received.

Messages _http\_response_ and _tcp\_data_ are sent by the TCP-processor system, and are not strictly part of the TCP socket contract.

#### TCP resets

[https://www.pico.net/kb/what-is-a-tcp-reset-rst](https://www.pico.net/kb/what-is-a-tcp-reset-rst)

When an unexpected TCP packet arrives at a host, that host usually responds by sending a reset packet back on the same connection. A reset packet is simply one with no payload and with the RST bit set in the TCP header flags.

There are a few circumstances in which a TCP packet might not be expected; the two most common are:

* The packet is an initial SYN packet trying to establish a connection to a server port on which no process is listening.
* The packet arrives on a TCP connection that was previously established, but the local application already closed its socket or exited and the OS closed the socket.

Other circumstances are possible, but are unlikely outside of malicious behavior such as attempts to hijack a TCP connection.

#### Collection

On the receive side, we attach to the `tcp_reset` kernel function which is called whenever kernel receives a packet with RST flag set. On the send side, there is no such convenient single function in the kernel which is called when sending a reset. We have to directly inspect packet data in the `skb` structure and check if `TCP_FLAG_RST` is set in the TCP segment header. This is done in `on_ip_send_skb` and `on_ip6_send_skb`.

After detecting a reset packet, we notify the userland portion of the collector relaying the socket on which the reset was sent/received along with the packet direction \(i.e., `RX` or `TX`\).

TCP reset counts are sent to the pipeline server ingest and are written to the timeseries database as a unitless counts like SYN timeouts or TCP retransmits.

### UDP

#### Messages

* udp\_new\_socket
* udp\_destroy\_socket
* udp\_stats
* dns\_packet

The _udp\_new\_socket_ message is sent when a new UDP socket is created. The _udp\_destroy\_socket_ message is sent when a UDP socket is closed.

The _udp\_stats_ message conveys UDP socket stats, as reported by the kernel.

The _dns\_packet_ message is sent when a DNS packet sent or received by an application.

## NAT

The part of the Linux kernel where NAT is implemented is called the _netfilter connection tracking system_, or _conntrack_.

We instrument a couple of kernel functions that deal with conntrack to extract NAT mappings.

### Messages

The following messages are sent from kernel-collector BPF to user-space:

* _existing\_conntrack\_tuple_
* _nf\_conntrack\_alter\_reply_
* _nf\_nat\_cleanup\_conntrack_
* _set\_state\_ipv4_

The _existing\_conntrack\_tuple_ message is sent during enumeration of existing conntracks -- those that were created before the kernel collector reached a steady state.

The _nf\_conntrack\_alter\_reply_ message is sent when there is new a translation for a socket's local or remote IP address \(or both\).

The _nf\_nat\_cleanup\_conntrack_ message is sent when a conntrack is deleted.

The three conntrack messages contain the `u64 ct` field that uniquely identifies a conntrack.

The _set\_state\_ipv4_ message assigns socket's local and remote IP address and port number.

#### Source/destination confusion

It is important to note that message fields that refer to _source_ or _destination_ IP address and port number don't refer to source and destination of a NAT mapping, but to IP socket's **local** and **remote** IP address and port number.

### Existing conntracks

Two _existing\_conntrack\_tuple_ are needed to establish a NAT mapping: one with the `u8 dir` field set to 0, and the other set to 1. The message with _dir_ set to 0 specifies the mappings source, while the message with _dir_ set to 1 specifies the mapping destination.

It is required that message with `dir == 0` precedes the corresponding message with `dir == 1`.

### New conntracts

The _nf\_conntrack\_alter\_reply_ message specifies both the mapping source and destination.

It is not required that a socket with the matching IP address and port number exist at this time. The _set\_state\_ipv4_ message can arrive before or after the _nf\_conntrack\_alter\_reply_ message_._

