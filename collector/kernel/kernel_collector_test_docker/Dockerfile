# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

FROM bitnami/minideb:bullseye@sha256:2ba2f05605e4bf439917d9e9252e72afdaa26c543bd425998643f900e898b9d1

# ca-certificates are required by libcurl
RUN install_packages ca-certificates
ENV SSL_CERT_DIR=/etc/ssl/certs

ENV EBPF_NET_INSTALL_DIR=/srv
ENV EBPF_NET_HOST_DIR=/hostfs
ENV EBPF_NET_DATA_DIR=/var/run/ebpf_net

ENTRYPOINT [ "/srv/entrypoint-kct.sh" ]

# required by kernel_headers.sh script
RUN install_packages coreutils curl sed tar dnf rpm stress-ng

RUN install_packages bc cgdb gawk gdb gzip iputils-ping jq netcat-openbsd procps python3 ripgrep vim valgrind

COPY srv /srv
WORKDIR /srv
