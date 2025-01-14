#pragma once
#include <limits>
#include <type_traits>

namespace small_vectors::inline v3_3::concepts
  {
template<typename value_type>
concept has_numeric_limits_max = requires {
  { std::numeric_limits<value_type>::max() } -> std::convertible_to<value_type>;
};
  }  // namespace small_vectors::inline v3_3::concepts

