/*
 * Copyright The OpenTelemetry Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <util/enum.h>

// from proc(5)
#define ENUM_NAME ProcessState
#define ENUM_TYPE char
#define ENUM_ELEMENTS(X)                                                                                                       \
  X(unknown, 0)                                                                                                                \
  X(uninterruptible_sleep, 'D') /*Waiting in uninterruptible disk sleep*/                                                      \
  X(wake_kill, 'K')             /*Wakekill (Linux 2.6.33 .. 3.13)*/                                                            \
  X(parked, 'P')                /*Parked (Linux 3.9 .. 3.13)*/                                                                 \
  X(running, 'R')               /*Running*/                                                                                    \
  X(interruptible_sleep, 'S')   /*Sleeping in an interruptible wait*/                                                          \
  X(stopped, 'T')               /*Stopped (on a signal) or (Linux < 2.6.33) trace stopped*/                                    \
  X(waking_paging, 'W')         /*Paging (Linux < 2.6.0) / Waking (Linux 2.6.33 .. 3.13)*/                                     \
  X(dead, 'X')                  /*Dead (Linux >= 2.6.0)*/                                                                      \
  X(zombie, 'Z')                /*Zombie*/                                                                                     \
  X(tracing_stop, 't')          /*Tracing stop (Linux >= 2.6.33)*/                                                             \
  X(dead_x, 'x')                /*Dead (Linux 2.6.33 .. 3.13)*/
#define ENUM_DEFAULT unknown
#include <util/enum_operators.inl>
