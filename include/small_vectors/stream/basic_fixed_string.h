// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once
#include <small_vectors/basic_fixed_string.h>
#include <iostream>

namespace small_vectors::inline v3_2
  {
template<concepts::integral_or_byte CharType, std::size_t N>
std::basic_ostream<CharType> &
  operator<<(std::basic_ostream<CharType> & os, basic_fixed_string<CharType, N> const & str)
  {
  // Directly use the string view for output. This works with char, wchar_t, etc.
  os << str.view();
  return os;
  }
  }  // namespace small_vectors::inline v3_2
