/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

// Regexp patterns for kernel blacklist
// failures must match all specified uname components
// components specified as NULL are not matched

// Pattern specification:
// { sysname, nodename, release, version, machine }

// minikube 4.15.0
{"Linux", "minikube", R"(^4\.15\.0.*)", NULL, NULL},

    // Linux 4.19.17 from Ubuntu mainline repo
    {"Linux", NULL, R"(^4\.19\.57.*)", NULL, NULL},

// Linux 5.1.16 from Ubuntu mainline repo
{
  "Linux", NULL, R"(^5\.1\.16.*)", NULL, NULL
}
