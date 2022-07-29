// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

//+build go1.7

package reflect2

import "unsafe"

//go:linkname resolveTypeOff reflect.resolveTypeOff
func resolveTypeOff(rtype unsafe.Pointer, off int32) unsafe.Pointer
