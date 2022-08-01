// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.generator

import io.opentelemetry.render.render.AggregationMethod
import io.opentelemetry.render.render.Field
import io.opentelemetry.render.render.Metric
import io.opentelemetry.render.render.MetricField
import io.opentelemetry.render.render.PackageDefinition
import org.eclipse.emf.ecore.resource.Resource
import org.eclipse.xtext.generator.AbstractGenerator
import org.eclipse.xtext.generator.IFileSystemAccess2
import org.eclipse.xtext.generator.IGeneratorContext
import io.opentelemetry.render.render.FieldTypeEnum
import io.opentelemetry.render.render.App
import java.io.ByteArrayOutputStream
import org.eclipse.xtext.resource.SaveOptions
import static extension io.opentelemetry.render.extensions.MetricFieldExtensions.*

/**
 * Generates code from your model files on save.
 *
 * See https://www.eclipse.org/Xtext/documentation/303_runtime_concepts.html#code-generation
 */
class RenderGenerator extends AbstractGenerator {

	override void doGenerate(Resource resource, IFileSystemAccess2 fsa,
		IGeneratorContext context)
	{
		val pkg_name = resource.allContents.filter(PackageDefinition).head.name

		// pack messages
		resource.allContents.filter(App).forEach[a | AppPacker.populate(a)]

		// output a file with the enriched model
		val resource_stream = new ByteArrayOutputStream()
		val save_options = SaveOptions.newBuilder.format.options.toOptionsMap
		resource.save(resource_stream, save_options)
		fsa.generateFile('render.packed', resource_stream.toString)

		MessageGenerator.doGenerate(resource, fsa, pkg_name)

		AppGenerator.doGenerate(resource, fsa, pkg_name)

		fsa.generateFile(pkg_name + '/metrics.h', generateMetricsH(resource, pkg_name))
	}

	def generatedCodeWarning() {
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

	def generateMetricPointField(MetricField field) {
		if (field.method == AggregationMethod.TDIGEST) {
			'''double «field.name»;'''
		} else {
			'''«field.cType» «field.name»;'''
		}
	}

	def generateField(MetricField field) {
		if (field.method == AggregationMethod.TDIGEST) {
			'''::util::TDigest «field.name»;'''
		} else if (field.method == AggregationMethod.GAUGE) {
			'''::data::Gauge<«field.cType»> «field.name»;'''
		} else if (field.method == AggregationMethod.COUNTER) {
			'''::data::Counter<«field.cType»> «field.name»;'''
		} else { // rate is default
			'''«field.cType» «field.name»;'''
		}
	}

	/***************************************************************************
	 * METRICS H
	 **************************************************************************/
	def generateMetricsH(Resource resource, String pkg_name) {
		'''
		«generatedCodeWarning()»
		#pragma once

		#include <platform/types.h>
		#include <util/counter.h>
		#include <util/gauge.h>
		#include <util/tdigest.h>

		namespace «pkg_name» {
		namespace metrics {

		/**
		 * «pkg_name»::metrics - Structs to hold aggregated telemetry data
		 */
		«FOR metric : resource.allContents.filter(Metric).toIterable»
			/**
			 * struct «metric.name»: holds aggregated data for metric «metric.name»
			 */
			struct «metric.name» {
				«FOR field : metric.fields»
					«generateField(field)»
				«ENDFOR»

				void dump_json(std::ostream &out) const {
					// dumps object in JSON format
					«FOR field : metric.fields BEFORE "out" SEPARATOR " << ','" AFTER ";"»
						<< "\"«field.name»\":\"" << «field.name» << '"'
					«ENDFOR»
				}

				friend std::ostream &operator <<(std::ostream &out, «metric.name» const &what) {
					what.dump_json(out);
					return out;
				}
			};
			/**
			 * struct «metric.name»_point: holds one data point for metric «metric.name»
			 */
			struct «metric.name»_point {
				«FOR field : metric.fields»
					«generateMetricPointField(field)»
				«ENDFOR»

				void dump_json(std::ostream &out) const {
					// dumps object in JSON format
					«FOR field : metric.fields BEFORE "out" SEPARATOR " << ','" AFTER ";"»
						<< "\"«field.name»\":\"" << «field.name» << '"'
					«ENDFOR»
				}

				friend std::ostream &operator <<(std::ostream &out, «metric.name»_point const &what) {
					what.dump_json(out);
					return out;
				}
			};
			/**
			 * struct «metric.name»_struct: used to accumulate data of metric «metric.name»
			 *				at the root of a aggregation tree.
			 */
			struct «metric.name»_accumulator {
				«metric.name» m;
				«FOR td : metric.fields.filter[method == AggregationMethod::TDIGEST]»
				::util::TDigestAccumulator «td.name»;
				«ENDFOR»

				/**
				 * Constructs «metric.name»_accumulator object.
				 */
				«metric.name»_accumulator()
				«FOR td : metric.fields.filter[method == AggregationMethod::TDIGEST] BEFORE ": " SEPARATOR ","»
					«td.name»(m.«td.name»)
				«ENDFOR»
				{}

				/**
				 * Returns enclosed «metric.name» metric object
				 */
				«metric.name»& get_metrics() {
				«FOR td : metric.fields.filter[method == AggregationMethod::TDIGEST]»
					«td.name».flush();
				«ENDFOR»
					return m;
				}

				void dump_json(std::ostream &out) const {
					// dumps object in JSON format
					«FOR field : metric.fields BEFORE "out" SEPARATOR " << ','" AFTER ";"»
						«IF field.method == AggregationMethod::TDIGEST»
							<< "\"«field.name»\":\"" << «field.name» << '"'
						«ELSE»
							<< "\"«field.name»\":\"" << m.«field.name» << '"'
						«ENDIF»
					«ENDFOR»
				}

				friend std::ostream &operator <<(std::ostream &out, «metric.name»_accumulator const &what) {
					what.dump_json(out);
					return out;
				}
			};
		«ENDFOR»
		} /* namespace metrics */
		} /* namespace «pkg_name» */
		'''
	}

}
