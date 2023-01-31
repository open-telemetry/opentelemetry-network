#!/bin/sh
export EBPF_NET_SRC="${EBPF_NET_SRC:-$(git rev-parse --show-toplevel)}"
"${EBPF_NET_SRC}/dev/devbox/build.sh" --base_box ubuntu/jammy64 --box_name ubuntu-jammy
