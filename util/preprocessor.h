/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// Preprocessor macro to stringize (convert into a string constant) the result of expansion of a macro argument.
#define PREPROC_STRINGIZE_IMPL(X) #X
#define PREPROC_STRINGIZE(X) PREPROC_STRINGIZE_IMPL(X)

// Preprocessor macro to concatenate two values, one or both of which is the result of expansion of a macro argument.
// One common use is to create a uniquely named anonymous local variable.
// As an example, see DEFER macro.
#define PREPROC_CONCAT_IMPL(X, Y) X##Y
#define PREPROC_CONCAT(X, Y) PREPROC_CONCAT_IMPL(X, Y)
