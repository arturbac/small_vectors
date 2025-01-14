#pragma once

#include <unordered_set>

namespace std
  {
template<typename Tp, typename Hash, typename Pred, typename Alloc>
consteval bool adl_decl_trivially_destructible_after_move(unordered_set<Tp, Hash, Pred, Alloc> const *)
  {
  return true;
  }
  }  // namespace std
