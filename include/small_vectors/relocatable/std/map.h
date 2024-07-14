#pragma once

#include <map>

namespace std
  {
template<typename Key, typename Tp, typename Compare, typename Alloc>
consteval bool adl_decl_relocatable(map<Key, Tp, Compare, Alloc> const *)
  {
  return true;
  }
  }  // namespace std

