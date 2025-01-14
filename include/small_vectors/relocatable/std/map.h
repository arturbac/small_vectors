#pragma once

#include <map>

namespace std
  {
template<typename Key, typename Tp, typename Compare, typename Alloc>
consteval bool adl_decl_trivially_destructible_after_move(map<Key, Tp, Compare, Alloc> const *)
  {
  return true;
  }
  }  // namespace std

