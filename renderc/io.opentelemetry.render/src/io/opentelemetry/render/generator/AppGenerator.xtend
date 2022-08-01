// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.generator

import org.eclipse.emf.ecore.resource.Resource
import org.eclipse.xtext.generator.IFileSystemAccess2
import io.opentelemetry.render.render.App

import static extension io.opentelemetry.render.extensions.AppExtensions.*
import static extension io.opentelemetry.render.extensions.SpanExtensions.*

class AppGenerator {


	static def void doGenerate(Resource resource, IFileSystemAccess2 fsa,
		String pkg_name)
	{

		val apps = resource.allContents.filter(App).toList

		for (app : apps) {
			val basename = pkg_name + "/" + app.name

			fsa.generateFile(basename + "/span_base.h", generateSpanBaseH(app))

			fsa.generateFile(basename + "/protocol.h", ProtocolGenerator.generateProtocolH(app, pkg_name))
			fsa.generateFile(basename + "/protocol.cc", ProtocolGenerator.generateProtocolCc(app))
			fsa.generateFile(basename + "/transform_builder.h", TransformBuilderGenerator.generateTransformerH(app, pkg_name))
			fsa.generateFile(basename + "/transform_builder.cc", TransformBuilderGenerator.generateTransformerCc(app))
			fsa.generateFile(basename + "/connection.h", ConnectionGenerator.generateConnectionH(app))
			fsa.generateFile(basename + "/connection.cc", ConnectionGenerator.generateConnectionCc(app))
			fsa.generateFile(basename + "/bpf.h", BpfGenerator.generateBpfH(app))
			fsa.generateFile(basename + "/writer.h", WriterGenerator.generateWriterH(app, pkg_name))
			fsa.generateFile(basename + "/writer.cc", WriterGenerator.generateWriterCc(app, pkg_name))
			fsa.generateFile(basename + "/encoder.h", WriterGenerator.generateEncoderH(app, pkg_name))
			fsa.generateFile(basename + "/encoder.cc", WriterGenerator.generateEncoderCc(app, pkg_name))
			fsa.generateFile(basename + "/otlp_log_encoder.h", WriterGenerator.generateOtlpLogEncoderH(app, pkg_name))
			fsa.generateFile(basename + "/otlp_log_encoder.cc", WriterGenerator.generateOtlpLogEncoderCc(app, pkg_name))

			val perfect_hash = new PerfectHash(app, false)
			fsa.generateFile(basename + "/hash.h", perfect_hash.generateH())
			fsa.generateFile(basename + "/hash.c", perfect_hash.generateC())

			fsa.generateFile(basename + '/index.h', SpanGenerator.generateIndexH(app, pkg_name))
			fsa.generateFile(basename + '/index.cc', SpanGenerator.generateIndexCc(app, pkg_name))

			fsa.generateFile(basename + '/containers.h', SpanGenerator.generateContainersH(app, pkg_name))
			fsa.generateFile(basename + '/containers.inl', SpanGenerator.generateContainersInl(app, pkg_name))
			fsa.generateFile(basename + '/containers.cc', SpanGenerator.generateContainersCc(app, pkg_name))

			fsa.generateFile(basename + '/keys.h', SpanGenerator.generateKeysH(app, pkg_name))

			fsa.generateFile(basename + '/handles.h', SpanGenerator.generateHandlesH(app, pkg_name))
			fsa.generateFile(basename + '/handles.cc', SpanGenerator.generateHandlesCc(app, pkg_name))

			fsa.generateFile(basename + '/auto_handles.h', SpanGenerator.generateAutoHandlesH(app, pkg_name))
			fsa.generateFile(basename + '/auto_handles.cc', SpanGenerator.generateAutoHandlesCc(app, pkg_name))

			fsa.generateFile(basename + '/weak_refs.h', SpanGenerator.generateWeakRefsH(resource, app, pkg_name))
			fsa.generateFile(basename + '/weak_refs.inl', SpanGenerator.generateWeakRefsInl(app, pkg_name))
			fsa.generateFile(basename + '/weak_refs.cc', SpanGenerator.generateWeakRefsCc(app, pkg_name))

			fsa.generateFile(basename + '/modifiers.h', SpanGenerator.generateModifiersH(app, pkg_name))
			fsa.generateFile(basename + '/modifiers.cc', SpanGenerator.generateModifiersCc(app, pkg_name))

			fsa.generateFile(basename + '/spans.h', SpanGenerator.generateSpansH(app, pkg_name))
			fsa.generateFile(basename + '/spans.cc', SpanGenerator.generateSpansCc(app, pkg_name))

			fsa.generateFile(basename + '/auto_handle_converters.h', SpanGenerator.generateAutoHandleConvertersH(app, pkg_name))
			fsa.generateFile(basename + '/auto_handle_converters.cc', SpanGenerator.generateAutoHandleConvertersCc(app, pkg_name))

			fsa.generateFile(basename + '/meta.h', SpanGenerator.generateMetaH(app, pkg_name))
		}
	}


	static def generateSpanBaseH(App app)
	{
		'''
		/********************************************
		 * JITBUF GENERATED CODE
		 * !!! generated code, do not modify !!!
		 ********************************************/

		#pragma once

		#include "weak_refs.h"

		/* message structs for decoding */
		#include "generated/«app.jsrv_h»"
		#include "generated/«app.descriptor_h»"

		#include <platform/types.h>

		namespace «app.pkg.name»::«app.name» {

		/******************************************************************************
		 * Span handlers
		 ******************************************************************************/

		«FOR span : app.spans»
		/**
		 * «span.baseClassName»
		 */
		class «span.baseClassName» {
		public:
			/** handlers */
			«FOR msg : span.messages»
				void «msg.name»(
					::«app.pkg.name»::«app.name»::weak_refs::«span.name» span_ref,
					u64 timestamp, «msg.parsed_msg.struct_name» *msg) {}
			«ENDFOR»
		};

		«ENDFOR»
		} /* namespace «app.pkg.name»::«app.name» */
		'''
	}
}
