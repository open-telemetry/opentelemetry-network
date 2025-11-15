FROM docker.io/bitnami/minideb:trixie@sha256:0766a3b76750541ae2c5f3b806e5201e1b2ca1549a0a036a057726dc9e5dfa4d

LABEL org.label-schema.name="opentelemetry-network-k8s-collector"
LABEL org.label-schema.description="OpenTelemetry Network Kubernetes metadata collector"
LABEL org.label-schema.vcs-url="https://github.com/open-telemetry/opentelemetry-network"
LABEL org.label-schema.schema-version="1.0"

COPY srv /srv
WORKDIR /srv

# Transitional compatibility: allow overriding the watcher container image
# by exposing the expected binary name used by the chart.
RUN if [ ! -e /srv/k8s-collector ]; then \
      ln -s /srv/k8s-collector-stripped /srv/k8s-collector; \
    fi \
 && ln -sf /srv/k8s-collector /srv/k8s-watcher

ENTRYPOINT ["/srv/k8s-collector"]
