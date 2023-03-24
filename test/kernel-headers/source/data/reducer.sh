#!/bin/bash
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set -xeE

trap 'catch $? $LINENO' ERR
catch() {
  echo "Error $1 occurred at $0 line $2"
  exit $1
}

docker pull localhost:5000/reducer

docker run --detach --rm \
  --network=host \
  localhost:5000/reducer \
  --port 8000 \
  --prom 0.0.0.0:7000 \
  --partitions-per-shard 1 \
  --num-ingest-shards=1 \
  --num-matching-shards=1 \
  --num-aggregation-shards=1 \
  --enable-aws-enrichment \
  --enable-otlp-grpc-metrics \
  --log-console \
  --debug

start_string="Starting OpenTelemetry eBPF Reducer"

remaining_attempts=30
while true
do
  containerid=$(docker ps | grep "reducer" | awk '{print $1}') || true
  if [[ "${containerid}" == "" ]]
  then
    docker ps
    echo "ERROR: Reducer container is not running!"
    exit 1
  fi

  result=$(docker logs ${containerid} | grep "${start_string}") || true
  if [[ "${result}" != "" ]]
  then
    break
  fi

  remaining_attempts=$(($remaining_attempts-1))
  if [[ $remaining_attempts == 0 ]]
  then
    docker ps
    docker logs ${containerid}
    echo "ERROR: Reducer did not start within the time expected!"
    exit 1
  fi

  sleep 1
done

echo "Reducer is running!"
