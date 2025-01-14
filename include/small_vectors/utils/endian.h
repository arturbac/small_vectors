#pragma once
#include <small_vectors/utils/utility_cxx20.h>
#include <concepts>
#include <bit>
#include <cstdint>
#include <algorithm>

namespace small_vectors::inline v3_3::utils
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

  }  // namespace small_vectors::inline v3_3::utils
