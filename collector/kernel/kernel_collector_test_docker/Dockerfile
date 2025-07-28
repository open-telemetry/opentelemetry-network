# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

FROM docker.io/bitnami/minideb:bookworm@sha256:53344e930094d0e57779aa0938a0efc0da5aab3bc5eb5b0aac859570b82c2da8

# ca-certificates are required by libcurl
RUN install_packages ca-certificates libcurlpp0 libllvm16 libabsl20220623 libelf1
ENV SSL_CERT_DIR=/etc/ssl/certs

ENV EBPF_NET_INSTALL_DIR=/srv
ENV EBPF_NET_HOST_DIR=/hostfs
ENV EBPF_NET_DATA_DIR=/var/run/ebpf_net

ENTRYPOINT [ "/srv/entrypoint-kct.sh" ]

RUN install_packages stress-ng

RUN install_packages bc cgdb gawk gdb gzip iputils-ping jq netcat-openbsd procps python3 ripgrep vim valgrind

COPY srv /srv
WORKDIR /srv
