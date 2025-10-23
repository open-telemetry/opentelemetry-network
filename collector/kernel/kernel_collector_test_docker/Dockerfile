# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

FROM docker.io/bitnami/minideb:bookworm@sha256:8b9a08ffe9f3d1b89d06f57c668f97c96ccb0dab9e5aaf9a411f94f0d70d286a

# ca-certificates are required by libcurl
RUN install_packages ca-certificates libcurlpp0 libllvm16 libabsl20220623 libelf1
ENV SSL_CERT_DIR=/etc/ssl/certs

ENV EBPF_NET_INSTALL_DIR=/srv
ENV EBPF_NET_HOST_DIR=/hostfs
ENV EBPF_NET_DATA_DIR=/var/run/ebpf_net

ENTRYPOINT [ "/srv/entrypoint-kct.sh" ]

RUN install_packages stress-ng

RUN install_packages bc cgdb gawk gdb gzip iputils-ping jq netcat-openbsd procps python3 ripgrep vim valgrind curl

COPY srv /srv
WORKDIR /srv
