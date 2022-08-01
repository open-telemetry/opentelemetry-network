// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package net.flowmill.render

class RenderStandaloneSetup extends RenderStandaloneSetupGenerated {
	def static void doSetup() {
		new RenderStandaloneSetup().createInjectorAndDoEMFRegistration()
	}
}
