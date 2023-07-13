#pragma once
#include <concepts>
#include <iterator>
#include <ranges>

namespace coll::ranges
{
  
  struct accumulate_t
    {
    template<std::input_iterator source_iterator, std::sentinel_for<source_iterator> sentinel>
    small_vector_cpp_static_call_operator
    constexpr auto operator()( source_iterator beg, sentinel end, auto init, auto binary_op)
        small_vector_static_call_operator_const noexcept
      {
      for (;beg != end; ++beg)
        init = binary_op(std::move(init), *beg);
      return init;
      }
    template<std::ranges::input_range input_range>
    small_vector_cpp_static_call_operator
    constexpr auto operator()( input_range range, auto init, auto binary_op )
        small_vector_static_call_operator_const noexcept
      {
      return operator()(std::ranges::begin(range), std::ranges::end(range), std::move(init), binary_op );
      }
    };
  inline constexpr accumulate_t accumulate;
}
