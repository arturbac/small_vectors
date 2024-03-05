#pragma once

#include <concepts>
#include <small_vectors/utils/utility_cxx20.h>

#include <cstdint>
#include <type_traits>
#include <string_view>

namespace small_vectors::inline v3_0::utils
  {
namespace internal
  {
  [[nodiscard]]
  constexpr bool enum_name_view_compare(std::string_view a, std::string_view b) noexcept
    {
    return a.size() == b.size() && a == b;
    }
  }  // namespace internal

namespace detail
  {
  template<typename type>
  concept enum_concept = std::is_enum<type>::value;
  }

///\brief tests if \ref value has set flag \ref test_flag
template<detail::enum_concept enum_type>
[[nodiscard]]
inline constexpr bool enum_test_flag(enum_type value, enum_type test_flag) noexcept
  {
  return 0 != (cxx23::to_underlying(value) & cxx23::to_underlying(test_flag));
  }

///\returns conditionaly set flag \param cond_e depending on \param cond otherwise returns {}
template<detail::enum_concept enum_type>
[[nodiscard]]
inline constexpr enum_type enum_cond_flag(bool cond, enum_type cond_e) noexcept
  {
  if(cond)
    return cond_e;
  else
    return {};
  }
  }  // namespace small_vectors::inline v3_0::utils
