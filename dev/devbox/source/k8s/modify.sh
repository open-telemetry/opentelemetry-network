#!/bin/bash -e
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

function print_help {
  echo "usage: $0 [--ebpf-net|--otel-demo] <YAML_FILE>"
  echo "  --ebpf-net: modify previously deployed OpenTelementry eBPF component(s)"
  echo "  --ebpf-net-local-helm-chart | -C: use local helm chart to modify OpenTelementry eBPF component(s)"
  echo "                                    (default is to use public splunk-otel-collector-chart/splunk-otel-collector)"
  echo "  --otel-demo: modify previously deployed OpenTelemetry Astronomy Shop Microservices Demo component(s)"
}

while [[ "$#" -gt 0 ]]; do
  arg="$1"; shift
  case "${arg}" in
    --ebpf-net)
      modify_ebpf_net="true"
      ;;

    --ebpf-net-local-helm-chart | -C)
      ebpf_net_use_local_helm_chart="true"
      ;;

    --otel-demo)
      modify_otel_demo="true"
      ;;

    *)
      if [ -n "${yaml_file}" ]
      then
        echo -e "\nOnly one yaml file should be specified.\n"
        print_help
        exit 1
      fi
      yaml_file=${arg}
      if [ ! -f "${yaml_file}" ]
      then
        echo -e "\nYAML_FILE ${yaml_file} does not exist.\n"
        print_help
        exit 1
      fi
      ;;
  esac
done

if [[ -z "${modify_ebpf_net}" && -z "${modify_otel_demo}" ]]
then
  echo -e "\nNeed to specify what to modify.\n"
  print_help
  exit 1
fi

if [[ -z "${yaml_file}" ]]
then
  echo -e "\nYAML_FILE not specified.\n"
  print_help
  exit 1
fi

if [[ "${modify_ebpf_net}" == "true" ]]
then
  if [[ "${ebpf_net_use_local_helm_chart}" == "true" ]]
  then
    chart="$HOME/splunk-otel-collector-chart/helm-charts/splunk-otel-collector"
  else
    chart="splunk-otel-collector-chart/splunk-otel-collector"
  fi

  namespace="ebpf-net-ns"
  release="ebpf-net"
fi

if [[ "${modify_otel_demo}" == "true" ]]
then
  chart="open-telemetry/opentelemetry-demo"
  namespace="otel-demo-ns"
  release="otel-demo"
fi

diff_output=$(microk8s helm diff upgrade --namespace=${namespace} ${release} ${chart} --reuse-values -f ${yaml_file})
ret="$?"
if [ "${ret}" != "0" ]; then
  echo "Helm command failed with exit code $?"
  exit "${ret}"
elif [ -z "${diff_output}" ]; then
  echo "No helm changes to apply!"
else
  echo "${diff_output}"
  echo -n "Do you wish to deploy these changes? Type 'y' to continue, ctrl-c to quit: "
  read choice

  case "${choice}" in
    y)
      echo -e "\nDeploying changes.\n"
      microk8s helm list -A
      echo
      (set -x; microk8s helm upgrade --namespace="${namespace}" "${release}" ${chart} --reuse-values -f "${yaml_file}")
      echo
      microk8s helm list -A
      echo
      echo "To rollback these changes: 'helm rollback ${release} <PREVIOUS_VERSION>'"
      ;;

    *)
      echo -e "\nNOT deploying.\n"
      exit 0
      ;;
  esac
fi

