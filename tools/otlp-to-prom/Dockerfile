# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

FROM docker.io/bitnami/minideb:trixie@sha256:0766a3b76750541ae2c5f3b806e5201e1b2ca1549a0a036a057726dc9e5dfa4d

RUN install_packages libgrpc++1.51

COPY srv /srv

ENTRYPOINT ["/srv/otlp-to-prom"]
