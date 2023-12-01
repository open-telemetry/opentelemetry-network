# Making a public release
Public builds are those intended to be used by the general audience.

## Release procedure:
1. Choose a commit on the main branch and tag it with the appropriate version tag (vMAJOR.MINOR.PATCH). Confirm that the version tag matches the version numbers in the version.sh file. Push that tag to the main repository (git push –tags).
1. Go to the project's Releases page (https://github.com/open-telemetry/opentelemetry-network/releases) and create a new draft release. Use the tag created in step 1. Use the existing naming convention for the new release name.
1. Navigate to https://github.com/open-telemetry/opentelemetry-network/actions/workflows/build-and-release.yaml and click the "Run workflow" button.

   Select public for the release type and make sure that the image prefix is correct (should be opentelemetry-ebpf-). Use the version tag created in step 1.
1. Download built packages by downloading the packages artifact produced during the workflow run in step 3.

   Upload RPM and DEB packages for the reducer, kernel collector and cloud collector into the GitHub release (edit the release, drop RPMs and DEBs into Attach binaries by dropping them here or selecting them rectangle).
1. Bump the version number in the version.sh file and get that change merged to the main repository.
1. On the docker repository (https://hub.docker.com/r/otel/), confirm that the following images are tagged with new tags (latest, latest-vMAJOR.MINOR, vMAJOR.MINOR.PATCH):
    * opentelemetry-ebpf-reducer
    * opentelemetry-ebpf-kernel-collector
    * opentelemetry-ebpf-cloud-collector
    * opentelemetry-ebpf-k8s-watcher
    * opentelemetry-ebpf-k8s-relay
1. Publish the new GitHub release.


## Unofficial builds
Unofficial builds, while public, are not intended for the use by the general audience. Because of that, no unofficial build should be tagged in such a way that it gets used automatically – no tagging with latest or with a version tag.

Use a personal fork of the opentelemetry-network repository on GitHub. Make sure that the following repository secrets and variables are set up for the fork (Settings -> Secrets and variables -> Actions):

variables:
 - DOCKER_REGISTRY: registry, such as quay.io
 - DOCKER_NAMESPACE: image name, such as o11ytest/network-explorer-debug

secrets:
 - DOCKER_USERNAME: username to login to the registry
 - DOCKER_PASSWORD: password to loging to the registry


Navigate to Actions -> build-and-release, click the "Run workflow" button. Select unofficial for the release type. Use whichever commit SHA, tag or branch name you wish to make a build of.

The resulting docker images will be tagged with vMAJOR.MINOR.PATCH-GITHASH tag.
