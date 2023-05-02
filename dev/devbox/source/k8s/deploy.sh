#!/bin/bash -e
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

function print_help {
  echo "usage: $0 [--demo|--dev1|--dev2|--dev3|--dev4|"
  echo "                    --ebpf-net|--ebpf-net-debug|--ebpf-net-local-registry|--ebpf-net-local-helm-chart|"
  echo "                    --ebpf-net-trace|--ebpf-net-use-otel-demo-otelcol"
  echo "                    --help|--logging-exporter|--otel-demo|"
  echo "                    --splunk-realm <REALM>|--splunk-token <TOKEN>]"
  echo "  --demo: deploy the Google Online Boutique Microservices Demo"
  echo "          see https://github.com/GoogleCloudPlatform/microservices-demo"
  echo "  --dev1: --ebpf-net --ebpf-net-debug --logging-exporter"
  echo "  --dev2: --demo --ebpf-net --ebpf-net-debug --logging-exporter"
  echo "  --dev3: --ebpf-net --ebpf-net-debug --logging-exporter --otel-demo"
  echo "  --dev4: --ebpf-net --ebpf-net-debug --ebpf-net-use-otel-demo-otelcol --otel-demo"
  echo "  --ebpf-net: deploy OpenTelementry eBPF"
  echo "  --ebpf-net-debug | -d: enable debug logging for OpenTelemetry eBPF"
  echo "  --ebpf-net-local-registry | -l: use local docker registry to deploy OpenTelementry eBPF"
  echo "                                  (default is to use public quay.io/signalfx docker registry)"
  echo "  --ebpf-net-local-helm-chart | -C: use local helm chart to deploy OpenTelementry eBPF"
  echo "                                    (default is to use public splunk-otel-collector-chart/splunk-otel-collector)"
  echo "  --ebpf-net-trace | -t: enable trace logging for OpenTelemetry eBPF"
  echo "  --ebpf-net-use-otel-demo-otelcol: deploy OpenTelemetry eBPF using the OpenTelemetry Collector deployed with the otel-demo"
  echo "  --help: display this help message and the container's help message"
  echo "  --logging-exporter | -L: enable logging exporter in OpenTelemetry Collector"
  echo "  --otel-demo: deploy the OpenTelemetry Astronomy Shop Microservices Demo"
  echo "               see https://github.com/open-telemetry/opentelemetry-demo"
  echo "  --splunk-realm: the realm to use when sending telemetry to Splunk Observability"
  echo "  --splunk-token: the access token to use when sending telemetry to Splunk Observability"
}

splunk_realm="REALM"
splunk_token="TOKEN"

while [[ "$#" -gt 0 ]]; do
  arg="$1"; shift
  case "${arg}" in
    --demo)
      deploy_microservices_demo="true"
      ;;

    --dev1)
      deploy_ebpf_net="true"
      ebpf_net_log_level="--set=networkExplorer.log.level=debug"
      use_logging_exporter="true"
      ;;

    --dev2)
      deploy_ebpf_net="true"
      ebpf_net_log_level="--set=networkExplorer.log.level=debug"
      use_logging_exporter="true"
      deploy_microservices_demo="true"
      ;;

    --dev3)
      deploy_ebpf_net="true"
      ebpf_net_log_level="--set=networkExplorer.log.level=debug"
      use_logging_exporter="true"
      deploy_otel_demo="true"
      ;;

    --dev4)
      deploy_ebpf_net="true"
      ebpf_net_log_level="--set=networkExplorer.log.level=debug"
      ebpf_net_use_otel_demo_otelcol="true"
      deploy_otel_demo="true"
      ;;

    --ebpf-net)
      deploy_ebpf_net="true"
      ;;

    --ebpf-net-debug | -d)
      ebpf_net_log_level="--set=networkExplorer.log.level=debug"
      ;;

    --ebpf-net-local-registry | -l)
      ebpf_net_use_local_registry="true"
      ;;

    --ebpf-net-local-helm-chart | -C)
      ebpf_net_use_local_helm_chart="true"
      ;;

    --logging-exporter | -L)
      use_logging_exporter="true"
      ;;

    --ebpf-net-trace | -t)
      ebpf_net_log_level="--set=networkExplorer.log.level=trace"
      ;;

    --ebpf-net-use-otel-demo-otelcol)
      ebpf_net_use_otel_demo_otelcol="true"
      ;;

    --help)
      print_help
      exit 0
      ;;

    --otel-demo)
      deploy_otel_demo="true"
      ;;

    --splunk-realm)
      splunk_realm="$1"
      shift
      ;;

    --splunk-token)
      splunk_token="$1"
      shift
      ;;

    *)
      print_help
      exit 0
      ;;
  esac
done

splunk_args="--set=splunkObservability.realm=${splunk_realm} --set=splunkObservability.accessToken=${splunk_token}"

if [[ "${deploy_microservices_demo}" == "" && "${deploy_otel_demo}" == "" && "${deploy_ebpf_net}" == "" ]]
then
  echo -e "Need to specify what to deploy.\n"
  print_help
  exit 1
fi

set -x

