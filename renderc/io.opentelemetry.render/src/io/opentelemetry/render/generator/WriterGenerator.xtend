// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.generator

import io.opentelemetry.render.render.FieldTypeEnum
import io.opentelemetry.render.render.App

import static extension io.opentelemetry.render.extensions.AppExtensions.*
import static extension io.opentelemetry.render.extensions.FieldExtensions.*
import static extension io.opentelemetry.render.extensions.MessageExtensions.*

class WriterGenerator {
	static def generateWriterH(App app, String pkg_name)
	{
		'''
		#pragma once

		#include <generated/«app.jb_h»>
		#include <generated/«app.jsrv_h»>
		#include <generated/«pkg_name»/«app.name»/encoder.h>

		#include <channel/ibuffered_writer.h>
		#include <platform/types.h>

		#include <spdlog/fmt/fmt.h>
		#include <util/log.h>

		#include <functional>
		#include <stdexcept>
		#include <system_error>
		#include <utility>

		namespace «app.pkg.name»::«app.name» {

		class Writer {
		public:
			using clock_t = std::function<u64()>;

			Writer(IBufferedWriter &buffer, clock_t clock, u64 time_adjustment = 0, Encoder *encoder = nullptr);

			Writer(Writer const &);
			Writer(Writer &&);

			Writer &operator =(Writer const &) = delete;
			Writer &operator =(Writer &&) = delete;

			«FOR msg : app.spans.flatMap[messages].toSet»

				/* «msg.name» */
				template <bool ThrowOnError = true>
				inline std::error_code «msg.name»(«msg.prototype») {
					return «msg.name»_tstamp<ThrowOnError>(clock_()+time_adjustment_«msg.commaCallPrototype»);
				}

				template <bool ThrowOnError = true>
				inline std::error_code «msg.name»_tstamp(u64 __tstamp«msg.commaPrototype») {
					auto __result = encoder_->«msg.name»(buffer_, __tstamp«msg.commaCallPrototype»);

					if constexpr (ThrowOnError) {
						if (__result) {
							throw std::system_error(
								__result,
								fmt::format("Writer::«msg.name» failed: {}", __result)
							);
						}
					}

					return __result;
				}
			«ENDFOR»

			bool is_writable() const {
				return buffer_.is_writable();
			}

			std::error_code flush() {
				if (is_writable()) {
					return buffer_.flush();
				}
				return {};
			}

		private:
			IBufferedWriter &buffer_;
			Encoder default_encoder_;
			Encoder *encoder_ = nullptr;
			clock_t clock_;
			u64 time_adjustment_;
		};

		} /* namespace «app.pkg.name»::«app.name» */
		'''
	}

	static def generateWriterCc(App app, String pkg_name)
	{
		'''
		#include <generated/«pkg_name»/«app.name»/writer.h>

		namespace «app.pkg.name»::«app.name» {

		Writer::Writer(IBufferedWriter &buffer, clock_t clock, u64 time_adjustment, Encoder *encoder):
			buffer_(buffer),
			encoder_(encoder ? encoder : &default_encoder_),
			clock_(std::move(clock)),
			time_adjustment_(time_adjustment)
		{}

		Writer::Writer(Writer const &rhs):
			buffer_(rhs.buffer_),
			encoder_(rhs.encoder_ == &rhs.default_encoder_ ? &default_encoder_ : rhs.encoder_),
			clock_(rhs.clock_),
			time_adjustment_(rhs.time_adjustment_)
		{}

		Writer::Writer(Writer &&rhs):
			buffer_(rhs.buffer_),
			encoder_(rhs.encoder_ == &rhs.default_encoder_ ? &default_encoder_ : rhs.encoder_),
			clock_(std::move(rhs.clock_)),
			time_adjustment_(std::move(rhs.time_adjustment_))
		{}

		} /* namespace «app.pkg.name»::«app.name» */
		'''
	}

	static def generateEncoderH(App app, String pkg_name)
	{
		'''
		#pragma once

		#include <generated/«app.jb_h»>
		#include <generated/«app.jsrv_h»>

		#include <channel/ibuffered_writer.h>
		#include <platform/types.h>

		namespace «app.pkg.name»::«app.name» {

		class Encoder {
		public:
			Encoder();
			virtual ~Encoder();

			«FOR msg : app.spans.flatMap[messages].toSet»
			virtual std::error_code «msg.name»(IBufferedWriter &__buffer, u64 __tstamp«msg.commaPrototype»);
			«ENDFOR»
		};

		} /* namespace «app.pkg.name»::«app.name» */
		'''
	}

