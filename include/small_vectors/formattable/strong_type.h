// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once
#include <small_vectors/utils/strong_type.h>
#include <format>

namespace small_vectors::inline v3_0::utils
  {
  }

template<typename value_type, typename tag>
struct std::formatter<small_vectors::utils::strong_type<value_type, tag>>
  {
  std::formatter<value_type> value_formatter;

  template<typename FormatContext>
#if __cplusplus >= 202301L
    requires std::formattable<value_type, typename FormatContext::char_type>
#endif
  constexpr auto parse(FormatContext & ctx)
    {
    return value_formatter.parse(ctx);
    }

  template<typename FormatContext>
#if __cplusplus >= 202301L
    requires std::formattable<value_type, typename FormatContext::char_type>
#endif
  [[nodiscard]]
  auto format(small_vectors::utils::strong_type<value_type, tag> const & strong_value, FormatContext & ctx) const ->
    typename FormatContext::iterator
    {
    return value_formatter.format(strong_value.value(), ctx);
    }
  };
