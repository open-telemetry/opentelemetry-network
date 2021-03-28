#pragma once

#include <platform/types.h>

enum class ServerCommand : u64 {
  NONE,
  DISABLE_SEND = 0xe5c94272c6a3028ful,
};
