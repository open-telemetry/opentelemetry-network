#!/bin/bash -e
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


config_file="$HOME/k8s/loadgen/default.py"
if [[ "$#" -gt 0 ]] && [[ -e "$HOME/k8s/loadgen/$1" ]]; then
	config_file="$HOME/k8s/loadgen/$1"
fi

kubectl create configmap loadgen-config --from-file=locustfile.py="${config_file}"
kubectl apply -f "$HOME/k8s/kubernetes-manifests.yaml"

watch -n 5 kubectl get pods -owide
