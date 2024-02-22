#pragma once

#include <concepts>
#include <bit>
#include <cstdint>
#include <algorithm>
#include <utils/utility_cxx20.h>

namespace utils
  {
#if defined(__cpp_lib_endian)
using std::endian;
#else
enum struct endian
  {
#ifdef _WIN32
  little = 0,
  big = 1,
  native = little
#else
  little = __ORDER_LITTLE_ENDIAN__,
  big = __ORDER_BIG_ENDIAN__,
  native = __BYTE_ORDER__
#endif
  };
#endif

  }  // namespace utils
