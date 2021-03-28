//
// Copyright 2021 Splunk Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

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
