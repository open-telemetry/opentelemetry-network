# Changelog

## [0.11.1](https://github.com/open-telemetry/opentelemetry-network/compare/v0.11.0...v0.11.1) (2025-11-19)


### Features

* **build:** add build environment multi-arch builds ([#400](https://github.com/open-telemetry/opentelemetry-network/issues/400)) ([5ccb104](https://github.com/open-telemetry/opentelemetry-network/commit/5ccb1042704d9c2a677d15cdb0429fc25fa8f571))
* **build:** add devcontainer.json for local builds ([#396](https://github.com/open-telemetry/opentelemetry-network/issues/396)) ([0595e13](https://github.com/open-telemetry/opentelemetry-network/commit/0595e130a2fb79fbf8223d062ad8a1c3f4c34560))
* **build:** refactor version info to cc file to speed up build ([#399](https://github.com/open-telemetry/opentelemetry-network/issues/399)) ([e9cb2b5](https://github.com/open-telemetry/opentelemetry-network/commit/e9cb2b5c82fbf77b2e655a4599d47e71c6e9c6c6))
* **collectors:** remove breakpad crash handlers ([#436](https://github.com/open-telemetry/opentelemetry-network/issues/436)) ([0789b71](https://github.com/open-telemetry/opentelemetry-network/commit/0789b71362434440f4e9dbce799a1e7728d10026))
* **FastDiv:** add remainder computation ([#461](https://github.com/open-telemetry/opentelemetry-network/issues/461)) ([24a417c](https://github.com/open-telemetry/opentelemetry-network/commit/24a417c4b0666e573e30517d70e8f8ccabd10c87))
* **k8s-collector:** add descriptive errors on k8s API connection failure ([454e8a4](https://github.com/open-telemetry/opentelemetry-network/commit/454e8a4259fd81b72ca688c3f111ea1477f90d19))
* **k8s-collector:** add heartbeat on connection to keep it alive ([ce0dd93](https://github.com/open-telemetry/opentelemetry-network/commit/ce0dd93ff58be4d5da32c98478ae45b68693990d))
* **k8s-collector:** add info messages on start ([7e3829c](https://github.com/open-telemetry/opentelemetry-network/commit/7e3829c80f08a423dd11f2054b7e4ba1215473fe))
* **k8s-collector:** add sending of version info message on handshake ([12ccda1](https://github.com/open-telemetry/opentelemetry-network/commit/12ccda152187968901d7c1f60023b751da14bc64))
* **k8s-collector:** flush data to reducer at most 100ms after processing ([000652b](https://github.com/open-telemetry/opentelemetry-network/commit/000652b63f5bdc7669087d481e750d075f391865))
* **k8s-collector:** improve reconnection log messaging ([4f0a02c](https://github.com/open-telemetry/opentelemetry-network/commit/4f0a02c662b90571567079f9c9351be107154505))
* **kernel-collector:** enhance iovec support for http handlers ([#394](https://github.com/open-telemetry/opentelemetry-network/issues/394)) ([e0d91c7](https://github.com/open-telemetry/opentelemetry-network/commit/e0d91c7d6bae29a084dc5cdf6736e5814f562bf5))
* **kernel-collector:** Use Rust main for kernel-collector ([#420](https://github.com/open-telemetry/opentelemetry-network/issues/420)) ([c9ca1d3](https://github.com/open-telemetry/opentelemetry-network/commit/c9ca1d3f895e4dd8fd28f2d52efd5d7690800df8))
* **reducer:** add aggregation framework and tests ([#462](https://github.com/open-telemetry/opentelemetry-network/issues/462)) ([dfa1c36](https://github.com/open-telemetry/opentelemetry-network/commit/dfa1c3622ed0d3cc003b89bdc68112d2b4febf5a))
* **reducer:** Add skeleton Rust agg core ([#456](https://github.com/open-telemetry/opentelemetry-network/issues/456)) ([6b23665](https://github.com/open-telemetry/opentelemetry-network/commit/6b23665ca59acd31864b948ecdba05d96d13d543))
* **reducer:** Migrate OpenTelemetry exporter to Rust ([#435](https://github.com/open-telemetry/opentelemetry-network/issues/435)) ([d8f0476](https://github.com/open-telemetry/opentelemetry-network/commit/d8f0476fc9aa6c160fc785dce7a5ab75d1a80421))
* **reducer:** Port aggregation core to Rust ([#463](https://github.com/open-telemetry/opentelemetry-network/issues/463)) ([714dbf9](https://github.com/open-telemetry/opentelemetry-network/commit/714dbf9099ea30fc7f5f8c79edbba1be2f849682))
* **reducer:** port timeslot utilities, element_queue, ReducerConfig to Rust and remove signal_handler dependence on parser ([#446](https://github.com/open-telemetry/opentelemetry-network/issues/446)) ([20aee38](https://github.com/open-telemetry/opentelemetry-network/commit/20aee389292ab046d5885aed28204d7a5362e494))
* **reducer:** Switch reducer to Rust main ([#432](https://github.com/open-telemetry/opentelemetry-network/issues/432)) ([aff9acd](https://github.com/open-telemetry/opentelemetry-network/commit/aff9acdc4b234067c78ceea8962b056caf997f4d))
* **render:** add parsed messages and decode code ([#458](https://github.com/open-telemetry/opentelemetry-network/issues/458)) ([320e7cb](https://github.com/open-telemetry/opentelemetry-network/commit/320e7cb20746deed9359c2dadc58caabb58cb80f))
* **render:** add perfect_hash_map crate ([#425](https://github.com/open-telemetry/opentelemetry-network/issues/425)) ([d7f9f1d](https://github.com/open-telemetry/opentelemetry-network/commit/d7f9f1d65ea987d826a9867fe8fcd302b7e07168))
* **render:** Add render_parser library ([#428](https://github.com/open-telemetry/opentelemetry-network/issues/428)) ([876ab02](https://github.com/open-telemetry/opentelemetry-network/commit/876ab02ec0f4162ccadec865475b95e3d313e54f))
* **render:** build Rust libraries from crates/; add render_source_dir target ([#430](https://github.com/open-telemetry/opentelemetry-network/issues/430)) ([fa65668](https://github.com/open-telemetry/opentelemetry-network/commit/fa65668de7da54f41b133da6142a4009089da996))
* **render:** Generate Rust perfect hash ([#423](https://github.com/open-telemetry/opentelemetry-network/issues/423)) ([b81c3d3](https://github.com/open-telemetry/opentelemetry-network/commit/b81c3d355a8a8f414a984e5186993424f9dc2b2e))
* **render:** port message serialization to Rust ([#410](https://github.com/open-telemetry/opentelemetry-network/issues/410)) ([7e65aba](https://github.com/open-telemetry/opentelemetry-network/commit/7e65aba501c93759ff7f9361b135c82daa3756b0))
* start implementing arm64 support ([#313](https://github.com/open-telemetry/opentelemetry-network/issues/313)) ([ae3083a](https://github.com/open-telemetry/opentelemetry-network/commit/ae3083a549d79e2e38c8165cb9e639ebae3a1f0c))


### Bug Fixes

* **benv:** add home link to /install ([#382](https://github.com/open-telemetry/opentelemetry-network/issues/382)) ([540cbd9](https://github.com/open-telemetry/opentelemetry-network/commit/540cbd913458609d2ce0802731d6799f0bd4f19d))
* **build:** patch google depot_tools's insertion of default.xml ([#397](https://github.com/open-telemetry/opentelemetry-network/issues/397)) ([648d147](https://github.com/open-telemetry/opentelemetry-network/commit/648d14746d4acd918a9abf249f299681fb3be52e))
* **ci:** add curl to kernel collector test container ([#387](https://github.com/open-telemetry/opentelemetry-network/issues/387)) ([8db3bd4](https://github.com/open-telemetry/opentelemetry-network/commit/8db3bd4536668ac68bd605c13fce5e7b3ff1cf07))
* **deb/rpm:** fix package build for release ([36c584c](https://github.com/open-telemetry/opentelemetry-network/commit/36c584cb5d3f1434dbc93ddc22f0fceb79529a13))
* **deps:** update dependency org.eclipse.emf:org.eclipse.emf.mwe2.launch to v2.23.0 ([#366](https://github.com/open-telemetry/opentelemetry-network/issues/366)) ([c32c326](https://github.com/open-telemetry/opentelemetry-network/commit/c32c3261e735c18831acd31e8a619d2d0f827a31))
* **deps:** update xtextversion to v2.40.0 ([#368](https://github.com/open-telemetry/opentelemetry-network/issues/368)) ([e7b9288](https://github.com/open-telemetry/opentelemetry-network/commit/e7b9288eb172b54f61e0ba65d92b5100c62ffafe))
* **k8s-collector:** add connect message to handshake ([1dd8baa](https://github.com/open-telemetry/opentelemetry-network/commit/1dd8baa21b1220c5d637064fdf5e40314594cedb))
* **k8s-collector:** keep separate Stores for ReplicaSet and Job metadata ([963e7f5](https://github.com/open-telemetry/opentelemetry-network/commit/963e7f58b804b3ac191faee96e441703534bc4d3))
* **kernel-collector:** fix string comparison in http detection ([#393](https://github.com/open-telemetry/opentelemetry-network/issues/393)) ([e40d331](https://github.com/open-telemetry/opentelemetry-network/commit/e40d331d47373461180e8237cddfa68c90a4aee6))
* **kernel-collector:** improve nat lifecycle hooks ([#377](https://github.com/open-telemetry/opentelemetry-network/issues/377)) ([a3315f8](https://github.com/open-telemetry/opentelemetry-network/commit/a3315f8f92871f05a8d52c1205b43c96fb4f6897))
* **kernel-collector:** make DNS kprobe available ([#378](https://github.com/open-telemetry/opentelemetry-network/issues/378)) ([b2118f1](https://github.com/open-telemetry/opentelemetry-network/commit/b2118f1c45c138564ab25b0d6c7753af085b35f1))
* **tests:** Resolve and work around some ASAN issues ([#408](https://github.com/open-telemetry/opentelemetry-network/issues/408)) ([8e0f38e](https://github.com/open-telemetry/opentelemetry-network/commit/8e0f38edf932281b4256985c8002d5cdae730989))


### CI

* add ccache to build-and-test.yaml ([#386](https://github.com/open-telemetry/opentelemetry-network/issues/386)) ([a5d0b75](https://github.com/open-telemetry/opentelemetry-network/commit/a5d0b750757be8a254abf1e314556df84bc6d0ea))
* add end-to-end test with kernel-collector, reducer, otelcol ([#427](https://github.com/open-telemetry/opentelemetry-network/issues/427)) ([bcdfc95](https://github.com/open-telemetry/opentelemetry-network/commit/bcdfc954ecc6d168e72809424af094d2eafe57f6))
* add release-please for triggering releases ([9ed8e06](https://github.com/open-telemetry/opentelemetry-network/commit/9ed8e06a61ec68191732a35c24e37b19c59dee68))
* increase workload visibility in bpf_log test ([#395](https://github.com/open-telemetry/opentelemetry-network/issues/395)) ([f585685](https://github.com/open-telemetry/opentelemetry-network/commit/f585685ff4d4c08e48ec9f13fc09588b9271fb6d))
* install packages for LVH using cache ([#405](https://github.com/open-telemetry/opentelemetry-network/issues/405)) ([f5b9504](https://github.com/open-telemetry/opentelemetry-network/commit/f5b95044bbf4e02019a5ad87e6647e91f82eeffd))
* reduce ccache sizes in build-and-test ([#407](https://github.com/open-telemetry/opentelemetry-network/issues/407)) ([892690d](https://github.com/open-telemetry/opentelemetry-network/commit/892690d40a8a5e7136ce533ff7b77a9b8b99b0e2))
* update e2e test to include k8s-collector ([3d67bf3](https://github.com/open-telemetry/opentelemetry-network/commit/3d67bf3eb79643726d3eebac5d0f8c9e936f7ddc))
* upload e2e container logs as artifacts ([71b6195](https://github.com/open-telemetry/opentelemetry-network/commit/71b619543c5ee6069f18c1cd43186c1aec706fb1))
* verify packaging deb and rpm succeeds ([68f0c23](https://github.com/open-telemetry/opentelemetry-network/commit/68f0c23f6b6015e5ac0d4a2283c08fcfc757d596))

## Changelog

All notable changes to this project will be documented in this file by Release Please.
