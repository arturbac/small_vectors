#pragma once
#include <type_traits>
#include <concepts>
#include <cstdint>
#include <iterator>

namespace coll::concepts
  {
using std::convertible_to;

template<uint32_t sz>
concept even_size = (sz & 1) == 0 && sz != 0;

template<typename iterator>
using iterator_category_t = typename std::iterator_traits<iterator>::iterator_category;

template<typename iterator>
concept forward_iterator = std::forward_iterator<iterator>;

template<typename iterator>
concept input_iterator = std::input_iterator<iterator>;

template<typename iterator>
concept output_iterator = convertible_to<iterator_category_t<iterator>, std::input_iterator_tag>;

template<typename iterator_type>
concept iterator = std::input_or_output_iterator<iterator_type>;

template<typename iterator>
concept random_access_iterator = convertible_to<iterator_category_t<iterator>, std::random_access_iterator_tag>;

template<typename value_type>
concept vector_constraints = (std::movable<value_type> or std::copyable<value_type>)and std::destructible<value_type>;

template<typename value_type>
concept nothrow_move_constructible = std::is_nothrow_move_constructible_v<value_type>;

template<typename value_type>
concept unsigned_arithmetic_integral = requires {
  requires not std::same_as<std::remove_cv_t<value_type>, bool>;
  requires std::unsigned_integral<value_type>;
};

template<typename value_type>
concept allocate_constraint = requires {
  requires sizeof(value_type) != 0;
  requires sizeof(value_type) % alignof(value_type) == 0u;
};
  }  // namespace coll::concepts

namespace coll::detail
  {
template<class T, class... Args>
inline constexpr bool is_nothrow_move_constr_and_constr_v
  = std::is_nothrow_constructible_v<T, Args...> && std::is_nothrow_move_constructible_v<T>;

template<class T, class... Args>
inline constexpr bool is_nothrow_copy_constr_and_constr_v
  = std::is_nothrow_constructible_v<T, Args...> && std::is_nothrow_copy_constructible_v<T>;
  }  // namespace coll::detail
