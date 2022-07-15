add_custom_target(collectors)
add_custom_target(collectors-docker)
add_custom_target(collectors-docker-registry)

add_subdirectory(cloud)
add_subdirectory(k8s)
add_subdirectory(kernel)
