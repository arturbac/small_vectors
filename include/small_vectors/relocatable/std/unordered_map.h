#pragma once

#include <unordered_map>

namespace std
  {
template<typename Key, typename Tp, typename Hash, typename Pred, typename Alloc>
consteval bool adl_decl_trivially_destructible_after_move(unordered_map<Key, Tp, Hash, Pred, Alloc> const *)
  {
  return true;
  }
  }  // namespace std