	static def generateEncoderCc(App app, String pkg_name)
	{
		'''
		#include <generated/«pkg_name»/«app.name»/encoder.h>

		#include <util/log.h>
		#include <util/log_formatters.h>

		namespace «app.pkg.name»::«app.name» {

		Encoder::Encoder() {}
		Encoder::~Encoder() {}
		«FOR msg : app.spans.flatMap[messages].toSet»

			/* «msg.name» */
			std::error_code Encoder::«msg.name»(IBufferedWriter &__buffer, u64 __tstamp«msg.commaPrototype») {
				/* allocate space on the stack, 64-bit aligned */
				/* zero out buffer so we don't exfiltrate uninitialized memory*/
				u64 __buf64[1 + (sizeof(struct «msg.wire_msg.struct_name») + 7) / 8] = {};
				u32 __len = «msg.wire_msg.size» + sizeof(u64);

				__buf64[0] = __tstamp;

				auto __dst_msg = («msg.wire_msg.struct_name» *)&__buf64[1];

				__dst_msg->_rpc_id = «msg.wire_msg.rpc_id»;

				/* copy all non-string fields */
				«FOR field : msg.fields.filter[type.enum_type != FieldTypeEnum.STRING || type.isShortString]»
					«IF field.isArray || field.type.isShortString»
						memcpy(&__dst_msg->«field.name»[0], &«field.name»[0], «field.size(true)»);
					«ELSE»
						__dst_msg->«field.name» = «field.name»;
					«ENDIF»
				«ENDFOR»

				u32 __consumed = «msg.wire_msg.size»;
				«IF msg.wire_msg.dynamic_size»
					/* handle dynamic string lengths */
					«FOR field : msg.wire_msg.fields.filter[type.enum_type == FieldTypeEnum.STRING]»
							/* not the last field: length is in wire message */
							__dst_msg->«field.name» = «field.name».len;
							__consumed += «field.name».len;
					«ENDFOR»
					/* last field: gets the rest of the message */
					__consumed += «msg.wire_msg.last_blob_field.name».len;
					if (__consumed > 0xffff) {
						throw std::runtime_error("Writer::«msg.name» tried to write dynamic message >= 1<<16");
					}
					__dst_msg->_len = (u16)__consumed;
				«ENDIF»

				/* start write */
				auto __allocated = __buffer.start_write(sizeof(u64) + __consumed);
				if (!__allocated) {
					LOG::error(
						"Writer::«msg.name» start_write failed on a closed channel: {}",
						__allocated.error()
					);
					return __allocated.error();
				}

				/* copy fixed part of message */
				char *__dest = (char *)*__allocated;
				assert(__dest);
				memcpy(__dest, __buf64, __len); __dest += __len;

				«IF msg.wire_msg.dynamic_size»
					«FOR field : msg.wire_msg.fields.filter[type.enum_type == FieldTypeEnum.STRING] + #[msg.wire_msg.last_blob_field]»
						/* dynamic string: «field.name» */
						memcpy(__dest, «field.name».buf, «field.name».len);
						__dest += «field.name».len;

					«ENDFOR»
				«ENDIF»
				/* finish write */
				__buffer.finish_write();

				return {};
			}
		«ENDFOR»

		} /* namespace «app.pkg.name»::«app.name» */
		'''
	}

	static def generateOtlpLogEncoderH(App app, String pkg_name)
	{
		'''
		#pragma once

		#include <generated/«pkg_name»/«app.name»/encoder.h>

		#include <channel/ibuffered_writer.h>
		#include <platform/types.h>

		#include <generated/«app.jb_h»>
		#include <generated/«app.jsrv_h»>

		namespace «app.pkg.name»::«app.name» {

		class OtlpLogEncoder: public Encoder {
		public:
			OtlpLogEncoder(std::string const host, std::string port);
			~OtlpLogEncoder();

			«FOR msg : app.spans.flatMap[messages].toSet»
			std::error_code «msg.name»(
				IBufferedWriter &__buffer,
				u64 __tstamp«msg.commaPrototype»
			) override;
			«ENDFOR»

		private:
			std::error_code __write_http_request(
				IBufferedWriter &__buffer,
				std::string_view message,
				std::string_view payload
			);

			std::string const host_;
			std::string port_;

			// when true, encode as a raw json message, otherwise encode as the
			// JSON translated protobuf request for Log Export
			static constexpr bool raw_message = false;
		};

		} /* namespace «app.pkg.name»::«app.name» */
		'''
	}

