#pragma once
#include <concepts>
#include <iterator>
#include <ranges>

namespace coll::ranges
{
  
  struct accumulate_t
    {
    template<std::input_iterator source_iterator, std::sentinel_for<source_iterator> sentinel>
    static constexpr auto operator()( source_iterator beg, sentinel end, auto init, auto binary_op)
      {
      for (;beg != end; ++beg)
        init = binary_op(std::move(init), *beg);
      return init;
      }
    template<std::ranges::input_range input_range>
    static constexpr auto operator()( input_range range, auto init, auto binary_op )
      {
      return operator()(std::ranges::begin(range), std::ranges::end(range), std::move(init), binary_op );
      }
    };
  inline constexpr accumulate_t accumulate;
}