num_pods_before_deploy=$(microk8s kubectl get pods -A | grep -v "^NAME" | wc -l)

if [[ "${deploy_microservices_demo}" == "true" ]]
then
  microk8s kubectl create ns demo-ns || true
  microk8s kubectl config set-context --current --namespace demo-ns

  microk8s kubectl apply -f $HOME/microservices-demo/release/kubernetes-manifests.yaml
fi

if [[ "${deploy_otel_demo}" == "true" ]]
then
  microk8s kubectl create ns otel-demo-ns || true
  microk8s kubectl config set-context --current --namespace otel-demo-ns

  microk8s helm repo add open-telemetry https://open-telemetry.github.io/opentelemetry-helm-charts
  microk8s helm repo update

  otel_demo_yaml="-f otel-demo.yaml"

  if [[ "${use_logging_exporter}" == "true" ]]
  then
    otel_demo_logging_exporter="--set=opentelemetry-collector.config.exporters.logging.verbosity=detailed"
  fi

  microk8s helm install otel-demo \
    ${otel_demo_yaml} \
    ${otel_demo_logging_exporter} \
    open-telemetry/opentelemetry-demo
fi

if [[ "${deploy_ebpf_net}" == "true" ]]
then
  if [[ "${ebpf_net_use_otel_demo_otelcol}" == "true" ]]
  then
    microk8s kubectl create ns otel-demo-ns || true
    microk8s kubectl config set-context --current --namespace otel-demo-ns
  else
    microk8s kubectl create ns ebpf-net-ns || true
    microk8s kubectl config set-context --current --namespace ebpf-net-ns
  fi

  if [[ "${ebpf_net_use_local_helm_chart}" == "true" ]]
  then
    chart="$HOME/splunk-otel-collector-chart/helm-charts/splunk-otel-collector"
  else
    microk8s helm repo add splunk-otel-collector-chart https://signalfx.github.io/splunk-otel-collector-chart
    microk8s helm repo update
    chart="splunk-otel-collector-chart/splunk-otel-collector"
  fi

  ebpf_net_yaml="-f ebpf-net.yaml"

  if [[ "${ebpf_net_use_local_registry}" == "true" ]]
  then
    ebpf_net_yaml="${ebpf_net_yaml} -f ebpf-net-local-registry.yaml"
  fi

  if [[ "${use_logging_exporter}" == "true" ]]
  then
    ebpf_net_yaml="${ebpf_net_yaml} -f ebpf-net-logging-exporter.yaml"
  fi

  if [[ "${ebpf_net_use_otel_demo_otelcol}" == "true" ]]
  then
    ebpf_net_yaml="${ebpf_net_yaml} -f ebpf-net-use-otel-demo-otelcol.yaml"
  fi

  microk8s helm install ebpf-net \
    ${ebpf_net_yaml} \
    ${ebpf_net_log_level} \
    ${splunk_args} \
    ${chart}
fi

microk8s helm list -A

set +x
echo
echo -e "\n---------- Waiting for all pods to start ----------"
echo "From another window you can:"
echo "watch -n 5 microk8s kubectl get pods -A"
remaining_attempts=240
while true
do
  num_pods=$(microk8s kubectl get pods -A | grep -v "^NAME" | wc -l)
  num_not_running=$(microk8s kubectl get pods -A | egrep -v "^NAME|Running" | wc -l)
  if [ ${num_pods} -gt ${num_pods_before_deploy} ] && [ ${num_not_running} -eq 0 ]
  then
    break
  fi

  remaining_attempts=$((${remaining_attempts}-1))
  if [ ${remaining_attempts} -eq 0 ]
  then
    echo
    microk8s kubectl get pods -A
    echo -e "\nERROR: Pods did not all not start within the time expected."
    exit 1
  fi

  echo -n "."
  sleep 5
done
echo

echo -e "\n---------- All pods are Running ----------"
microk8s kubectl get pods -A

# Note: if deploying both at the same time, ports will only be exposed for otel-demo
if [[ "$deploy_otel_demo" == "true" ]]
then
  echo -e "\n---------- Exposing port 8080 for OpenTelemetry Astronomy Shop Microservices Demo ----------"
  echo "---------- Exposing Prometheus port 9090 for OpenTelemetry Astronomy Shop Microservices Demo ----------"
  set -x
  microk8s kubectl port-forward -n otel-demo-ns svc/otel-demo-frontendproxy 8080:8080 2>&1 > /dev/null &
  microk8s kubectl port-forward -n otel-demo-ns svc/otel-demo-prometheus-server 9090:9090 2>&1 > /dev/null &
  microk8s kubectl port-forward -n otel-demo-ns svc/otel-demo-otelcol 4318:4318 2>&1 > /dev/null &
elif [[ "$deploy_microservices_demo" == "true" ]]
then
  echo -e "\n---------- Exposing port 8080 for Google Online Boutique Microservices Demo ----------"
  pod=$(microk8s kubectl get pods -n demo-ns | grep "frontend" | awk '{print $1}')
  set -x
  microk8s kubectl port-forward -n demo-ns ${pod} 8080:8080 2>&1 > /dev/null &
fi
