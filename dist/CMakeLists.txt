# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

set(SYSTEMD_UNIT_DIR "/usr/lib/systemd/system")
set(CONFIG_DIR "/etc/opentelemetry-ebpf")

################################################################################
# Reducer
#

string(JOIN ";" CPACK_DEBIAN_REDUCER_PACKAGE_CONTROL_EXTRA
  "${CMAKE_CURRENT_SOURCE_DIR}/reducer/deb/conffiles"
  "${CMAKE_CURRENT_SOURCE_DIR}/reducer/deb/postinst"
  "${CMAKE_CURRENT_SOURCE_DIR}/reducer/deb/prerm"
  "${CMAKE_CURRENT_SOURCE_DIR}/reducer/deb/postrm"
)

set(CPACK_RPM_REDUCER_USER_FILELIST
  "%config(noreplace) ${CONFIG_DIR}/reducer.args"
  "%config(noreplace) ${CONFIG_DIR}/reducer.yaml"
)

set(CPACK_RPM_REDUCER_POST_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/reducer/rpm/post.sh")
set(CPACK_RPM_REDUCER_PRE_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/reducer/rpm/preun.sh")
set(CPACK_RPM_REDUCER_POST_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/reducer/rpm/postun.sh")

install(
  FILES
    reducer/reducer.service
  DESTINATION ${SYSTEMD_UNIT_DIR}
  COMPONENT reducer
)

install(
  FILES
    reducer/reducer.args
    reducer/reducer.yaml
  DESTINATION ${CONFIG_DIR}
  COMPONENT reducer
)

get_target_property(REDUCER_BIN reducer-stripped OUTPUT)
install(
  PROGRAMS ${REDUCER_BIN}
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  RENAME reducer
  COMPONENT reducer
)


################################################################################
# Kernel Collector
#

set(CPACK_RPM_KERNEL-COLLECTOR_PACKAGE_REQUIRES "bash, kernel-devel")
set(CPACK_DEBIAN_KERNEL-COLLECTOR_PACKAGE_DEPENDS "bash, linux-headers")

string(JOIN ";" CPACK_DEBIAN_KERNEL-COLLECTOR_PACKAGE_CONTROL_EXTRA
  "${CMAKE_CURRENT_SOURCE_DIR}/kernel-collector/deb/conffiles"
  "${CMAKE_CURRENT_SOURCE_DIR}/kernel-collector/deb/postinst"
  "${CMAKE_CURRENT_SOURCE_DIR}/kernel-collector/deb/prerm"
  "${CMAKE_CURRENT_SOURCE_DIR}/kernel-collector/deb/postrm"
)

set(CPACK_RPM_KERNEL-COLLECTOR_USER_FILELIST
  "%config(noreplace) ${CONFIG_DIR}/kernel-collector.yaml"
  "%config(noreplace) ${CONFIG_DIR}/kernel-collector.args"
)

set(CPACK_RPM_KERNEL-COLLECTOR_POST_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/kernel-collector/rpm/post.sh")
set(CPACK_RPM_KERNEL-COLLECTOR_PRE_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/kernel-collector/rpm/preun.sh")
set(CPACK_RPM_KERNEL-COLLECTOR_POST_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/kernel-collector/rpm/postun.sh")

install(
  FILES
    kernel-collector/kernel-collector.service
  DESTINATION ${SYSTEMD_UNIT_DIR}
  COMPONENT kernel-collector
)

install(
  FILES
    kernel-collector/kernel-collector.args
    kernel-collector/kernel-collector.yaml
  DESTINATION ${CONFIG_DIR}
  COMPONENT kernel-collector
)

get_target_property(KERNEL_COLLECTOR_BIN kernel-collector-stripped OUTPUT)
install(
  PROGRAMS ${KERNEL_COLLECTOR_BIN}
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  RENAME kernel-collector
  COMPONENT kernel-collector
)


################################################################################
# Cloud Collector
#

string(JOIN ";" CPACK_DEBIAN_CLOUD-COLLECTOR_PACKAGE_CONTROL_EXTRA
  "${CMAKE_CURRENT_SOURCE_DIR}/cloud-collector/deb/conffiles"
  "${CMAKE_CURRENT_SOURCE_DIR}/cloud-collector/deb/postinst"
  "${CMAKE_CURRENT_SOURCE_DIR}/cloud-collector/deb/prerm"
  "${CMAKE_CURRENT_SOURCE_DIR}/cloud-collector/deb/postrm"
)

set(CPACK_RPM_CLOUD-COLLECTOR_USER_FILELIST
  "%config(noreplace) ${CONFIG_DIR}/cloud-collector.yaml"
  "%config(noreplace) ${CONFIG_DIR}/cloud-collector.args"
)

set(CPACK_RPM_CLOUD-COLLECTOR_POST_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cloud-collector/rpm/post.sh")
set(CPACK_RPM_CLOUD-COLLECTOR_PRE_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cloud-collector/rpm/preun.sh")
set(CPACK_RPM_CLOUD-COLLECTOR_POST_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cloud-collector/rpm/postun.sh")

install(
  FILES
    cloud-collector/cloud-collector.service
  DESTINATION ${SYSTEMD_UNIT_DIR}
  COMPONENT cloud-collector
)

install(
  FILES
    cloud-collector/cloud-collector.args
    cloud-collector/cloud-collector.yaml
  DESTINATION ${CONFIG_DIR}
  COMPONENT cloud-collector
)

get_target_property(CLOUD_COLLECTOR_BIN cloud-collector-stripped OUTPUT)
install(
  PROGRAMS ${CLOUD_COLLECTOR_BIN}
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  RENAME cloud-collector
  COMPONENT cloud-collector
)


################################################################################
# Packaging
#

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenTelemetry-eBPF")
set(CPACK_PACKAGE_DESCRIPTION
"The OpenTelemetry eBPF project develops components that collect and analyze
telemetry from the operating system, cloud, and container orchestrators. Its initial focus
is on collecting network data to enable users to gain insight into their distributed
applications.")
set(CPACK_PACKAGE_VENDOR "OpenTelemetry")
set(CPACK_PACKAGE_CONTACT "https://github.com/orgs/open-telemetry/teams/ebpf-maintainers")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/open-telemetry/opentelemetry-ebpf")

set(CPACK_RESOURCE_FILE_README "${PROJECT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE.txt")

set(CPACK_GENERATOR "TGZ;RPM;DEB")
set(CPACK_COMPONENTS_GROUPING IGNORE) # one package per component
set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_DEB_COMPONENT_INSTALL ON)

set(CPACK_RPM_FILE_NAME "RPM-DEFAULT")
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")

set(CPACK_RPM_PACKAGRE_REQUIRES "bash")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "bash")

include(CPack)

cpack_add_component(
  reducer
  DESCRIPTION
    "OpenTelemetry-eBPF Reducer.\n${CPACK_PACKAGE_DESCRIPTION}"
)

cpack_add_component(
  kernel-collector
  DESCRIPTION
    "OpenTelemetry-eBPF Kernel Collector.\n${CPACK_PACKAGE_DESCRIPTION}"
)

cpack_add_component(
  cloud-collector
  DESCRIPTION
    "OpenTelemetry-eBPF Cloud Collector.\n${CPACK_PACKAGE_DESCRIPTION}"
)

cpack_add_component(tools DESCRIPTION "OpenTelemetry-eBPF tools")
