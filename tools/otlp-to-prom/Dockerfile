# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

FROM docker.io/bitnami/minideb:bookworm@sha256:9fdb57309b6d56b7baafc12be811d9c03be342f6f0bfb2fb92e33d7470e9897c

RUN install_packages libgrpc++1.51

COPY srv /srv

ENTRYPOINT ["/srv/otlp-to-prom"]
