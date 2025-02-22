#pragma once
#include <small_vectors/concepts/concepts.h>

namespace small_vectors::inline v3_3
  {
struct trivially_relocatable
  {
  };

struct not_trivially_relocatable
  {
  ~not_trivially_relocatable() {}
  };

template<typename T>
using conditional_trivial_reloc_base
  = std::conditional_t<concepts::is_trivially_relocatable<T>, trivially_relocatable, not_trivially_relocatable>;
  }  // namespace small_vectors::inline v3_3
