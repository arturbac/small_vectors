// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once
#include <small_vectors/basic_string.h>
#include <format>

template<typename char_type, uint64_t N, typename T>
  requires std::formattable<std::basic_string_view<char_type>, char_type>
struct std::formatter<small_vectors::basic_string_t<char_type, N, T>, char_type>
  {
  std::formatter<std::basic_string_view<char_type>, char_type> value_formatter;

  template<typename FormatContext>
  constexpr auto parse(FormatContext & ctx)
    {
    return value_formatter.parse(ctx);
    }

  template<typename FormatContext>
  [[nodiscard]]
  auto format(small_vectors::basic_string_t<char_type, N, T> const & value, FormatContext & ctx) const ->
    typename FormatContext::iterator
    {
    std::basic_string_view<char_type> view(value);
    return value_formatter.format(view, ctx);
    }
  };

