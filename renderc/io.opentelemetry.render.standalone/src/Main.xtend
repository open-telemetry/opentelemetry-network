// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.standalone

import java.io.File
import java.util.Set

import com.google.common.collect.ImmutableList
import com.google.common.collect.ImmutableSet
import com.google.inject.Guice

import org.eclipse.xtext.builder.standalone.ILanguageConfiguration
import org.eclipse.xtext.builder.standalone.LanguageAccessFactory
import org.eclipse.xtext.builder.standalone.StandaloneBuilder
import org.eclipse.xtext.builder.standalone.StandaloneBuilderModule
import org.eclipse.xtext.generator.IFileSystemAccess
import org.eclipse.xtext.generator.OutputConfiguration

import org.kohsuke.args4j.CmdLineParser
import org.kohsuke.args4j.ParserProperties
import org.kohsuke.args4j.CmdLineException
import org.kohsuke.args4j.Option

class Main {
  @Option(name = "-i", usage="input directory", required=true)
  String inputDirectory

  @Option(name = "-o", usage="output directory", required=true)
  String outputDirectory

  def static void main(String[] args) {
    new Main().doMain(args)
  }

  def void doMain(String[] args) {
    val parser = new CmdLineParser(this, ParserProperties.defaults().withUsageWidth(80))

    try {
      parser.parseArgument(args)
    } catch (CmdLineException e) {
      System.err.println("Error: " + e.message)
      System.err.println("Usage:")
      parser.printUsage(System.err)
      System.exit(-1)
    }

    val injector = Guice.createInjector(new StandaloneBuilderModule())
    val builder = injector.getInstance(StandaloneBuilder)

    builder.baseDir = new File(".").absolutePath
    builder.sourceDirs = ImmutableList.<String>of(new File(inputDirectory).absolutePath)
    builder.classPathEntries = ImmutableList.<String>of()

    val languages = new LanguageAccessFactory().createLanguageAccess(
      ImmutableList.of(new RenderLanguageConfiguration(outputDirectory)),
      Main.getClassLoader())
    builder.languages = languages

    if (!builder.launch()) {
      System.exit(-1);
    }
  }

  static class RenderLanguageConfiguration implements ILanguageConfiguration {
    String outputDirectory

    new(String outputDir) {
      outputDirectory = outputDir
    }

    override String getSetup() {
      "io.opentelemetry.render.RenderStandaloneSetup"
    }

    override Set<OutputConfiguration> getOutputConfigurations() {
      val config = new OutputConfiguration(IFileSystemAccess.DEFAULT_OUTPUT)
      config.setOutputDirectory(outputDirectory)

      return ImmutableSet.of(config)
    }

    override boolean isJavaSupport() {
      return false
    }
  }
}
