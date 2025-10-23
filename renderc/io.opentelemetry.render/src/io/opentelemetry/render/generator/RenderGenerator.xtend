// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.generator

import java.io.ByteArrayOutputStream

import org.eclipse.emf.ecore.resource.Resource
import org.eclipse.xtext.generator.AbstractGenerator
import org.eclipse.xtext.generator.IFileSystemAccess2
import org.eclipse.xtext.generator.IGeneratorContext
import org.eclipse.xtext.resource.SaveOptions

import io.opentelemetry.render.render.App
import io.opentelemetry.render.render.Field
import io.opentelemetry.render.render.FieldTypeEnum
import io.opentelemetry.render.render.PackageDefinition

/**
 * Generates code from your model files on save.
 *
 * See https://www.eclipse.org/Xtext/documentation/303_runtime_concepts.html#code-generation
 */
class RenderGenerator extends AbstractGenerator {

  AppGenerator appGenerator = new AppGenerator()
  MetricsGenerator metricsGenerator = new MetricsGenerator()
  RustCargoGenerator rustCargoGenerator = new RustCargoGenerator()

  override void doGenerate(Resource resource, IFileSystemAccess2 fsa, IGeneratorContext context) {
    val apps = resource.allContents.filter(App).toList()

    // Pack messages.
    apps.forEach[a | AppPacker.populate(a)]

    // Output a file with the enriched model.
    {
      val pkgName = resource.allContents.filter(PackageDefinition).head.name
      val resourceStream = new ByteArrayOutputStream()
      val saveOptions = SaveOptions.newBuilder().format().getOptions()
      resource.save(resourceStream, saveOptions.toOptionsMap())
      fsa.generateFile(pkgName + ".render.packed", resourceStream.toString)
    }

    for (app : apps) {
      appGenerator.doGenerate(app, fsa)
    }

    // Generate per-package Rust aggregator crate (bundles all app crates)
    rustCargoGenerator.doGeneratePackage(apps, fsa)

    metricsGenerator.doGenerate(resource, fsa)
  }

  static def generatedCodeWarning() {
    '''
    /**********************************************
     * !!! render-generated code, do not modify !!!
     **********************************************/

    '''
  }

  /***************************************************************************
   * FIELD HELPER FUNCTIONS
   **************************************************************************/
  static def integerTypeSize(FieldTypeEnum enum_type) {
    switch (enum_type) {
        case FieldTypeEnum.S8,
        case FieldTypeEnum.U8: 1
        case FieldTypeEnum.S16,
        case FieldTypeEnum.U16: 2
        case FieldTypeEnum.S32,
        case FieldTypeEnum.U32: 4
        case FieldTypeEnum.S64,
        case FieldTypeEnum.U64: 8
        case FieldTypeEnum.S128,
        case FieldTypeEnum.U128: 16
        case FieldTypeEnum.STRING:
          throw new RuntimeException("String not supported in hash")
    }
  }

  static def fieldSize(Field field) {
    val non_array_size =
      if (field.type.isShortString)
        field.type.size
      else
        integerTypeSize(field.type.enum_type)

    if (field.isArray)
      non_array_size * field.array_size
    else
      non_array_size
  }

}
