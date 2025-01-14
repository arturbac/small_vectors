#pragma once

#include <deque>

namespace std
  {
template<typename Tp, typename Alloc>
consteval bool adl_decl_trivially_destructible_after_move(deque<Tp, Alloc> const *)
  {
  return true;
  }
  }  // namespace std

