#pragma once
#include <small_vectors/version.h>
#include <concepts>
#include <type_traits>
#include <cstdint>
#include <iterator>

namespace small_vectors::inline v3_2::concepts
  {
using std::convertible_to;

template<uint32_t sz>
concept even_size = (sz & 1) == 0 && sz != 0;

template<typename iterator>
using iterator_category_t = typename std::iterator_traits<iterator>::iterator_category;

template<typename iter_t>
inline constexpr bool is_contiguous_iterator_v = std::contiguous_iterator<iter_t>;

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
concept vector_constraints = (std::movable<value_type> or std::copyable<value_type>) and std::destructible<value_type>;

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
template<typename value_type>
concept trivially_copyable = std::is_trivially_copyable_v<value_type>;

template<class T>
concept trivial = std::is_trivial_v<T>;

template<class T, class... Args>
inline constexpr bool is_nothrow_move_constr_and_constr_v
  = std::is_nothrow_constructible_v<T, Args...> && std::is_nothrow_move_constructible_v<T>;

template<class T, class... Args>
inline constexpr bool is_nothrow_copy_constr_and_constr_v
  = std::is_nothrow_constructible_v<T, Args...> && std::is_nothrow_copy_constructible_v<T>;

///\brief explicit declared relocation capable
template<typename T>
concept explicit_relocatable = requires(T const * value) {
  { adl_decl_relocatable(value) } -> std::same_as<bool>;
  requires adl_decl_relocatable(static_cast<T const *>(nullptr));
};

template<typename T>
concept relocatable = explicit_relocatable<T> || std::is_trivially_destructible_v<T>;

template<typename T, typename... Args>
concept same_as_any_of = std::disjunction_v<std::is_same<T, Args>...>;

template<typename T, typename... Args>
concept all_same = std::conjunction_v<std::is_same<T, Args>...>;

template<typename T>
concept prefix_incrementable = requires(T value) {
  { ++value } -> std::same_as<T &>;
};

template<typename T>
concept postfix_incrementable = requires(T value) {
  { ++value } -> std::same_as<T &>;
};

template<typename T>
concept prefix_decrementable = requires(T value) {
  { --value } -> std::same_as<T &>;
};

template<typename T>
concept addable = requires(T a, T b) {
  { a + b } -> std::convertible_to<T>;
};

template<typename T>
concept substractable = requires(T a, T b) {
  { a - b } -> std::convertible_to<T>;
};

template<typename T>
concept multiplicatable = requires(T a, T b) {
  { a * b } -> std::convertible_to<T>;
};
template<typename T, typename U>
concept multiplicatable_with = requires(T a, U b) {
  { a * b } -> std::convertible_to<T>;
};

template<typename T>
concept dividable = requires(T a, T b) {
  { a / b } -> std::convertible_to<T>;
};
template<typename T>
concept has_modulo_operator = requires(T a, T b) {
  { a % b } -> std::convertible_to<T>;
};

template<typename T>
concept has_bitwise_xor_operator = requires(T a, T b) {
  { a ^ b } -> std::convertible_to<T>;
};

template<typename T>
concept has_bitwise_not_operator = requires(T a) {
  { ~a } -> std::convertible_to<T>;
};

template<typename T>
concept has_left_shift_operator = requires(T obj, T obj2) {
  { obj << obj2 } -> std::convertible_to<T>;
};

template<typename T, typename U>
concept has_left_shift_operator_with = requires(T obj, U obj2) {
  { obj << obj2 } -> std::convertible_to<T>;
};

template<typename T, typename U>
concept has_left_shift_operator_with_integral = requires(T t, U u) {
  requires std::integral<U>;
    { t << u } -> std::convertible_to<T>;
};

template<typename T>
concept has_right_shift_operator = requires(T obj, T obj2) {
  { obj >> obj2 } -> std::convertible_to<T>;
};

template<typename T, typename U>
concept has_right_shift_operator_with = requires(T obj, U obj2) {
  { obj >> obj2 } -> std::convertible_to<T>;
};

template<typename T, typename U>
concept has_right_shift_operator_with_integral = requires(T t, U u) {
  requires std::integral<U>;
    { t >> u } -> std::convertible_to<T>;
};

template<typename T>
concept has_bitwise_and_operator = requires(T a, T b) {
  { a & b } -> std::convertible_to<T>;
};

template<typename T, typename U>
concept has_bitwise_and_operator_with = requires(T a, U b) {
  { a & b } -> std::convertible_to<T>;
};

template<typename T>
concept has_bitwise_or_operator = requires(T a, T b) {
  { a | b } -> std::convertible_to<T>;
};

template<typename T>
concept addable_assign = requires(T a, T const & b) {
  { a += b } -> same_as_any_of<T &, T const &>;
};

template<typename T>
concept subtractable_assign = requires(T a, T const & b) {
  { a -= b } -> same_as_any_of<T &, T const &>;
};
template<typename T>
concept multiplicatable_assign = requires(T a, T const & b) {
  { a *= b } -> same_as_any_of<T &, T const &>;
};
template<typename T>
concept dividable_assign = requires(T a, T const & b) {
  { a /= b } -> same_as_any_of<T &, T const &>;
};

template<typename T>
concept has_modulo_assign = requires(T a, T const & b) {
  { a %= b } -> same_as_any_of<T &, T const &>;
};

template<typename T>
concept has_bitwise_xor_assign = requires(T a, T const & b) {
  { a ^= b } -> same_as_any_of<T &, T const &>;
};

template<typename T>
concept has_left_shift_assign = requires(T a, T const & b) {
  { a <<= b } -> same_as_any_of<T &, T const &>;
};

template<typename T>
concept has_right_shift_assign = requires(T a, T const & b) {
  { a >>= b } -> same_as_any_of<T &, T const &>;
};

template<typename T>
concept has_bitwise_and_assign = requires(T a, T const & b) {
  { a &= b } -> same_as_any_of<T &, T const &>;
};

template<typename T>
concept has_bitwise_or_assign = requires(T a, T const & b) {
  { a |= b } -> same_as_any_of<T &, T const &>;
};
  }  // namespace small_vectors::inline v3_2::concepts
