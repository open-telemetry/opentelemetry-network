FROM bitnami/minideb:buster

LABEL org.label-schema.name="flowmill/k8s-relay" \
      org.label-schema.description="Relays Kubernetes Metadata from k8s-collector to Flowmill" \
      org.label-schema.schema-version="1.0"

# ca-certificates are required by libcurl
RUN apt-get update && apt-get install -y ca-certificates
ENV SSL_CERT_DIR=/etc/ssl/certs

ENTRYPOINT [ "/srv/entrypoint.sh" ]

COPY srv /srv
WORKDIR /srv
RUN if [ ! -e /srv/k8s-relay ]; then \
      ln /srv/k8s-relay-stripped /srv/k8s-relay; \
    fi
