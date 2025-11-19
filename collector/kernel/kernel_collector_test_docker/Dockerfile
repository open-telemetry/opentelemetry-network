# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

FROM docker.io/bitnami/minideb:trixie@sha256:bde17110f4074ea9ef0552e751be2d49111dd529eaea72bccd7490a11e042d56

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
