#pragma once

#include <iterator>
#include <type_traits>

namespace small_vectors::inline v3_0::concepts
  {
template<typename iterator_type>
concept iterator_traits_defined = requires {
  typename std::iterator_traits<iterator_type>::value_type;
  typename std::iterator_traits<iterator_type>::difference_type;
  typename std::iterator_traits<iterator_type>::pointer;
  typename std::iterator_traits<iterator_type>::reference;
  typename std::iterator_traits<iterator_type>::iterator_category;
};

  }
