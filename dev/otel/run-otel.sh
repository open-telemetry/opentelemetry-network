touch "${PWD}/otel.log"

export OTEL_COLLECTOR_IMAGE=${OTEL_COLLECTOR_IMAGE:-otel/opentelemetry-collector}
export OTEL_COLLECTOR_PORT=${OTEL_COLLECTOR_PORT:-4318}

set -x

docker run \
  --rm \
  ${OTEL_COLLECTOR_ENV_VARS} \
  -p 8000:${OTEL_COLLECTOR_PORT} \
  -v "${PWD}/otel-config.yaml:/etc/otel/config.yaml" \
  -v "${PWD}/otel.log:/var/log/otel.log" \
  ${OTEL_COLLECTOR_IMAGE}
