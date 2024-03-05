#pragma once
#include <concepts>
#include <functional>

namespace small_vectors::inline v3_0::concepts
  {
template<typename value_type>
concept hashable = requires(value_type value) {
  {
  std::hash<value_type>{}(value)
  } -> std::convertible_to<std::size_t>;
};
  }
