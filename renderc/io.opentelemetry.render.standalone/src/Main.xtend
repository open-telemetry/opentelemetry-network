// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.standalone

import com.google.common.collect.ImmutableList
import com.google.common.collect.ImmutableSet
import com.google.inject.Guice
import java.io.File
import java.util.ArrayList
import java.util.Set
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
	String input_directory
	
	@Option(name = "-o", usage="output directory", required=true)
	String output_directory
	
	def static void main(String[] args) {
		new Main().doMain(args)	
	}
	
	def void doMain(String[] args) {
		val parser_properties = ParserProperties.defaults().withUsageWidth(80)
		val parser = new CmdLineParser(this, parser_properties)

		try {
			parser.parseArgument(args)
		} catch( CmdLineException e ) {
			println(e.message)
			println("java RenderStandalone [options...]")
			parser.printUsage(System.err)
			System.exit(-1)
		}
		
		val injector = Guice.createInjector(new StandaloneBuilderModule())
		val builder = injector.getInstance(StandaloneBuilder)
		
		val paths = new ArrayList<String>()
		paths.add(new File(input_directory).absolutePath)
		builder.sourceDirs = paths
		
		val languages = new LanguageAccessFactory().createLanguageAccess(
			ImmutableList.of(new RenderLanguageConfiguration(output_directory)), 
			Main.getClassLoader())
		builder.setBaseDir(new File('.').absolutePath)
		builder.languages = languages
		
		builder.classPathEntries = ImmutableList.<String> of ()

		val succeeded = builder.launch()
		if (!succeeded) {
			System.exit(-1);
		}
	}
	
	static class RenderLanguageConfiguration implements ILanguageConfiguration {
		String output_directory
		
		new(String output_dir) {
			output_directory = output_dir
		}
		
		override String getSetup() {
			"io.opentelemetry.render.RenderStandaloneSetup"
		}
		
		override Set<OutputConfiguration> getOutputConfigurations() {
			val config = new OutputConfiguration(IFileSystemAccess.DEFAULT_OUTPUT)
			config.setOutputDirectory(output_directory)
			
			return ImmutableSet.of(config)
		}
		
		override boolean isJavaSupport() {
			return false
		}
	}
}
