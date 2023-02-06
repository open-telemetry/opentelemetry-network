#!/bin/bash -e
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


echo "Running OpenTelemetry collector server"
echo

function print_help {
  echo "usage: $0 [--contrib|--help|--host|--otel|--prom|--splunk]"
  echo
  echo "  default is to run the OpenTelemetry Contrib Collector"
  echo "  --contrib: run the OpenTelemetry Contrib Collector"
  echo "  --help: display this help message and the container's help message"
  echo "  --host: run with --network=host"
  echo "  --otel: run the OpenTelemetry Collector"
  echo "  --prom: run use Prometheus receiver to scrape metrics (default is OTLP gRPC receiver)"
  echo "  --splunk: run the Splunk distribution of OpenTelemetry Collector"
}

EBPF_NET_SRC_ROOT="${EBPF_NET_SRC_ROOT:-$HOME/src}"

# Default to opentelemetry-collector-contrib
otelcol_to_use="otelcol-contrib"

while [[ "$#" -gt 0 ]]; do
  arg="$1"; shift
  case "${arg}" in
    --contrib)
      otelcol_to_use="otelcol-contrib"
      ;;

    --host)
      use_network_host="true"
      ;;

    --help)
      print_help
      exit 0
      ;;

    --otel)
      otelcol_to_use="otelcol"
      ;;

    --prom)
      use_prom_receiver="true"
      use_network_host="true"
      ;;

    --splunk)
      otelcol_to_use="splunk-otelcol"
      ;;

    *)
      print_help
      exit 0
      ;;
  esac
done

# Defaults
OTEL_COLLECTOR_CONFIG_FILE="${EBPF_NET_SRC_ROOT}/dev/devbox/source/otelcol-config.yaml"
OTEL_COLLECTOR_CONFIG_FILE_INTERNAL="/etc/otelcol/config.yaml"
OTEL_COLLECTOR_CONTAINER_NAME="otelcol"
OTEL_COLLECTOR_ENV_VARS=""
OTEL_COLLECTOR_LOG_FILE="${PWD}/otel.log"
OTEL_COLLECTOR_PORTS="-p 4317:4317 -p 4318:4318 -p 8888:8888 -p 13133:13133"

if [[ ${use_prom_receiver} == "true" ]]
then
  sed -i "s/receivers: \[otlp\]/receivers: \[otlp, prometheus\]/" ${OTEL_COLLECTOR_CONFIG_FILE}
else
  sed -i "s/receivers: \[otlp, prometheus\]/receivers: \[otlp\]/" ${OTEL_COLLECTOR_CONFIG_FILE}
fi

if [[ ${use_network_host} == "true" ]]
then
  OTEL_COLLECTOR_PORTS="--network=host"
fi

case "${otelcol_to_use}" in
  otelcol)
    # https://hub.docker.com/r/otel/opentelemetry-collector/tags
    OTEL_COLLECTOR_IMAGE="otel/opentelemetry-collector:latest"
    ;;
  otelcol-contrib)
    # https://hub.docker.com/r/otel/opentelemetry-collector-contrib/tags
    OTEL_COLLECTOR_IMAGE="otel/opentelemetry-collector-contrib:latest"
    OTEL_COLLECTOR_CONFIG_FILE_INTERNAL="/etc/otelcol-contrib/config.yaml"
    ;;
  splunk-otelcol)
    # https://github.com/signalfx/splunk-otel-collector/blob/main/docs/getting-started/linux-manual.md
    # https://quay.io/repository/signalfx/splunk-otel-collector?tab=tags
    OTEL_COLLECTOR_IMAGE="quay.io/signalfx/splunk-otel-collector:latest"
    OTEL_COLLECTOR_ENV_VARS="-e SPLUNK_ACCESS_TOKEN=YOUR_TOKEN_HERE -e SPLUNK_REALM=YOUR_REALM_HERE -e SPLUNK_CONFIG=${OTEL_COLLECTOR_CONFIG_FILE_INTERNAL}"
    ;;
esac

echo "OTEL_COLLECTOR_IMAGE ${OTEL_COLLECTOR_IMAGE}"
echo "OTEL_COLLECTOR_CONFIG_FILE ${OTEL_COLLECTOR_CONFIG_FILE}"
echo "OTEL_COLLECTOR_CONFIG_FILE_INTERNAL ${OTEL_COLLECTOR_CONFIG_FILE_INTERNAL}"
echo "OTEL_COLLECTOR_ENV_VARS ${OTEL_COLLECTOR_ENV_VARS}"
echo "OTEL_COLLECTOR_PORTS ${OTEL_COLLECTOR_PORTS}"
echo "OTEL_COLLECTOR_LOG_FILE ${OTEL_COLLECTOR_LOG_FILE}"

set -x

touch "${OTEL_COLLECTOR_LOG_FILE}"
chmod 666 "${OTEL_COLLECTOR_LOG_FILE}"

docker run \
  --rm \
  ${OTEL_COLLECTOR_ENV_VARS} \
  ${OTEL_COLLECTOR_PORTS} \
  -v "${OTEL_COLLECTOR_CONFIG_FILE}:${OTEL_COLLECTOR_CONFIG_FILE_INTERNAL}" \
  -v "${OTEL_COLLECTOR_LOG_FILE}:/var/log/otel.log" \
  --name ${OTEL_COLLECTOR_CONTAINER_NAME} \
  ${OTEL_COLLECTOR_IMAGE}
