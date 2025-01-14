#pragma once

#include <vector>

namespace std
  {
template<typename Tp, typename Alloc>
consteval bool adl_decl_trivially_destructible_after_move(vector<Tp, Alloc> const *)
  {
  return true;
  }
  }  // namespace std
