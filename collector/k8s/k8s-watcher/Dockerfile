FROM bitnami/minideb:buster

COPY srv /srv

WORKDIR /srv
ENTRYPOINT ["/srv/k8s-watcher"]

LABEL org.label-schema.name="flowmill/k8s-watcher" \
      org.label-schema.description="Flowmill Kubernetes Metadata Collector" \
      org.label-schema.schema-version="1.0"
