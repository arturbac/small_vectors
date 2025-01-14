#pragma once

#include <set>

namespace std
  {
template<typename Key, typename Compare, typename Alloc>
consteval bool adl_decl_trivially_destructible_after_move(set<Key, Compare, Alloc> const *)
  {
  return true;
  }
  }  // namespace std
