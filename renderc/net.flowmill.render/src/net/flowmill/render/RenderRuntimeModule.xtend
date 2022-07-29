// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package net.flowmill.render

import com.google.inject.Provider
import org.eclipse.xtext.formatting2.IFormatter2
import net.flowmill.render.generator.RenderFormatter

class RenderRuntimeModule extends AbstractRenderRuntimeModule {
	def Provider<IFormatter2> provideIFormatter2() {
		return new RenderFormatter.Factory()
	}
}
