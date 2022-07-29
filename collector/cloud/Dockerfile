# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

FROM bitnami/minideb:buster

LABEL org.label-schema.name="flowmill/cloud-collector" \
      org.label-schema.description="Flowmill Cloud Metadata Collector" \
      org.label-schema.schema-version="1.0"

# ca-certificates are required by libcurl
RUN apt-get update && apt-get install -y ca-certificates
ENV SSL_CERT_DIR=/etc/ssl/certs

ENTRYPOINT [ "/srv/entrypoint.sh" ]

COPY srv /srv
WORKDIR /srv
RUN if [ ! -e /srv/cloud-collector ]; then \
      ln /srv/cloud-collector-stripped /srv/cloud-collector; \
    fi
