// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0
package io.opentelemetry.render.extensions;

import io.opentelemetry.render.render.MetricField;

public class MetricFieldExtensions {
  public static CharSequence cType(final MetricField field) {
    return FieldTypeExtensions.cType(field.getType(), (-1));
  }
}