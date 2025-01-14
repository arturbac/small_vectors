#pragma once

#include <small_vectors/version.h>
#include <concepts>
#include <iterator>

/// \brief Burrows–Wheeler transform
namespace small_vectors::inline v3_3::algo::lower_bound
  {

// Idea from Andrei Alexandrescu on improved lower bound
struct bound_leaning_lower_bound_fn
  {
  template<
    std::random_access_iterator iterator,
    std::sentinel_for<iterator> sentinel,
    typename value_type,
    typename compare_type>
    requires std::
      invocable<compare_type, typename std::iterator_traits<iterator>::value_type const &, value_type const &>
    constexpr auto operator()(iterator first, sentinel last, value_type const & v, compare_type less) const noexcept(
      noexcept(less(*first, v))
    ) -> iterator
    {
    if(first == last)
      return iterator(last);  // Convert sentinel to iterator if they're different types

    iterator middle;
    auto distance = std::distance(first, last);

    if(distance == 1)
      middle = first;
    else
      middle = first + distance / 2;

    if(less(*middle, v))
      {
      for(first = middle + 1; first != last; first = middle + 1)
        {
        distance = std::distance(first, last);
        if(distance == 1)
          middle = first;
        else
          middle = first + 3 * distance / 4;

        if(!less(*middle, v))
          {
          last = middle;
          break;
          }
        }
      }
    else
      {
      for(last = middle; first != last; last = middle)
        {
        distance = std::distance(first, last);
        if(distance == 1)
          middle = first;
        else
          middle = first + distance / 4;

        if(less(*middle, v))
          {
          first = middle + 1;
          break;
          }
        }
      }
    return std::lower_bound(first, iterator(last), v, std::move(less));  // Convert sentinel to iterator if needed
    }
  };

inline constexpr bound_leaning_lower_bound_fn bound_leaning{};
  }  // namespace small_vectors::inline v3_3::algo::lower_bound
