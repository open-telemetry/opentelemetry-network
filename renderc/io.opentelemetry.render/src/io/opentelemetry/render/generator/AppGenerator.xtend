// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.generator

import org.eclipse.emf.ecore.resource.Resource
import org.eclipse.xtext.generator.IFileSystemAccess2

import io.opentelemetry.render.render.App
import static extension io.opentelemetry.render.extensions.AppExtensions.*

class AppGenerator {

  BpfGenerator bpfGenerator = new BpfGenerator()
  SpanGenerator spanGenerator = new SpanGenerator()
  HashGenerator hashGenerator = new HashGenerator()
  WriterGenerator writerGenerator = new WriterGenerator()
  ProtocolGenerator protocolGenerator = new ProtocolGenerator()
  ConnectionGenerator connectionGenerator = new ConnectionGenerator()
  TransformBuilderGenerator transformBuilderGenerator = new TransformBuilderGenerator()

  def void doGenerate(Resource resource, IFileSystemAccess2 fsa) {
    for (app : resource.allContents.filter(App).toIterable) {
      bpfGenerator.doGenerate(app, fsa)
      spanGenerator.doGenerate(app, fsa)
      hashGenerator.doGenerate(app, fsa)
      writerGenerator.doGenerate(app, fsa)
      protocolGenerator.doGenerate(app, fsa)
      connectionGenerator.doGenerate(app, fsa)
      transformBuilderGenerator.doGenerate(app, fsa)
    }
  }

  static def outputPath(App app, String fileName) {
    app.pkg.name + "/" + app.name + "/" + fileName
  }

}
