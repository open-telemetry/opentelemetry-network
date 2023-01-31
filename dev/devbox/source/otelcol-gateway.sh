#!/bin/bash -e
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


echo "Running OpenTelemetry collector server"
echo "To connect from Flowmill kernel collector agent: kernel-collector.sh --otel"
echo

function print_help {
  echo "usage: $0 [--contrib|--help|--host|--splunk]"
  echo
  echo "  default is to run the Splunk OpenTelemetry Collector"
  echo "  --contrib: run the OpenTelemetry Contrib Collector"
  echo "  --help: display this help message and the container's help message"
  echo "  --host: run with --network=host"
  echo "  --otel: run the OpenTelemetry Collector"
  echo "  --prom-only: run the OpenTelemetry Collector configured to scrape prometheus metrics"
}

EBPF_NET_SRC="${EBPF_NET_SRC:-$HOME/src}"

OTEL_COLLECTOR_CONFIG_FILE_INTERNAL="/etc/otelcol/config.yaml"
OTEL_COLLECTOR_ENV_VARS=""
OTEL_COLLECTOR_LOG_FILE="${PWD}/otel.log"
OTEL_COLLECTOR_CONTAINER_NAME="otelcol"

# Default to splunk-otelcol
#https://github.com/signalfx/splunk-otel-collector/blob/main/docs/getting-started/linux-manual.md
#
# File exporter rotation was introduced in otelcol version 0.60.0, inadvertantly causing the otelcol to fail when file rotation
# is not specified.  Workarounds include 1) running a pre-0.60.0 version and 2) specifying file rotation settings, for example
#exporters:
#  file:
#    path: /var/log/otel.log
#    rotation:
#      max_megabytes: 100000
#      max_days: 3
#      max_backups: 3
#      localtime: true
#
# There is a fix out for properly handling when file exporter rotation is not specified that is still not part of the splunk-otelcol as of 0.62.0.
# https://github.com/open-telemetry/opentelemetry-collector-contrib/pull/14705
# Until it is, specifically use 0.59.0.
#
#OTEL_COLLECTOR_IMAGE="quay.io/signalfx/splunk-otel-collector:latest"
OTEL_COLLECTOR_IMAGE="quay.io/signalfx/splunk-otel-collector:0.59.0"
OTEL_COLLECTOR_CONFIG_FILE="${EBPF_NET_SRC}/dev/devbox/source/splunk-otelcol-config.yaml"
OTEL_COLLECTOR_ENV_VARS="-e SPLUNK_ACCESS_TOKEN=12345 -e SPLUNK_REALM=us0 -e SPLUNK_CONFIG=${OTEL_COLLECTOR_CONFIG_FILE_INTERNAL}"
OTEL_COLLECTOR_PORTS="-p 4317:4317 -p 4318:4318 -p 8888:8888 -p 13133:13133"

while [[ "$#" -gt 0 ]]; do
  arg="$1"; shift
  case "${arg}" in
    --contrib)
      # note latest is not actually latest, so specify version tag
      # see https://hub.docker.com/r/otel/opentelemetry-collector-contrib/tags
      OTEL_COLLECTOR_IMAGE="otel/opentelemetry-collector-contrib:0.59.0"
      OTEL_COLLECTOR_CONFIG_FILE_INTERNAL="/etc/otelcol-contrib/config.yaml"
      ;;

    --host)
      use_network_host="true"
      ;;

    --help)
      print_help
      exit 0
      ;;

    --otel)
      # note latest is not actually latest, so specify version tag
      # see https://hub.docker.com/r/otel/opentelemetry-collector/tags
      OTEL_COLLECTOR_IMAGE="otel/opentelemetry-collector:0.59.0"
      OTEL_COLLECTOR_CONFIG_FILE="${EBPF_NET_SRC}/dev/devbox/source/otelcol-config.yaml"
      OTEL_COLLECTOR_PORTS="-p 4317:4317 -p 4318:4318 -p 8888:8888"
      ;;

    --prom-only)
      prom_only="true"
      use_network_host="true"
      ;;

    *)
      print_help
      exit 0
      ;;
  esac
done

if [[ ${prom_only} == "true" ]]
then
  OTEL_COLLECTOR_CONFIG_FILE="${EBPF_NET_SRC}/dev/devbox/source/splunk-otelcol-prom-only-config.yaml"
  OTEL_COLLECTOR_LOG_FILE="${PWD}/otel-prom.log"
  OTEL_COLLECTOR_CONTAINER_NAME="otelcol-prom"
fi

if [[ ${use_network_host} == "true" ]]
then
  OTEL_COLLECTOR_PORTS="--network=host"
fi

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
