#pragma once
//  The MIT License (MIT)
//
// Copyright © 2022 Artur Bać
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the “Software”), to deal in the Software without
// restriction, including without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <small_vectors/detail/safe_buffers.h>
#include <small_vectors/detail/uninitialized_constexpr.h>
#include <small_vectors/detail/vector_storage.h>
#include <small_vectors/detail/vector_func.h>
#include <small_vectors/detail/adapter_iterator.h>

#include <span>
#include <cassert>

namespace small_vectors::inline v3_3::detail
  {
template<typename value_type, typename size_type>
inline constexpr void sv_deallocate(value_type * p, size_type count) noexcept;
  }  // namespace small_vectors::inline v3_3::detail

namespace small_vectors::inline v3_3
  {
using detail::vector_outcome_e;

using detail::union_min_number_of_elements;
using detail::vector_tune_e;

template<typename V, std::unsigned_integral S, uint64_t N = union_min_number_of_elements<V, S>()>
struct small_vector
  {
  using value_type = V;
  using reference = value_type &;
  using const_reference = value_type const &;
  using size_type = S;
  using iterator = detail::adapter_iterator<value_type *>;
  using const_iterator = detail::adapter_iterator<value_type const *>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using storage_type = detail::small_vector_storage<V, S, N>;
  using dynamic_storage_type = typename storage_type::dynamic_storage_type;
  using enum detail::small_vector_storage_type;
  using enum detail::vector_outcome_e;

  static constexpr bool support_reallocation() noexcept { return true; }

  storage_type storage_;

  [[nodiscard]]
  inline constexpr auto capacity() const noexcept -> size_type
    {
    return storage_.capacity();
    }

  [[nodiscard]]
  static inline constexpr auto buffered_capacity() noexcept -> size_type
    {
    return storage_type::buffered_capacity;
    }

  [[nodiscard]]
  inline constexpr auto data() const noexcept -> value_type const *
    {
    return storage_.data();
    }

  [[nodiscard]]
  inline constexpr auto data() noexcept -> value_type *
    {
    return storage_.data();
    }

  [[nodiscard]]
  inline constexpr auto size() const noexcept -> size_type
    {
    return storage_.size_;
    }

  [[nodiscard]]
  static inline constexpr auto max_size() noexcept -> size_type
    {
    return std::numeric_limits<size_type>::max();
    }

  [[nodiscard]]
  inline constexpr auto active_storage() const noexcept -> detail::small_vector_storage_type
    {
    return storage_.active_storage();
    }

  constexpr operator std::span<value_type const>() const noexcept { return std::span{data(), size()}; }

  constexpr operator std::span<value_type>() noexcept { return std::span{data(), size()}; }

  inline constexpr small_vector() noexcept = default;

  constexpr explicit small_vector(size_type count) noexcept(noexcept(detail::resize(*this, count)))
    {
    detail::resize(*this, count);
    }

  template<class InputIt>
  constexpr small_vector(InputIt first, InputIt last)
    {
    auto res{detail::insert(*this, end(), first, last)};
    detail::handle_error(res);
    }

  constexpr small_vector(std::initializer_list<value_type> init)
    {
    auto res{detail::insert(*this, end(), init.begin(), init.end())};
    detail::handle_error(res);
    }

  constexpr small_vector(small_vector && rh) noexcept(std::is_nothrow_move_constructible_v<value_type>)
    {
    storage_.construct_move(std::move(rh.storage_));
    }

  constexpr auto operator=(small_vector && rh) noexcept(std::is_nothrow_move_assignable_v<value_type>) -> small_vector &
    {
    storage_.assign_move(std::move(rh.storage_));
    return *this;
    }

  ///\warning copy constructor may throw always, for dynamic as there is no other way to signalize allocation error
  constexpr small_vector(small_vector const & rh) { storage_.construct_copy(rh.storage_); }

  ///\warning copy constructor from compatibile small_vector with different buffered_size may throw always,
  //          for dynamic as there is no other way to signalize allocation error
  template<uint64_t M>
    requires(M != N)
  explicit constexpr small_vector(small_vector<value_type, size_type, M> const & rh)
    {
    storage_.construct_copy(rh.storage_);
    }

  ///\warning copy assignment may throw always, for dynamic as there is no other way to signalize allocation error
  constexpr auto operator=(small_vector const & rh) -> small_vector &
    {
    storage_.assign_copy(rh.storage_);
    return *this;
    }

  constexpr auto assign(small_vector && rh) noexcept(std::is_nothrow_move_assignable_v<value_type>) -> small_vector &
    {
    storage_.assign_move(std::move(rh.storage_));
    return *this;
    }

  ///\warning copy assignment may throw always, for dynamic as there is no other way to signalize allocation error
  template<uint64_t M>
  constexpr auto
    assign(small_vector<value_type, size_type, M> const & rh) noexcept(std::is_nothrow_copy_assignable_v<value_type>)
      -> small_vector &
    {
    storage_.assign_copy(rh.storage_);
    return *this;
    }

  constexpr ~small_vector() { storage_.destroy(); }

  template<concepts::unsigned_arithmetic_integral arg_size_type>
  [[nodiscard]]
  inline constexpr auto operator[](arg_size_type index) const noexcept -> value_type const &
    {
    assert(index < size());
    if constexpr(detail::check_valid_element_access)
      {
      if(storage_.size_ <= index) [[unlikely]]
        detail::report_invalid_element_access("out of bounds element access ", storage_.size_, index);
      }
    small_vectors_clang_unsafe_buffer_usage_begin  //
      return data()[index];
    small_vectors_clang_unsafe_buffer_usage_end  //
    }

  template<concepts::unsigned_arithmetic_integral arg_size_type>
  [[nodiscard]]
  inline constexpr auto operator[](arg_size_type index) noexcept -> value_type &
    {
    assert(index < size());
    if constexpr(detail::check_valid_element_access)
      {
      if(storage_.size_ <= index) [[unlikely]]
        detail::report_invalid_element_access("out of bounds element access ", storage_.size_, index);
      }
    small_vectors_clang_unsafe_buffer_usage_begin  //
      return data()[index];
    small_vectors_clang_unsafe_buffer_usage_end  //
    }

  template<concepts::unsigned_arithmetic_integral arg_size_type>
  [[nodiscard]]
  inline constexpr auto at(arg_size_type index) const noexcept -> value_type const &
    {
    assert(index < size());
    if constexpr(detail::check_valid_element_access)
      {
      if(storage_.size_ <= index) [[unlikely]]
        detail::report_invalid_element_access("out of bounds element access ", storage_.size_, index);
      }
    small_vectors_clang_unsafe_buffer_usage_begin  //
      return data()[index];
    small_vectors_clang_unsafe_buffer_usage_end  //
    }

  template<concepts::unsigned_arithmetic_integral arg_size_type>
  [[nodiscard]]
  inline constexpr auto at(arg_size_type index) noexcept -> value_type &
    {
    assert(index < size());
    if constexpr(detail::check_valid_element_access)
      {
      if(storage_.size_ <= index) [[unlikely]]
        detail::report_invalid_element_access("out of bounds element access ", storage_.size_, index);
      }
    small_vectors_clang_unsafe_buffer_usage_begin  //
      return data()[index];
    small_vectors_clang_unsafe_buffer_usage_end  //
    }

  // compatibility with old code
  inline constexpr auto empty() const noexcept -> bool { return detail::empty(*this); }

  inline constexpr auto begin() const noexcept -> const_iterator { return const_iterator{detail::begin(*this)}; }

  inline constexpr auto cbegin() const noexcept -> const_iterator { return begin(); }

  inline constexpr auto begin() noexcept -> iterator { return iterator{detail::begin(*this)}; }

  inline constexpr auto end() const noexcept -> const_iterator { return const_iterator{detail::end(*this)}; }

  inline constexpr auto cend() const noexcept -> const_iterator { return end(); }

  inline constexpr auto end() noexcept -> iterator { return iterator{detail::end(*this)}; }

  inline constexpr auto rbegin() noexcept -> reverse_iterator { return reverse_iterator{end()}; }

  inline constexpr auto rbegin() const noexcept -> const_reverse_iterator { return const_reverse_iterator{end()}; }

  inline constexpr auto crbegin() const noexcept -> const_reverse_iterator { return const_reverse_iterator{end()}; }

  inline constexpr auto rend() noexcept -> reverse_iterator { return reverse_iterator{begin()}; }

  inline constexpr auto rend() const noexcept -> const_reverse_iterator { return const_reverse_iterator{begin()}; }

  inline constexpr auto crend() const noexcept -> const_reverse_iterator { return const_reverse_iterator{begin()}; }

  ///\brief Appends a new element to the end of the container
  ///       if user type throw during move or construction it meets strong exception guarantee
  ///       thismethod throw bad_alloc when reallocation fails to allocate memory
  ///       standalone function not throwing may be used instead emplace_back( vector, args ... )
  template<typename... Args>
  inline constexpr auto emplace_back(Args &&... args) -> value_type &
    {
    auto res = detail::emplace_back(*this, std::forward<Args>(args)...);
    detail::handle_error(res);
    return back();
    }

  ///\brief Appends a new element to the end of the container
  ///       if user type throw during move or construction it meets strong exception guarantee
  ///       thismethod throw bad_alloc when reallocation fails to allocate memory
  ///       standalone function not throwing may be used instead push_back( vector, args ... )
  template<typename T>
  inline constexpr void push_back(T && value)
    {
    auto res = detail::push_back(*this, std::forward<T>(value));
    detail::handle_error(res);
    }

  inline constexpr auto front() const noexcept -> value_type const & { return detail::front(*this); }

  inline constexpr auto front() noexcept -> value_type & { return detail::front(*this); }

  inline constexpr auto back() const noexcept -> value_type const & { return detail::back(*this); }

  inline constexpr auto back() noexcept -> value_type & { return detail::back(*this); }

  inline constexpr void clear() noexcept { detail::clear(*this); }

  inline constexpr auto erase(const_iterator pos) noexcept(noexcept(detail::erase(*this, pos))) -> iterator
    {
    return detail::erase(*this, pos);
    }

  inline constexpr auto
    erase(const_iterator first, const_iterator last) noexcept(noexcept(detail::erase(*this, first, last))) -> iterator
    {
    return detail::erase(*this, first, last);
    }

  inline constexpr auto erase_at_end(const_iterator pos) noexcept -> iterator
    {
    return detail::erase_at_end(*this, pos);
    }

  inline constexpr void pop_back() noexcept { detail::pop_back(*this); }

  template<concepts::random_access_iterator source_iterator>
  inline constexpr void insert(const_iterator itpos, source_iterator itbeg, source_iterator itend)
    {
    auto res = detail::insert(*this, itpos, itbeg, itend);
    detail::handle_error(res);
    }

  ///\returns Iterator pointing to the emplaced element.
  template<typename... Args>
  inline constexpr auto emplace(const_iterator itpos, Args &&... args) -> iterator
    {
    auto ix{std::distance(cbegin(), itpos)};
    auto res = detail::emplace(*this, itpos, std::forward<Args>(args)...);
    detail::handle_error(res);
    return std::next(begin(), ix);
    }

  inline constexpr void reserve(size_type new_cap)
    {
    auto res = detail::reserve(*this, new_cap);
    detail::handle_error(res);
    }

  inline constexpr auto resize(size_type new_size)
    {
    auto res = detail::resize(*this, new_size);
    detail::handle_error(res);
    }

  // private detail use
  inline constexpr void set_size_priv_(size_type pos_ix) noexcept { storage_.size_ = pos_ix; }

  inline constexpr dynamic_storage_type exchange_priv_(dynamic_storage_type new_storage, size_type size) noexcept
    {
    return storage_.exchange_priv_(new_storage, size);
    }

  inline constexpr dynamic_storage_type switch_static_priv_() noexcept
    requires(N != 0)
    {
    return storage_.switch_static_priv_();
    }

  inline constexpr auto shrink_to_fit() -> vector_outcome_e { return detail::shrink_to_fit(*this); }

  inline constexpr void swap(small_vector & b) noexcept { storage_.swap(b.storage_); }
  };

// always relocatable after move
template<typename V, std::unsigned_integral S, uint64_t N>
consteval bool adl_decl_trivially_destructible_after_move(small_vector<V, S, N> const *)
  {
  return true;
  }

template<typename V, std::unsigned_integral S, uint64_t N>
small_vector(small_vector<V, S, N> &&) -> small_vector<V, S, N>;

template<typename V, std::unsigned_integral S, uint64_t N>
small_vector(small_vector<V, S, N> const &) -> small_vector<V, S, N>;

template<typename V>
using vector = small_vector<V, std::uint32_t, 0>;

namespace concepts
  {
  ///\brief constraint requiring type to be a static_vector or const static_vector
  template<typename small_vector_type>
  concept same_as_small_vector = requires {
    typename small_vector_type::value_type;
    typename small_vector_type::size_type;
      { small_vector_type::buffered_capacity() } noexcept -> std::same_as<typename small_vector_type::size_type>;
      requires concepts::unsigned_arithmetic_integral<typename small_vector_type::size_type>;
    requires std::same_as<
      std::remove_const_t<small_vector_type>,
      small_vector<
        typename small_vector_type::value_type,
        typename small_vector_type::size_type,
        small_vector_type::buffered_capacity()>>;
  };
  }  // namespace concepts

using detail::at_least;

template<typename V, typename S, uint64_t N>
[[nodiscard]]
inline constexpr auto size(small_vector<V, S, N> const & vec) noexcept -> typename small_vector<V, S, N>::size_type
  {
  return vec.size();
  }

template<typename V, typename S, uint64_t N>
[[nodiscard]]
inline constexpr auto empty(small_vector<V, S, N> const & vec) noexcept -> bool
  {
  return vec.empty();
  }

template<concepts::same_as_small_vector small_vector_type, concepts::unsigned_arithmetic_integral arg_size_type>
[[nodiscard]]
inline constexpr auto & at(small_vector_type & vec, arg_size_type index) noexcept
  {
  return vec.at(index);
  }

template<typename V, typename S, uint64_t N>
[[nodiscard]]
inline constexpr auto capacity(small_vector<V, S, N> const & vec) noexcept -> typename small_vector<V, S, N>::size_type
  {
  return vec.capacity();
  }

template<typename V, typename S, uint64_t N>
[[nodiscard]]
inline constexpr auto max_size(small_vector<V, S, N> const &) noexcept -> typename small_vector<V, S, N>::size_type
  {
  return small_vector<V, S, N>::max_size();
  }

template<typename V, typename S, uint64_t N>
[[nodiscard]]
inline constexpr auto free_space(small_vector<V, S, N> const & vec) noexcept ->
  typename small_vector<V, S, N>::size_type
  {
  return vec.free_space();
  }

template<concepts::same_as_small_vector small_vector_type>
[[nodiscard]]
inline constexpr auto data(small_vector_type & vec) noexcept
  {
  return vec.data();
  }

template<concepts::same_as_small_vector small_vector_type>
[[nodiscard]]
inline constexpr auto begin(small_vector_type & vec) noexcept
  {
  return vec.begin();
  }

template<typename V, typename S, uint64_t N>
[[nodiscard]]
inline constexpr auto cbegin(small_vector<V, S, N> const & vec) noexcept
  {
  return vec.cbegin();
  }

template<concepts::same_as_small_vector small_vector_type>
[[nodiscard]]
inline constexpr auto end(small_vector_type & vec) noexcept
  {
  return vec.end();
  }

template<concepts::same_as_small_vector small_vector_type>
[[nodiscard]]
inline constexpr auto & front(small_vector_type & vec) noexcept
  {
  return vec.front();
  }

template<concepts::same_as_small_vector small_vector_type>
[[nodiscard]]
inline constexpr auto & back(small_vector_type & vec) noexcept
  {
  return vec.back();
  }

template<typename V, typename S, uint64_t N>
inline constexpr auto
  erase_at_end(small_vector<V, S, N> & vec, typename small_vector<V, S, N>::const_iterator pos) noexcept ->
  typename small_vector<V, S, N>::iterator
  {
  return vec.erase_at_end(pos);
  }

template<typename V, typename S, uint64_t N>
inline constexpr void clear(small_vector<V, S, N> & vec) noexcept
  {
  vec.clear();
  }

template<typename V, typename S, uint64_t N>
inline constexpr auto erase(small_vector<V, S, N> & vec, typename small_vector<V, S, N>::const_iterator pos) ->
  typename small_vector<V, S, N>::iterator
  {
  return vec.erase(pos);
  }

template<typename V, typename S, uint64_t N>
inline constexpr auto erase(
  small_vector<V, S, N> & vec,
  typename small_vector<V, S, N>::const_iterator first,
  typename small_vector<V, S, N>::const_iterator last
) -> typename small_vector<V, S, N>::iterator
  {
  return vec.erase(first, last);
  }

template<typename V, typename S, uint64_t N>
inline constexpr void pop_back(small_vector<V, S, N> & vec) noexcept
  {
  vec.pop_back();
  }

template<typename V, typename S, uint64_t N, concepts::random_access_iterator source_iterator>
inline constexpr auto insert(
  small_vector<V, S, N> & vec,
  typename small_vector<V, S, N>::const_iterator itpos,
  source_iterator itbeg,
  source_iterator itend
)
  {
  return vec.insert(itpos, itbeg, itend);
  }

template<typename V, typename S, uint64_t N, typename... Args>
inline constexpr auto
  emplace(small_vector<V, S, N> & vec, typename small_vector<V, S, N>::const_iterator itpos, Args &&... args) ->
  typename small_vector<V, S, N>::iterator
  {
  return vec.emplace(itpos, std::forward<Args>(args)...);
  }

template<typename V, typename S, uint64_t N, typename... Args>
inline constexpr auto emplace_back(small_vector<V, S, N> & vec, Args &&... args) ->
  typename small_vector<V, S, N>::value_type &
  {
  return vec.emplace_back(std::forward<Args>(args)...);
  }

template<typename V, typename S, uint64_t N, typename T>
inline constexpr void push_back(small_vector<V, S, N> & vec, T && value)
  {
  vec.push_back(std::forward<T>(value));
  }

template<typename V, typename S, uint64_t N>
inline constexpr void reserve(small_vector<V, S, N> & vec, typename small_vector<V, S, N>::size_type new_cap)
  {
  vec.reserve(new_cap);
  }

template<typename V, typename S, uint64_t N>
inline constexpr void resize(small_vector<V, S, N> & vec, typename small_vector<V, S, N>::size_type new_size)
  {
  vec.resize(new_size);
  }

template<typename V, typename S, uint64_t N>
inline constexpr auto shrink_to_fit(small_vector<V, S, N> & vec) -> vector_outcome_e
  {
  return vec.shrink_to_fit();
  }
  }  // namespace small_vectors::inline v3_3
