# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

FROM docker.io/bitnami/minideb:trixie@sha256:6318eb83decf39fcb82188eaa09ba094a2a713ce5f37e5ac4e2178f6326c5c8e

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
