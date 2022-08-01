// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render

class RenderStandaloneSetup extends RenderStandaloneSetupGenerated {
	def static void doSetup() {
		new RenderStandaloneSetup().createInjectorAndDoEMFRegistration()
	}
}
