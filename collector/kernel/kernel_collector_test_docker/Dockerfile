# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

FROM docker.io/bitnami/minideb:trixie@sha256:0766a3b76750541ae2c5f3b806e5201e1b2ca1549a0a036a057726dc9e5dfa4d

# ca-certificates are required by libcurl
RUN install_packages ca-certificates libcurlpp0 libabsl20240722 libelf1
ENV SSL_CERT_DIR=/etc/ssl/certs

ENV EBPF_NET_INSTALL_DIR=/srv
ENV EBPF_NET_HOST_DIR=/hostfs
ENV EBPF_NET_DATA_DIR=/var/run/ebpf_net

ENTRYPOINT [ "/srv/entrypoint-kct.sh" ]

RUN install_packages stress-ng

RUN install_packages bc cgdb gawk gdb gzip iputils-ping jq netcat-openbsd procps python3 ripgrep vim valgrind curl

COPY srv /srv
WORKDIR /srv
