#!/bin/bash -xe

export EBPF_NET_SRC="${EBPF_NET_SRC:-$(git rev-parse --show-toplevel)}"

vagrant up
vagrant ssh -- -R 5000:localhost:5000 -L 59090:localhost:9090
