// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once
#include <small_vectors/basic_fixed_string.h>

namespace std
  {
template<typename CharType, std::size_t N>
struct formatter<small_vectors::v3_0::basic_fixed_string<CharType, N>, CharType>
  {
  formatter<std::basic_string_view<CharType>, CharType> value_formatter;

  template<typename FormatContext>
  constexpr auto parse(FormatContext & ctx)
    {
    return value_formatter.parse(ctx);
    }

  template<typename FormatContext>
  [[nodiscard]]
  auto format(small_vectors::v3_0::basic_fixed_string<CharType, N> const & value, FormatContext & ctx) const ->
    typename FormatContext::iterator
    {
    return value_formatter.format(value.view(), ctx);
    }
  };
  }  // namespace std

