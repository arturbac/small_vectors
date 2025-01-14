#pragma once

#include <small_vectors/version.h>
#include <concepts>
#include <cstddef>

namespace small_vectors::inline v3_3::concepts
  {
template<typename value_type>
concept integral_or_byte = std::integral<value_type> || std::is_same_v<value_type, std::byte>;
  }
