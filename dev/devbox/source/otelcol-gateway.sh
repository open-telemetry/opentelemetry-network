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
  echo "  --prom: use Prometheus receiver to scrape metrics (default is OTLP gRPC receiver)"
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
otel_collector_config_file="${EBPF_NET_SRC_ROOT}/dev/devbox/source/otelcol-config.yaml"
otel_collector_config_file_internal="/etc/otelcol/config.yaml"
otel_collector_container_name="otelcol"
otel_collector_env_vars=""
otel_collector_log_file="${PWD}/otel.log"
otel_collector_ports="-p 4317:4317 -p 4318:4318 -p 8888:8888 -p 13133:13133"

if [[ ${use_prom_receiver} == "true" ]]
then
  sed -i "s/receivers: \[otlp\]/receivers: \[otlp, prometheus\]/" ${otel_collector_config_file}
else
  sed -i "s/receivers: \[otlp, prometheus\]/receivers: \[otlp\]/" ${otel_collector_config_file}
fi

if [[ ${use_network_host} == "true" ]]
then
  otel_collector_ports="--network=host"
fi

case "${otelcol_to_use}" in
  otelcol)
    # https://hub.docker.com/r/otel/opentelemetry-collector/tags
    otel_collector_image="otel/opentelemetry-collector:latest"
    ;;
  otelcol-contrib)
    # https://hub.docker.com/r/otel/opentelemetry-collector-contrib/tags
    otel_collector_image="otel/opentelemetry-collector-contrib:latest"
    otel_collector_config_file_internal="/etc/otelcol-contrib/config.yaml"
    ;;
  splunk-otelcol)
    # https://github.com/signalfx/splunk-otel-collector/blob/main/docs/getting-started/linux-manual.md
    # https://quay.io/repository/signalfx/splunk-otel-collector?tab=tags
    otel_collector_image="quay.io/signalfx/splunk-otel-collector:latest"
    otel_collector_env_vars="-e SPLUNK_ACCESS_TOKEN=YOUR_TOKEN_HERE -e SPLUNK_REALM=YOUR_REALM_HERE -e SPLUNK_CONFIG=${otel_collector_config_file_internal}"
    ;;
esac

echo "otel_collector_image ${otel_collector_image}"
echo "otel_collector_config_file ${otel_collector_config_file}"
echo "otel_collector_config_file_internal ${otel_collector_config_file_internal}"
echo "otel_collector_env_vars ${otel_collector_env_vars}"
echo "otel_collector_ports ${otel_collector_ports}"
echo "otel_collector_log_file ${otel_collector_log_file}"

set -x

touch "${otel_collector_log_file}"
chmod 666 "${otel_collector_log_file}"

docker run \
  --rm \
  ${otel_collector_env_vars} \
  ${otel_collector_ports} \
  -v "${otel_collector_config_file}:${otel_collector_config_file_internal}" \
  -v "${otel_collector_log_file}:/var/log/otel.log" \
  --name ${otel_collector_container_name} \
  ${otel_collector_image}
