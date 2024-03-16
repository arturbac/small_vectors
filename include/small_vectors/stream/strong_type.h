// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once
#include <small_vectors/utils/strong_type.h>
#include <small_vectors/concepts/stream_insertable.h>
#include <iostream>

namespace small_vectors::inline v3_0::utils
  {

template<small_vectors::concepts::stream_insertable ValueType, typename Tag>
std::ostream & operator<<(std::ostream & os, strong_type<ValueType, Tag> const & st)
  {
  os << st.get();
  return os;
  }

  }  // namespace small_vectors::inline v3_0::utils
