FROM segment/chamber:2 AS chamber

FROM bitnami/minideb:buster

LABEL org.label-schema.name="flowmill/aws-collector" \
      org.label-schema.description="Flowmill AWS Metadata Collector" \
      org.label-schema.schema-version="1.0"

# ca-certificates are required by libcurl
RUN apt-get update && apt-get install -y ca-certificates
ENV SSL_CERT_DIR=/etc/ssl/certs

ENTRYPOINT [ "/srv/entrypoint.sh" ]

COPY --from=chamber /chamber /bin/chamber
COPY srv /srv
WORKDIR /srv
RUN if [ ! -e /srv/aws-collector ]; then \
      ln /srv/aws-collector-stripped /srv/aws-collector; \
    fi
