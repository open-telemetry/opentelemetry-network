# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

FROM bitnami/minideb:buster@sha256:837cf5cdc9fdb8c6cc144b41e3626b28384697f661862dc29a6c95e9e521e118

COPY srv /srv

ENTRYPOINT ["/srv/otlp-to-prom"]
