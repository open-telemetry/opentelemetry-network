# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# Note that this docker image requires a base image and utilities that work for all target distributions and kernel versions.
# RHEL/Centos/similar targets
#   yum, yum-utils are currently needed for installing kernel headers
#     they are available in debian:buster but not debian:bullseye
#     they are available in ubuntu:bionic but not ubuntu:focal or ubuntu:jammy
# Debian/Ubuntu/similar targets
#   apt/dpkg support for zstd compression is needed for installing kernel headers with Ubuntu 21.10 and later
#     debian's dpkg does not yet support zstd compression - see https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=892664
# For now use ubuntu:bionic.  In order to use a more recent base image, kernel_headers.sh
# needs to be refactored to use dnf instead of yum, which is deprecated and its package no longer available.

FROM ubuntu:bionic

LABEL org.label-schema.name="opentelemetry-ebpf-kernel-collector"
LABEL org.label-schema.description="OpenTelemetry eBPF kernel information collector"
LABEL org.label-schema.vcs-url="https://github.com/open-telemetry/opentelemetry-ebpf"
LABEL org.label-schema.schema-version="1.0"

# ca-certificates are required by libcurl
RUN apt-get update && apt-get install -y ca-certificates
ENV SSL_CERT_DIR=/etc/ssl/certs

ENV EBPF_NET_INSTALL_DIR=/srv
ENV EBPF_NET_HOST_DIR=/hostfs
ENV EBPF_NET_DATA_DIR=/var/run/ebpf_net

ENTRYPOINT [ "/srv/entrypoint.sh" ]

# required by kernel_headers.sh script
RUN apt-get install -y coreutils curl sed tar yum yum-utils

ARG BUILD_TYPE
RUN if [ "$BUILD_TYPE" = "Debug" ]; then \
      apt-get install -y bc cgdb gawk gdb gzip iputils-ping jq netcat-openbsd procps ripgrep vim valgrind; \
    fi

COPY srv /srv
WORKDIR /srv
RUN if [ ! -e /srv/kernel-collector ]; then \
      ln /srv/kernel-collector-stripped /srv/kernel-collector; \
    fi
