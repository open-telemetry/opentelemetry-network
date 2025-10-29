// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

extern "C" int otn_kernel_collector_main(int argc, const char **argv);

int main(int argc, char *argv[])
{
  return otn_kernel_collector_main(argc, const_cast<const char **>(argv));
}
