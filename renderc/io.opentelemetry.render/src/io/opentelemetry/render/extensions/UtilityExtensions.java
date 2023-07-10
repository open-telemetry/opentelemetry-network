// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.extensions;

import com.google.common.base.CaseFormat;

public class UtilityExtensions {
  public static String toCamelCase(final String s) {
    return CaseFormat.LOWER_UNDERSCORE.to(CaseFormat.UPPER_CAMEL, s);
  }
}
