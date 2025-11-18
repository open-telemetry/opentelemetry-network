# Making a public release
Public builds are those intended to be used by the general audience.

## Release procedure (Release Please)
1. Ensure that changes on `main` use Conventional Commit messages so Release Please can determine the correct next version.
1. Wait for the `release-please` workflow to open a Release PR, or trigger it manually from https://github.com/open-telemetry/opentelemetry-network/actions/workflows/release-please.yml.
1. Review the Release PR:
   - Confirm that `VERSION` and `CHANGELOG.md` are updated as expected.
   - Make any edits you need to the release notes in the PR description.
1. Merge the Release PR.
1. Once merged, Release Please will:
   - Create a Git tag `vMAJOR.MINOR.PATCH`.
   - Publish a GitHub Release with the generated release notes.
1. The `build-and-release` workflow will run automatically on the `release.published` event:
   - It checks out the release tag, builds RPM/DEB packages and container images, and uploads the RPM/DEB packages to the existing GitHub Release.
   - It pushes container images to the configured registry with tags:
     - `latest`
     - `latest-vMAJOR.MINOR`
     - `vMAJOR.MINOR.PATCH`
1. Verify the published GitHub Release at https://github.com/open-telemetry/opentelemetry-network/releases and confirm the assets.
1. On the Docker registry (for example https://hub.docker.com/r/otel/), confirm that the following images are tagged with the new tags:
    * opentelemetry-ebpf-reducer
    * opentelemetry-ebpf-kernel-collector
    * opentelemetry-ebpf-cloud-collector
    * opentelemetry-ebpf-k8s-watcher
    * opentelemetry-ebpf-k8s-relay

If needed, you can still trigger the `build-and-release` workflow manually from
https://github.com/open-telemetry/opentelemetry-network/actions/workflows/build-and-release.yaml
with `release_type: public` to rebuild artifacts for an existing tag.


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
