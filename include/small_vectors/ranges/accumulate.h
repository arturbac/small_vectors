#pragma once
#include <small_vectors/utils/static_call_operator.h>
#include <small_vectors/version.h>
#include <concepts>
#include <iterator>
#include <ranges>

namespace small_vectors::inline v3_3::ranges
  {
struct accumulate_t
  {
  template<std::input_iterator source_iterator, std::sentinel_for<source_iterator> sentinel>
  [[nodiscard]]
  small_vector_static_call_operator constexpr auto operator()(
    source_iterator beg, sentinel end, auto init, auto binary_op
  ) small_vector_static_call_operator_const noexcept
    {
    small_vectors_clang_unsafe_buffer_usage_begin  //
      for(; beg != end; ++beg) init
      = binary_op(std::move(init), *beg);
    small_vectors_clang_unsafe_buffer_usage_end  //
      return init;
    }

  template<std::input_iterator source_iterator, std::sentinel_for<source_iterator> sentinel>
  [[nodiscard]]
  small_vector_static_call_operator constexpr auto operator()(source_iterator beg, sentinel end, auto init)
    {
    return operator()(beg, end, std::move(init), std::plus<decltype(init)>{});
    }

  template<std::ranges::input_range input_range>
  [[nodiscard]]
  small_vector_static_call_operator constexpr auto
    operator()(input_range const & range, auto init, auto binary_op) small_vector_static_call_operator_const noexcept
    {
    return operator()(std::ranges::begin(range), std::ranges::end(range), std::move(init), binary_op);
    }

  template<std::ranges::input_range input_range>
  [[nodiscard]]
  small_vector_static_call_operator constexpr auto
    operator()(input_range const & range, auto init) small_vector_static_call_operator_const noexcept
    {
    return operator()(range, std::move(init), std::plus<decltype(init)>{});
    }
  };

inline constexpr accumulate_t accumulate;
  }  // namespace small_vectors::inline v3_3::ranges
