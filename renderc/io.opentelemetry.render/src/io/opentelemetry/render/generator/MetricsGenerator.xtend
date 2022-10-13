// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.generator

import org.eclipse.emf.ecore.resource.Resource
import org.eclipse.xtext.generator.IFileSystemAccess2

import io.opentelemetry.render.render.AggregationMethod
import io.opentelemetry.render.render.Metric
import io.opentelemetry.render.render.MetricField
import io.opentelemetry.render.render.PackageDefinition
import static io.opentelemetry.render.generator.RenderGenerator.generatedCodeWarning
import static extension io.opentelemetry.render.extensions.MetricFieldExtensions.cType

class MetricsGenerator {

  def void doGenerate(Resource resource, IFileSystemAccess2 fsa) {
    val pkgName = resource.allContents.filter(PackageDefinition).head.name
    fsa.generateFile(pkgName + "/metrics.h", generateMetricsH(resource, pkgName))
  }

  private def generateMetricPointField(MetricField field) {
    if (field.method == AggregationMethod.TDIGEST) {
      '''double «field.name»;'''
    } else {
      '''«field.cType» «field.name»;'''
    }
  }

  private def generateField(MetricField field) {
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
  private def generateMetricsH(Resource resource, String pkg_name) {
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
       *        at the root of a aggregation tree.
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
    } // namespace metrics

    } // namespace «pkg_name»
    '''
  }

}