# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# Tried alpine (nope,  binary incompatible with glibc) & distroless (nope, missing some libs, probably fixable with some effort)
FROM docker-hub.repo.splunkdev.net/bitnami/minideb:unstable

LABEL \
  org.label-schema.name="opentelemetry/opentelemetry-ebpf-reducer" \
  org.label-schema.description="OpenTelemetry eBPF reducer" \
  org.label-schema.vcs-url="https://github.com/open-telemetry/opentelemetry-ebpf" \
  org.label-schema.usage="./README.md" \
  org.label-schema.schema-version="1.0"

RUN install_packages gdb cgdb bc sed coreutils netcat-openbsd jq gawk ripgrep vim

# Inbound metrics on 8000 (currently via stunnel); prometheus scrapes 7000, 7001, 7010
EXPOSE 7000 7001 7010 8000

ENV EBPF_NET_INSTALL_DIR=/srv
ENV EBPF_NET_DATA_DIR=/var/run/ebpf_net

ENTRYPOINT ["/srv/entrypoint.sh"]
CMD ["--port", "8000", "--prom", "0.0.0.0:7010"]

COPY srv /srv
RUN mv /srv/reducer-static /srv/opentelemetry-npm-reducer
