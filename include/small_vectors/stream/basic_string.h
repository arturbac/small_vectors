// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once
#include <small_vectors/basic_string.h>
#include <iostream>

namespace small_vectors::inline v3_0
  {
template<typename char_type, uint64_t N, typename T>
std::ostream & operator<<(std::ostream & os, basic_string_t<char_type, N, T> const & str)
  {
  std::basic_string_view<char_type> view(str);
  os << view;
  return os;
  }
  }  // namespace small_vectors::inline v3_0