	static def generateOtlpLogEncoderCc(App app, String pkg_name)
	{
		'''
		#include <generated/«pkg_name»/«app.name»/otlp_log_encoder.h>

		#include <jitbuf/jb.h>
		#include <util/log.h>
		#include <util/log_formatters.h>
		#include <util/raw_json.h>

		namespace «app.pkg.name»::«app.name» {

		OtlpLogEncoder::OtlpLogEncoder(std::string const host, std::string port):
			host_(std::move(host)),
			port_(std::move(port))
		{}

		OtlpLogEncoder::~OtlpLogEncoder() {}

		«FOR msg : app.spans.flatMap[messages].toSet»

			std::error_code OtlpLogEncoder::«msg.name»(
				IBufferedWriter &__buffer,
				u64 __tstamp«msg.commaPrototype»
			) {
				«IF !msg.pipelineOnly»
					std::ostringstream __out;

					/* write protobuf JSON envelope start */

					if constexpr (raw_message) {
						__out << "{\"name\":\"«msg.name»\",\"timestamp\":\"" << __tstamp << "\",\"body\":{";
					} else {
						__out << "{\"resourceLogs\":[{\"instrumentationLibraryLogs\":[{\"log_records\":["
								"{\"name\":\"«msg.name»\",\"timeUnixNano\":\"" << __tstamp << "\",\"body\":"
								"{\"kvlist_value\":{\"values\":[";
					}

					/* encode as json */

					«FOR field : msg.fields.sortBy[id]»
						«IF field.id != msg.fields.map[id].min»
							__out << ',';
						«ENDIF»

						if constexpr (raw_message) {
							__out << "\"«field.name»\":";
						} else {
							__out << "{\"key\":\"«field.name»\",\"value\":{\"stringValue\":\"";
						}

						«IF field.type.isShortString»
							print_escaped_json_string<raw_message>(__out, render_array_to_string_view(«field.name»));
						«ELSEIF field.type.enum_type == FieldTypeEnum.STRING»
							print_escaped_json_string<raw_message>(__out, «field.name»);
						«ELSEIF field.isArray»
							print_escaped_json_string<raw_message>(
								__out,
								std::string_view{
									reinterpret_cast<char const *>(«field.name»),
									«field.array_size»}
							);
						«ELSE»
							if constexpr (raw_message) { __out << '"'; }
							print_json_value(__out, «field.name»);
							if constexpr (raw_message) { __out << '"'; }
						«ENDIF»

						if constexpr (!raw_message) { __out << "\"}}"; }
					«ENDFOR»

					/* write protobuf JSON envelope end */

					if constexpr (raw_message) {
						__out << "}}\n";
					} else {
						__out << "]}}}]}]}]}\n";
					}

					return __write_http_request(__buffer, "«msg.name»", __out.str());
				«ELSE»
					return {};
				«ENDIF»
			}
		«ENDFOR»

		std::error_code OtlpLogEncoder::__write_http_request(
			IBufferedWriter &__buffer,
			std::string_view message,
			std::string_view payload
		) {
			{
				std::stringstream header;

				/* write HTTP header */

				header << "POST /v1/logs HTTP/1.1\r\n"
					"Host: " << host_ << ':' << port_ << "\r\n"
					"Content-Type: application/json\r\n";
				header << "Content-Length: " << payload.size() << "\r\n";
				header << "\r\n";

				if (auto __result = __buffer.write_as_chunks(header.str()); !__result) {
					LOG::error(
						"OTLP Log Encoder: failed to write HTTP header for message `{}`: {}",
						message, __result.error()
					);
					return __result.error();
				}
			}

			/* flush to buffer */

			if (auto __result = __buffer.write_as_chunks(payload); !__result) {
				LOG::error(
					"OTLP Log Encoder: failed to write payload for message `{}`: {}",
					message, __result.error()
				);
				return __result.error();
			}

			return __buffer.flush();
		}

		} /* namespace «app.pkg.name»::«app.name» */
		'''
	}
}
