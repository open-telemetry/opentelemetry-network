// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.extensions

import io.opentelemetry.render.render.App
import io.opentelemetry.render.render.Document

class AppExtensions {

  static def hashName(App app) {
    app.name + "_hash"
  }

  static def hashFunctor(App app) {
    app.name + "_hasher_t"
  }

  static def hashSize(App app) {
    app.hashName.toUpperCase + "_SIZE"
  }

  static def pkg(App app) {
    (app.eContainer as Document).package
  }

  static def metrics(App app) {
    (app.eContainer as Document).metrics
  }

  static def remoteApps(App app) {
    return app.spans.filter[isProxy].map[remoteApp].toSet
  }

  static def messages(App app) {
    app.spans.flatMap[messages].toList
  }

}
