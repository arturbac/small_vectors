#pragma once
#include <small_vectors/version.h>
#include <small_vectors/utils/utility_cxx20.h>
#include <small_vectors/concepts/concepts.h>
#include <memory>
#include <cstring>
#include <algorithm>

namespace small_vectors::inline v3_3::detail
  {
template<typename iterator_type>
using iterator_value_type_t = std::iter_value_t<iterator_type>;

///\return distance for growing valid range of iterator pair
template<concepts::unsigned_arithmetic_integral size_type, typename iterator>
inline constexpr auto udistance(iterator first, iterator last) noexcept
  {
  return static_cast<size_type>(std::distance(first, last));
  }

template<concepts::unsigned_arithmetic_integral size_type, typename iterator>
  requires(not concepts::unsigned_arithmetic_integral<iterator>)
inline constexpr auto unext(iterator ptr, size_type offset) noexcept
  {
  return std::next(ptr, static_cast<ptrdiff_t>(offset));
  }

template<concepts::unsigned_arithmetic_integral size_type, typename iterator>
  requires(not concepts::unsigned_arithmetic_integral<iterator>)
inline constexpr auto uprev(iterator ptr, size_type offset) noexcept
  {
  return std::prev(ptr, static_cast<ptrdiff_t>(offset));
  }

///\brief no implicit convertion sum
template<concepts::unsigned_arithmetic_integral size_type, concepts::unsigned_arithmetic_integral... Args>
inline constexpr auto nic_sum(size_type first, Args... args) noexcept -> size_type
  {
  if constexpr(sizeof...(Args) != 0u)
    return static_cast<size_type>(first + nic_sum(args...));
  return static_cast<size_type>(first);
  }

///\brief no implicit convertion sub
template<concepts::unsigned_arithmetic_integral size_type, concepts::unsigned_arithmetic_integral osize_type>
inline constexpr auto nic_sub(size_type first, osize_type second) noexcept -> size_type
  {
  return static_cast<size_type>(first - second);
  }

template<concepts::iterator InputIterator, typename size_type>
  requires requires {
    requires std::is_nothrow_destructible_v<typename std::iterator_traits<InputIterator>::value_type>;
    requires concepts::unsigned_arithmetic_integral<size_type>;
    requires not std::is_trivially_destructible_v<typename std::iterator_traits<InputIterator>::value_type>;
  }
inline constexpr void destroy_range(InputIterator data, size_type first, size_type last) noexcept
  {
  std::destroy(unext(data, first), unext(data, last));
  }

///\brief used for unwinding already moved/copied range after exeption is throw in used move/copy operator
template<bool use_nothrow, typename InputIt>
struct range_unwinder
  {
  using value_type = typename std::iterator_traits<InputIt>::value_type;

  InputIt first_res_, last_;

  constexpr range_unwinder(InputIt first_res) noexcept : first_res_{first_res}, last_{first_res} {}

  constexpr void release()
    {
    if constexpr(use_nothrow || !std::is_trivially_destructible_v<value_type>)
      first_res_ = last_;
    }

  constexpr ~range_unwinder()
    {
    if constexpr(use_nothrow || !std::is_trivially_destructible_v<value_type>)
      if(first_res_ != last_)
        std::destroy(first_res_, last_);
    }
  };

template<typename value_type>
constexpr void
  uninitialized_default_construct(value_type * store) noexcept(std::is_nothrow_default_constructible_v<value_type>)
  {
  if(std::is_constant_evaluated())
    std::construct_at(store);
  else
    ::new(store) value_type;
  }

template<typename value_type>
constexpr void
  uninitialized_value_construct(value_type * store) noexcept(std::is_nothrow_default_constructible_v<value_type>)
  {
  std::construct_at(store);
  }

template<typename iterator>
inline constexpr void uninitialized_default_construct(
  iterator first, iterator last
) noexcept(std::is_nothrow_constructible_v<iterator_value_type_t<iterator>>)
  {
  if(std::is_constant_evaluated())
    {
    small_vectors_clang_unsafe_buffer_usage_begin  //
      for(; first != last; ++first) std::construct_at(std::addressof(*first));
    small_vectors_clang_unsafe_buffer_usage_end  //
    }
  else
    std::uninitialized_default_construct(first, last);
  }

template<typename iterator, typename size_type>
inline constexpr void uninitialized_value_construct_n(
  iterator first, size_type count
) noexcept(std::is_nothrow_constructible_v<iterator_value_type_t<iterator>>)
  {
  if(std::is_constant_evaluated())
    {
    small_vectors_clang_unsafe_buffer_usage_begin  //
      for(size_type ix{}; ix != count; ++ix) std::construct_at(std::addressof(first[ix]));
    small_vectors_clang_unsafe_buffer_usage_end  //
    }
  else
    std::uninitialized_value_construct_n(first, count);
  }

template<typename iterator>
inline constexpr auto uninitialized_value_construct(
  iterator first, iterator last
) noexcept(std::is_nothrow_constructible_v<iterator_value_type_t<iterator>>)
  {
  if(std::is_constant_evaluated())
    {
    small_vectors_clang_unsafe_buffer_usage_begin  //
      for(; first != last; ++first) std::construct_at(std::addressof(*first));
    small_vectors_clang_unsafe_buffer_usage_end  //
    }
  else
    std::uninitialized_value_construct(first, last);
  return last;
  }

template<typename iterator>
inline constexpr auto uninitialized_fill(
  iterator first, iterator last, iterator_value_type_t<iterator> fill_value
) noexcept(std::is_nothrow_constructible_v<iterator_value_type_t<iterator>>) -> iterator
  {
  constexpr bool use_nothrow = std::is_nothrow_constructible_v<iterator_value_type_t<iterator>>;
  using unwind = range_unwinder<use_nothrow, iterator>;
  unwind cur{first};
  small_vectors_clang_unsafe_buffer_usage_begin  //
    for(; cur.last_ != last; (void)++cur.last_) std::construct_at(std::addressof(*cur.last_), fill_value);
  small_vectors_clang_unsafe_buffer_usage_end  //
    cur.release();
  return cur.last_;
  }

template<typename InputIterator>
inline constexpr decltype(auto) deref_iter(InputIterator iter) noexcept
  {
  return *iter;
  }

template<typename InputIterator>
inline constexpr auto && deref_iter(std::move_iterator<InputIterator> iter) noexcept
  {
  return std::move(*iter);
  }

template<typename Iter>
concept contiguous_iterator_with_trivialy_copy_constructible
  = std::contiguous_iterator<Iter> && std::is_trivially_copy_constructible_v<std::iter_value_t<Iter>>;

// -------------------------------
// -- uninitialized_copy --

template<
  contiguous_iterator_with_trivialy_copy_constructible InputIterator,
  std::integral Size,
  contiguous_iterator_with_trivialy_copy_constructible OutputIterator>
inline auto uninitialized_copy_n_impl(InputIterator first, Size count, OutputIterator out)
  {
  // static constexpr auto elem_size{sizeof(std::iter_value_t<InputIterator>)};
  return std::uninitialized_copy_n(first, count, out);
  // std::memcpy(std::addressof(*out), std::addressof(*first), elem_size * std::size_t(count));
  // return std::next(out, std::ptrdiff_t(count));
  }

template<concepts::input_iterator InputIterator, std::integral Size, concepts::forward_iterator ForwardIterator>
inline auto uninitialized_copy_n_impl(InputIterator first, Size count, ForwardIterator result)
  {
  static_assert(!(
    contiguous_iterator_with_trivialy_copy_constructible<InputIterator>
    && contiguous_iterator_with_trivialy_copy_constructible<ForwardIterator>
  ));
  constexpr bool use_nothrow = std::is_nothrow_constructible_v<iterator_value_type_t<InputIterator>>;
  using unwind = range_unwinder<use_nothrow, ForwardIterator>;
  unwind cur{result};
  small_vectors_clang_unsafe_buffer_usage_begin  //
    for(; count > 0; --count, (void)++first, ++cur.last_) std::construct_at(std::addressof(*cur.last_), *first);
  small_vectors_clang_unsafe_buffer_usage_end  //
    cur.release();
  return cur.last_;
  }

template<
  contiguous_iterator_with_trivialy_copy_constructible InputIterator,
  contiguous_iterator_with_trivialy_copy_constructible OutputIterator>
inline auto uninitialized_copy_impl(InputIterator first, InputIterator last, OutputIterator out)
  {
  std::size_t const number_of_elems{std::size_t(last - first)};
  return uninitialized_copy_n_impl(first, number_of_elems, out);
  }

template<concepts::input_iterator InputIterator, concepts::forward_iterator ForwardIterator>
inline auto uninitialized_copy_impl(InputIterator first, InputIterator last, ForwardIterator result)
  {
  static_assert(!(
    contiguous_iterator_with_trivialy_copy_constructible<InputIterator>
    && contiguous_iterator_with_trivialy_copy_constructible<ForwardIterator>
  ));
  constexpr bool use_nothrow = std::is_nothrow_constructible_v<iterator_value_type_t<InputIterator>>;
  using unwind = range_unwinder<use_nothrow, ForwardIterator>;
  unwind cur{result};
  small_vectors_clang_unsafe_buffer_usage_begin  //
    for(; first != last; ++first, (void)++cur.last_) std::construct_at(std::addressof(*cur.last_), *first);
  small_vectors_clang_unsafe_buffer_usage_end  //
    cur.release();
  return cur.last_;
  }

template<concepts::input_iterator InputIterator, concepts::forward_iterator ForwardIterator>
inline constexpr auto uninitialized_copy(
  InputIterator first, InputIterator last, ForwardIterator result
) noexcept(std::is_nothrow_constructible_v<iterator_value_type_t<InputIterator>>) -> ForwardIterator
  {
  if(std::is_constant_evaluated())
    {
    small_vectors_clang_unsafe_buffer_usage_begin  //
      for(; first != last; ++first, (void)++result) std::construct_at(std::addressof(*result), *first);
    small_vectors_clang_unsafe_buffer_usage_end  //
      return result;
    }
  else
    return uninitialized_copy_impl(first, last, result);
  }

template<concepts::input_iterator InputIterator, std::integral Size, concepts::forward_iterator ForwardIterator>
inline constexpr void uninitialized_copy_n(
  InputIterator first, Size count, ForwardIterator result
) noexcept(std::is_nothrow_constructible_v<iterator_value_type_t<InputIterator>>)
  {
  if(std::is_constant_evaluated())
    {
    small_vectors_clang_unsafe_buffer_usage_begin  //
      for(; count > 0; --count, (void)++first, ++result) std::construct_at(std::addressof(*result), *first);
    small_vectors_clang_unsafe_buffer_usage_end  //
    }
  else
    uninitialized_copy_n_impl(first, count, result);
  }

// -------------------------------
// -- uninitialized_move --

template<typename Iter>
concept contiguous_iterator_with_trivialy_move_constructible
  = std::contiguous_iterator<Iter> && std::is_trivially_move_constructible_v<std::iter_value_t<Iter>>;

template<
  contiguous_iterator_with_trivialy_move_constructible InputIterator,
  contiguous_iterator_with_trivialy_move_constructible OutputIterator>
inline auto uninitialized_move_impl(InputIterator first, InputIterator last, OutputIterator out)
  {
  std::size_t const number_of_elems{std::size_t(last - first)};
  return uninitialized_copy_n_impl(first, number_of_elems, out);
  }

template<concepts::input_iterator InputIterator, concepts::forward_iterator ForwardIterator>
inline auto uninitialized_move_impl(InputIterator first, InputIterator last, ForwardIterator result)
  {
  static_assert(!(
    contiguous_iterator_with_trivialy_move_constructible<InputIterator>
    && contiguous_iterator_with_trivialy_move_constructible<ForwardIterator>
  ));
  constexpr bool use_nothrow = std::is_nothrow_constructible_v<iterator_value_type_t<InputIterator>>;
  using unwind = range_unwinder<use_nothrow, ForwardIterator>;
  unwind cur{result};
  small_vectors_clang_unsafe_buffer_usage_begin  //
    for(; first != last; ++first, (void)++cur.last_) std::construct_at(std::addressof(*cur.last_), std::move(*first));
  small_vectors_clang_unsafe_buffer_usage_end  //
    cur.release();
  return cur.last_;
  }

template<
  contiguous_iterator_with_trivialy_move_constructible InputIterator,
  std::integral Size,
  contiguous_iterator_with_trivialy_move_constructible OutputIterator>
inline auto uninitialized_move_n_impl(InputIterator first, Size count, OutputIterator out)
  {
  return uninitialized_copy_n_impl(first, count, out);
  }

template<concepts::input_iterator InputIterator, std::integral Size, concepts::forward_iterator ForwardIterator>
auto uninitialized_move_n_impl(InputIterator first, Size count, ForwardIterator result)
  {
  static_assert(!(
    contiguous_iterator_with_trivialy_move_constructible<InputIterator>
    && contiguous_iterator_with_trivialy_move_constructible<ForwardIterator>
  ));
  constexpr bool use_nothrow = std::is_nothrow_constructible_v<iterator_value_type_t<InputIterator>>;
  using unwind = range_unwinder<use_nothrow, ForwardIterator>;
  unwind cur{result};
  small_vectors_clang_unsafe_buffer_usage_begin  //
    for(; count > 0; --count, (void)++first, ++cur.last_)
      std::construct_at(std::addressof(*cur.last_), std::move(*first));
  small_vectors_clang_unsafe_buffer_usage_end  //
    cur.release();
  return cur.last_;
  }

template<concepts::input_iterator InputIterator, concepts::forward_iterator ForwardIterator>
inline constexpr void uninitialized_move(
  InputIterator first, InputIterator last, ForwardIterator result
) noexcept(std::is_nothrow_move_constructible_v<iterator_value_type_t<InputIterator>>)
  {
  if(std::is_constant_evaluated())
    for(; first != last; ++first, (void)++result)
      std::construct_at(std::addressof(result), std::move(*first));
  else
    uninitialized_move_impl(first, last, result);
  }

template<concepts::input_iterator InputIterator, std::integral Size, concepts::forward_iterator ForwardIterator>
inline constexpr void uninitialized_move_n(
  InputIterator first, Size count, ForwardIterator result
) noexcept(std::is_nothrow_move_constructible_v<iterator_value_type_t<InputIterator>>)
  {
  if(std::is_constant_evaluated())
    {
    small_vectors_clang_unsafe_buffer_usage_begin  //
      for(; count > 0; --count, (void)++first, ++result) std::construct_at(std::addressof(*result), std::move(*first));
    small_vectors_clang_unsafe_buffer_usage_end  //
    }
  else
    uninitialized_move_n_impl(first, count, result);
  }

template<typename iterator>
inline constexpr bool move_if_noexcept_cond
  = std::is_nothrow_move_constructible_v<iterator> || !std::is_copy_constructible_v<iterator>;

template<typename iterator>
inline constexpr auto make_move_if_noexcept_iterator(iterator it)
  {
  using value_type = typename std::iterator_traits<iterator>::value_type;
  using return_type = std::conditional_t<move_if_noexcept_cond<value_type>, std::move_iterator<iterator>, iterator>;
  return return_type(it);
  }

template<typename value_type>
inline constexpr auto make_move_if_noexcept_iterator(value_type * it)
  {
  using return_type
    = std::conditional_t<move_if_noexcept_cond<value_type>, std::move_iterator<value_type *>, value_type const *>;
  return return_type(it);
  }

// for strong exception guarantee
// https://en.cppreference.com/w/cpp/utility/move_if_noexcept
template<concepts::input_iterator InputIterator, std::integral size_type, concepts::forward_iterator ForwardIterator>
inline constexpr void uninitialized_move_if_noexcept_n(
  InputIterator first, size_type count, ForwardIterator result
) noexcept(std::is_nothrow_move_constructible_v<iterator_value_type_t<InputIterator>>)
  {
  constexpr bool use_nothrow = std::is_nothrow_move_constructible_v<iterator_value_type_t<InputIterator>>;
  if constexpr(use_nothrow)
    uninitialized_move_n(first, count, result);
  else
    uninitialized_copy_n(first, count, result);
  }

// -------------------------------
// -- uninitialized_relocate --

template<concepts::iterator InputIterator, std::integral size_type, concepts::forward_iterator ForwardIterator>
  requires(true == std::is_nothrow_move_constructible_v<iterator_value_type_t<InputIterator>>)
inline constexpr void uninitialized_relocate_n(InputIterator first, size_type count, ForwardIterator result) noexcept
  {
  using value_type = iterator_value_type_t<InputIterator>;
  uninitialized_move_n(first, count, result);
  if constexpr(!concepts::trivially_destructible_after_move<value_type>)
    destroy_range(first, size_type(0u), count);
  }

template<concepts::iterator InputIterator, std::integral size_type, concepts::forward_iterator ForwardIterator>
inline constexpr void uninitialized_relocate_if_noexcept_n(
  InputIterator first, size_type count, ForwardIterator result
) noexcept(std::is_nothrow_move_constructible_v<iterator_value_type_t<InputIterator>>)
  {
  using value_type = iterator_value_type_t<InputIterator>;

  constexpr bool use_nothrow = std::is_nothrow_move_constructible_v<iterator_value_type_t<InputIterator>>;
  if constexpr(use_nothrow)
    uninitialized_relocate_n(first, count, result);
  else
    {
    uninitialized_copy_n(first, count, result);
    if constexpr(!std::is_trivially_destructible_v<value_type>)
      destroy_range(first, size_type(0u), count);
    }
  }

template<concepts::iterator InputIterator, std::integral size_type, concepts::forward_iterator ForwardIterator>
inline constexpr void uninitialized_relocate_with_copy_n(
  InputIterator first, size_type count, ForwardIterator result
) noexcept(std::is_nothrow_copy_constructible_v<iterator_value_type_t<InputIterator>>)
  {
  using value_type = iterator_value_type_t<InputIterator>;
  uninitialized_copy_n(first, count, result);
  if constexpr(!std::is_trivially_destructible_v<value_type>)
    destroy_range(first, size_type(0u), count);
  }

template<concepts::forward_iterator iterator, std::integral size_type>
inline constexpr void uninitialized_uneven_range_swap(
  iterator iter1, size_type size1, iterator iter2, size_type size2
) noexcept(std::is_nothrow_move_constructible_v<iterator_value_type_t<iterator>>)
  {
  if(size1 > size2)
    {
    std::swap(iter1, iter2);
    std::swap(size1, size2);
    }
  small_vectors_clang_unsafe_buffer_usage_begin  //
    std::swap_ranges(iter1, iter1 + size1, iter2);
  uninitialized_relocate_if_noexcept_n(iter2 + size1, size_type(size2 - size1), iter1 + size1);
  small_vectors_clang_unsafe_buffer_usage_end  //
  }
  }  // namespace small_vectors::inline v3_3::detail

