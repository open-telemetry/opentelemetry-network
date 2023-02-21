#!/bin/bash -e
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

function print_help {
  echo "usage: $0 [--demo|--ebpf-net|--ebpf-net-debug|--ebpf-net-local|--ebpf-net-trace|--ebpf-net-logging-exporter|"
  echo "                    --help|--otel-demo|--splunk-realm <REALM>|--splunk-token <TOKEN>]"
  echo
  echo "  --demo: deploy the Google Online Boutique Microservices Demo"
  echo "          see https://github.com/GoogleCloudPlatform/microservices-demo"
  echo "  --ebpf-net: deploy OpenTelementry eBPF"
  echo "  --ebpf-net-debug: enable debug logging for OpenTelemetry eBPF"
  echo "  --ebpf-net-local: use local docker registry to deploy OpenTelementry eBPF images (default is to use public quay.io/signalfx images)"
  echo "  --ebpf-net-logging-exporter: enable logging exporter in OpenTelemetry Collector deployed with OpenTelemetry eBPF"
  echo "  --ebpf-net-trace: enable trace logging for OpenTelemetry eBPF"
  echo "  --help: display this help message and the container's help message"
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

    --ebpf-net)
      deploy_ebpf_net="true"
      ;;

    --ebpf-net-debug)
      ebpf_net_log_level="--set=networkExplorer.log.level=debug"
      ;;

    --ebpf-net-local)
      ebpf_net_use_local_registry="true"
      ;;

    --ebpf-net-logging-exporter)
      ebpf_net_logging_exporter="--set=gateway.config.service.pipelines.metrics.exporters=logging"
      ;;

    --ebpf-net-trace)
      ebpf_net_log_level="--set=networkExplorer.log.level=trace"
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

if [[ "$deploy_microservices_demo" == "" && "$deploy_otel_demo" == "" && "$deploy_ebpf_net" == "" ]]
then
  echo -e "Need to specify what to deploy.\n"
  print_help
  exit 1
fi

set -x

if [[ "$deploy_microservices_demo" == "true" ]]
then
  microk8s kubectl create ns demo-ns || true
  microk8s kubectl config set-context --current --namespace demo-ns

  microk8s kubectl apply -f ~/microservices-demo/release/kubernetes-manifests.yaml
fi

if [[ "$deploy_otel_demo" == "true" ]]
then
  microk8s kubectl create ns otel-demo-ns || true
  microk8s kubectl config set-context --current --namespace otel-demo-ns

  microk8s helm repo add open-telemetry https://open-telemetry.github.io/opentelemetry-helm-charts
  microk8s helm -n otel-demo-ns install otel-demo open-telemetry/opentelemetry-demo
fi

if [[ "$deploy_ebpf_net" == "true" ]]
 then
  microk8s kubectl create ns ebpf-net-ns || true
  microk8s kubectl config set-context --current --namespace ebpf-net-ns

  microk8s helm repo add splunk-otel-collector-chart https://signalfx.github.io/splunk-otel-collector-chart
  microk8s helm repo update

  ebpf_net_yaml="-f ebpf-net.yaml"
  if [[ "${ebpf_net_use_local_registry}" == "true" ]]
  then
    # pull from local registry, tag, push to microk8s registry
    # TODO can microk8s cluster access local registry directly, possibly using microk8s host-access addon?
    docker pull localhost:5000/kernel-collector:latest
    docker tag $(docker images | grep localhost:5000/kernel-collector | awk '{print $3'}) localhost:32000/kernel-collector:latest
    docker push localhost:32000/kernel-collector:latest

    docker pull localhost:5000/reducer:latest
    docker tag $(docker images | grep localhost:5000/reducer | awk '{print $3'}) localhost:32000/reducer:latest
    docker push localhost:32000/reducer:latest

    ebpf_net_yaml="${ebpf_net_yaml} -f ebpf-net-local-registry.yaml"
  fi

  microk8s helm install ebpf-net \
    ${ebpf_net_yaml} \
    ${ebpf_net_log_level} \
    ${ebpf_net_logging_exporter} \
    ${splunk_args} \
    splunk-otel-collector-chart/splunk-otel-collector
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
  num_not_running=$(microk8s kubectl get pods -A | egrep -v "^NAME|Running" | wc -l)
  if [[ "${num_not_running}" == "0" ]]
  then
    break
  fi

  remaining_attempts=$(($remaining_attempts-1))
  if [[ $remaining_attempts == 0 ]]
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
