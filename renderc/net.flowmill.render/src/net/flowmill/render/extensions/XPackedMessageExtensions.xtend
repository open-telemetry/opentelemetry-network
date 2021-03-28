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

import net.flowmill.render.render.XPackedMessage
import net.flowmill.render.render.FieldType
import static extension net.flowmill.render.extensions.FieldTypeExtensions.cType

class XPackedMessageExtensions {
	static def cType(XPackedMessage packed_msg, FieldType fieldType)
	{
		fieldType.cType(packed_msg.packedStrings)
	}

}
