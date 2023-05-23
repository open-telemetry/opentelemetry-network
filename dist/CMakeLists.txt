# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set(SYSTEMD_UNIT_DIR "/usr/lib/systemd/system/")
set(CONFIG_DIR "/etc/opentelemetry-ebpf/")

install(
  FILES
    reducer.service
  DESTINATION ${SYSTEMD_UNIT_DIR}
  COMPONENT reducer
)
install(
  FILES
    reducer.args
  DESTINATION ${CONFIG_DIR}
  COMPONENT reducer
)

install(
  FILES
    kernel-collector.service
  DESTINATION ${SYSTEMD_UNIT_DIR}
  COMPONENT kernel-collector
)
install(
  FILES
    kernel-collector.args
  DESTINATION ${CONFIG_DIR}
  COMPONENT kernel-collector
)

install(
  FILES
    cloud-collector.service
  DESTINATION ${SYSTEMD_UNIT_DIR}
  COMPONENT cloud-collector
)
install(
  FILES
    cloud-collector.args
  DESTINATION ${CONFIG_DIR}
  COMPONENT cloud-collector
)
