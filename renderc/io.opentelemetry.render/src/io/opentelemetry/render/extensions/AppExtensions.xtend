// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package net.flowmill.render.extensions

import static extension net.flowmill.render.extensions.UtilityExtensions.toCamelCase
import net.flowmill.render.render.App
import net.flowmill.render.render.Document

class AppExtensions {
	static def connName(App app) {
		app.name.toCamelCase + "Connection"
	}
	static def configClass(App app) {
		app.name.toCamelCase + "DefaultConfig"
	}
	static def transformBuilder(App app) {
		app.name.toCamelCase + "TransformBuilder"
	}
	static def printerName(App app) {
		app.name.toCamelCase + "Printer"
	}
	static def hashName(App app) {
		app.name + "_hash"
	}
	static def hashSize(App app) {
		app.hashName.toUpperCase + "_SIZE"
	}

	static def pkg(App app) {
		(app.eContainer as Document).package
	}

	static def remoteApps(App app) {
		return app.spans.filter[isProxy].map[remoteApp].toSet
	}
}
