#pragma once

#include <small_vectors/version.h>
#include <iterator>
#include <type_traits>

namespace small_vectors::inline v3_3::concepts
  {
template<typename iterator_type>
concept iterator_traits_defined = requires {
  typename std::iterator_traits<iterator_type>::value_type;
  typename std::iterator_traits<iterator_type>::difference_type;
  typename std::iterator_traits<iterator_type>::pointer;
  typename std::iterator_traits<iterator_type>::reference;
  typename std::iterator_traits<iterator_type>::iterator_category;
};

  }  // namespace small_vectors::inline v3_3::concepts
