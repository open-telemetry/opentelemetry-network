# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

FROM bitnami/minideb:bullseye@sha256:1a991d4665ebeb68c16a35224d66e77d6daf2c503f8c72572e9890e6e511f193

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
