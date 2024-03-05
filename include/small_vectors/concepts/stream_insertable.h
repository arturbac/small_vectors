#pragma once
#include <concepts>
#include <iosfwd>

namespace small_vectors::inline v3_0::concepts
  {
template<typename value_type>
concept stream_insertable = requires(std::ostream & os, value_type const & value) {
  {
  os << value
  } -> std::same_as<std::ostream &>;
};
  }
