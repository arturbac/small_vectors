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

#include <cassert>

#include <small_vectors/detail/safe_buffers.h>
#include <small_vectors/detail/uninitialized_constexpr.h>
#include <small_vectors/detail/vector_storage.h>
#include <small_vectors/detail/vector_func.h>
#include <small_vectors/detail/adapter_iterator.h>
#include <small_vectors/detail/conditional_trivial_reloc_base.h>

namespace small_vectors::inline v3_3
  {
using detail::vector_outcome_e;
using detail::vector_tune_e;

///\brief vector like container with constant storage space inside struct
///       for trivialy constructible elements it can be used in consteval expresions with clang, with gcc is possibility
///       to use with any objects types
template<concepts::vector_constraints V, uint64_t N>
struct [[clang::trivial_abi]]
static_vector : public conditional_trivial_reloc_base<V>
  {
  using value_type = V;
  using reference = value_type &;
  using const_reference = value_type const &;
  using size_type = typename detail::size_type_select_t<N>;
  using const_iterator = detail::adapter_iterator<value_type const *>;
  using iterator = detail::adapter_iterator<value_type *>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  using enum detail::vector_outcome_e;

  static constexpr size_type capacity_ = N;

  static constexpr bool support_reallocation() noexcept { return false; }

  detail::static_vector_storage<value_type, capacity_> storage_;

  [[nodiscard]]
  static inline constexpr auto capacity() noexcept -> size_type
    {
    return capacity_;
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
    return capacity_;
    }

  constexpr static_vector() noexcept = default;

  constexpr static_vector(static_vector && rh) noexcept
    requires concepts::trivially_copyable<value_type>
  = default;

  constexpr static_vector(static_vector const & rh) noexcept
    requires concepts::trivially_copyable<value_type>
  = default;

  constexpr static_vector(static_vector && rh) noexcept(std::is_nothrow_move_constructible_v<value_type>)
    requires(std::move_constructible<value_type> && !concepts::trivially_copyable<value_type>)
    {
    storage_.construct_move(std::move(rh.storage_));
    }

  constexpr static_vector(static_vector const & rh) noexcept(std::is_nothrow_copy_constructible_v<value_type>)
    requires(std::copy_constructible<value_type> && !concepts::trivially_copyable<value_type>)
    {
    storage_.construct_copy(rh.storage_);
    }

  template<uint64_t M>
    requires(M < N && std::copy_constructible<value_type>)
  explicit constexpr static_vector(
    static_vector<value_type, M> const & rh
  ) noexcept(std::is_nothrow_copy_constructible_v<value_type>)
    {
    storage_.construct_copy(rh.storage_);
    }

  template<std::input_iterator InputIt>
  constexpr static_vector(InputIt first, InputIt last)
    {
    auto res{detail::insert(*this, end(), first, last)};
    detail::handle_error(res);
    }

  // static_vector is address independant which means it is trivialy copyable for trivially_copyable<value_type>
  constexpr static_vector & operator=(static_vector && rh) noexcept
    requires concepts::trivially_copyable<value_type>
  = default;

  constexpr static_vector & operator=(static_vector const & rh) noexcept
    requires concepts::trivially_copyable<value_type>
  = default;

  constexpr static_vector & operator=(static_vector && rh) noexcept(std::is_nothrow_move_assignable_v<value_type>)
    requires(std::movable<value_type> && !concepts::trivially_copyable<value_type>)
    {
    storage_.assign_move(std::move(rh.storage_));
    return *this;
    }

  constexpr static_vector & operator=(static_vector const & rh) noexcept(std::is_nothrow_copy_assignable_v<value_type>)
    requires(std::copyable<value_type> && !concepts::trivially_copyable<value_type>)
    {
    storage_.assign_copy(rh.storage_);
    return *this;
    }

  constexpr ~static_vector()
    {
    if constexpr(!std::is_trivially_destructible_v<value_type>)
      detail::destroy_range(data(), size_type{0}, storage_.size_);
    }

  // compatibility with old code
  inline constexpr auto empty() const noexcept -> bool { return detail::empty(*this); }

  inline constexpr auto begin() const noexcept -> const_iterator { return const_iterator{detail::begin(*this)}; }

  inline constexpr auto cbegin() const noexcept -> const_iterator { return const_iterator{detail::cbegin(*this)}; }

  inline constexpr auto begin() noexcept -> iterator { return iterator{detail::begin(*this)}; }

  inline constexpr auto end() const noexcept -> const_iterator { return const_iterator{detail::end(*this)}; }

  inline constexpr auto cend() const noexcept -> const_iterator { return const_iterator{detail::end(*this)}; }

  inline constexpr auto end() noexcept -> iterator { return iterator{detail::end(*this)}; }

  inline constexpr auto rbegin() noexcept -> reverse_iterator { return reverse_iterator{end()}; }

  inline constexpr auto rbegin() const noexcept -> const_reverse_iterator { return const_reverse_iterator{end()}; }

  inline constexpr auto crbegin() const noexcept -> const_reverse_iterator { return const_reverse_iterator{end()}; }

  inline constexpr auto rend() noexcept -> reverse_iterator { return reverse_iterator{begin()}; }

  inline constexpr auto rend() const noexcept -> const_reverse_iterator { return const_reverse_iterator{begin()}; }

  inline constexpr auto crend() const noexcept -> const_reverse_iterator { return const_reverse_iterator{begin()}; }

  [[nodiscard]]
  inline constexpr auto operator[](concepts::unsigned_arithmetic_integral auto index) const noexcept
    -> value_type const &
    {
    assert(index < storage_.size_);
    if constexpr(detail::check_valid_element_access)
      {
      if(storage_.size_ <= index) [[unlikely]]
        detail::report_invalid_element_access("out of bounds element access ", storage_.size_, index);
      }
    small_vectors_clang_unsafe_buffer_usage_begin  //
      return data()[index];
    small_vectors_clang_unsafe_buffer_usage_end  //
    }

  [[nodiscard]]
  inline constexpr auto operator[](concepts::unsigned_arithmetic_integral auto index) noexcept -> value_type &
    {
    assert(index < storage_.size_);
    if constexpr(detail::check_valid_element_access)
      {
      if(storage_.size_ <= index) [[unlikely]]
        detail::report_invalid_element_access("out of bounds element access ", storage_.size_, index);
      }
    small_vectors_clang_unsafe_buffer_usage_begin  //
      return data()[index];
    small_vectors_clang_unsafe_buffer_usage_end  //
    }

  template<vector_tune_e tune = vector_tune_e::checked, typename... Args>
  inline constexpr auto emplace_back(Args &&... args) noexcept(
    tune == vector_tune_e::unchecked || noexcept(detail::emplace_back(*this, std::forward<Args>(args)...))
  )
    {
    if constexpr(tune == vector_tune_e::checked)
      return detail::emplace_back(*this, std::forward<Args>(args)...);
    else
      return detail::emplace_back_unchecked(*this, std::forward<Args>(args)...);
    }

  template<typename T>
  inline constexpr void push_back(T && value) noexcept(noexcept(detail::emplace_back(*this, std::forward<T>(value))))
    {
    detail::emplace_back(*this, std::forward<T>(value));
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

  inline constexpr void pop_back() noexcept { detail::pop_back(*this); }

  template<concepts::random_access_iterator source_iterator>
  inline constexpr auto insert(iterator itpos, source_iterator itbeg, source_iterator itend) noexcept(
    noexcept(detail::insert(*this, itpos, itbeg, itend))
  ) -> vector_outcome_e
    {
    return detail::insert(*this, itpos, itbeg, itend);
    }

  template<vector_tune_e tune = vector_tune_e::checked, typename... Args>
  inline constexpr auto emplace(iterator itpos, Args &&... args) noexcept(
    tune == vector_tune_e::unchecked || noexcept(detail::emplace(*this, itpos, std::forward<Args>(args)...))
  )
    {
    if constexpr(tune == vector_tune_e::checked)
      return detail::emplace(*this, itpos, std::forward<Args>(args)...);
    else
      return detail::emplace_unchecked(*this, itpos, std::forward<Args>(args)...);
    }

  inline constexpr auto resize(size_type new_size) noexcept(noexcept(detail::resize(*this, new_size)))
    -> vector_outcome_e
    {
    return detail::resize(*this, new_size);
    }

  inline constexpr auto erase_at_end(const_iterator pos) noexcept -> iterator
    {
    return detail::erase_at_end(*this, pos);
    }

  inline constexpr void set_size_priv_(size_type pos_ix) noexcept { storage_.size_ = pos_ix; }

  inline constexpr void swap(static_vector & b) noexcept { storage_.swap(b.storage_); }
  };

// always relocatable after move
template<concepts::vector_constraints V, uint64_t N>
consteval bool adl_decl_trivially_destructible_after_move(static_vector<V, N> const *)
  {
  return true;
  }

namespace concepts
  {
  ///\brief constraint requiring type to be a static_vector or const static_vector
  template<typename static_vector_type>
  concept same_as_static_vector = requires {
    typename static_vector_type::value_type;
    static_vector_type::capacity_;
    requires concepts::unsigned_arithmetic_integral<decltype(static_vector_type::capacity_)>;
    requires std::same_as<
      std::remove_const_t<static_vector_type>,
      static_vector<typename static_vector_type::value_type, static_vector_type::capacity_>>;
  };
  }  // namespace concepts

template<typename V, uint64_t N>
[[nodiscard]]
inline constexpr auto size(static_vector<V, N> const & vec) noexcept -> typename static_vector<V, N>::size_type
  {
  return vec.size();
  }

template<typename V, uint64_t N>
[[nodiscard]]
inline constexpr auto empty(static_vector<V, N> const & vec) noexcept -> bool
  {
  return vec.empty();
  }

template<concepts::same_as_static_vector static_vector_type, concepts::unsigned_arithmetic_integral index_type>
inline constexpr auto & at(static_vector_type & vec, index_type index) noexcept
  {
  return vec[index];
  }

template<typename V, uint64_t N>
[[nodiscard]]
inline constexpr auto capacity(static_vector<V, N> const &) noexcept -> typename static_vector<V, N>::size_type
  {
  return static_vector<V, N>::capacity();
  }

template<typename V, uint64_t N>
[[nodiscard]]
inline constexpr auto max_size(static_vector<V, N> const &) noexcept -> typename static_vector<V, N>::size_type
  {
  return static_vector<V, N>::max_size();
  }

template<typename V, uint64_t N>
[[nodiscard]]
inline constexpr auto free_space(static_vector<V, N> const & vec) noexcept -> typename static_vector<V, N>::size_type
  {
  return detail::free_space(vec);
  }

template<concepts::same_as_static_vector static_vector_type>
[[nodiscard]]
inline constexpr auto data(static_vector_type & vec) noexcept
  {
  return vec.data();
  }

template<concepts::same_as_static_vector static_vector_type>
[[nodiscard]]
inline constexpr auto begin(static_vector_type & vec) noexcept
  {
  return vec.begin();
  }

template<typename V, uint64_t N>
[[nodiscard]]
inline constexpr auto cbegin(static_vector<V, N> const & vec) noexcept
  {
  return vec.cbegin();
  }

template<concepts::same_as_static_vector static_vector_type>
[[nodiscard]]
inline constexpr auto end(static_vector_type & vec) noexcept
  {
  return vec.end();
  }

template<concepts::same_as_static_vector static_vector_type>
[[nodiscard]]
inline constexpr auto & front(static_vector_type & vec) noexcept
  {
  return vec.front();
  }

template<concepts::same_as_static_vector static_vector_type>
[[nodiscard]]
inline constexpr auto & back(static_vector_type & vec) noexcept
  {
  return vec.back();
  }

template<typename V, uint64_t N>
inline constexpr auto erase_at_end(static_vector<V, N> & vec, typename static_vector<V, N>::const_iterator pos) noexcept
  -> typename static_vector<V, N>::iterator
  {
  return vec.erase_at_end(pos);
  }

template<typename V, uint64_t N>
inline constexpr void clear(static_vector<V, N> & vec) noexcept
  {
  vec.clear();
  }

template<typename V, uint64_t N>
[[nodiscard]]
inline constexpr auto erase(static_vector<V, N> & vec, typename static_vector<V, N>::const_iterator pos) ->
  typename static_vector<V, N>::iterator
  {
  return vec.erase(pos);
  }

template<typename V, uint64_t N>
[[nodiscard]]
inline constexpr auto erase(
  static_vector<V, N> & vec,
  typename static_vector<V, N>::const_iterator first,
  typename static_vector<V, N>::const_iterator last
) -> typename static_vector<V, N>::iterator
  {
  return vec.erase(first, last);
  }

template<typename V, uint64_t N>
inline constexpr void pop_back(static_vector<V, N> & vec) noexcept
  {
  vec.pop_back();
  }

template<typename V, uint64_t N, concepts::random_access_iterator source_iterator>
inline constexpr auto insert(
  static_vector<V, N> & vec, typename static_vector<V, N>::iterator itpos, source_iterator itbeg, source_iterator itend
) -> vector_outcome_e
  {
  return vec.insert(itpos, itbeg, itend);
  }

template<typename V, uint64_t N>
inline constexpr auto resize(static_vector<V, N> & vec, typename static_vector<V, N>::size_type sz) -> vector_outcome_e
  {
  return vec.resize(sz);
  }

///\brief Appends a new element to the end of the container
///\warning this is unchecked version, insertion into full container is undefined behavior
template<vector_tune_e tune = vector_tune_e::checked, typename V, uint64_t N, typename... Args>
  requires(tune == vector_tune_e::unchecked)
inline constexpr void emplace_back(static_vector<V, N> & vec, Args &&... args) noexcept(
  noexcept(detail::emplace_back_unchecked(vec, std::forward<Args>(args)...))
)
  {
  detail::emplace_back_unchecked(vec, std::forward<Args>(args)...);
  }

///\brief Appends a new element to the end of the container
///\returns if there is no space left it returns \ref static_vector_outcome_e::out_of_storage
///         else it returns \ref static_vector_outcome_e::no_error
template<vector_tune_e tune = vector_tune_e::checked, typename V, uint64_t N, typename... Args>
  requires(tune == vector_tune_e::checked)
inline constexpr vector_outcome_e emplace_back(static_vector<V, N> & vec, Args &&... args) noexcept(
  noexcept(detail::emplace_back(vec, std::forward<Args>(args)...))
)
  {
  return detail::emplace_back(vec, std::forward<Args>(args)...);
  }

template<typename V, uint64_t N, typename T>
inline constexpr void
  push_back(static_vector<V, N> & vec, T && value) noexcept(noexcept(detail::emplace_back(vec, std::forward<T>(value))))
  {
  detail::emplace_back(vec, std::forward<T>(value));
  }

template<vector_tune_e tune = vector_tune_e::checked, typename V, uint64_t N, typename... Args>
  requires(tune == vector_tune_e::unchecked)
inline constexpr void emplace(
  static_vector<V, N> & vec, typename static_vector<V, N>::iterator itpos, Args &&... args
) noexcept(noexcept(detail::emplace_unchecked(vec, itpos, std::forward<Args>(args)...)))
  {
  detail::emplace_unchecked(vec, itpos, std::forward<Args>(args)...);
  }

template<vector_tune_e tune = vector_tune_e::checked, typename V, uint64_t N, typename... Args>
  requires(tune == vector_tune_e::checked)
inline constexpr vector_outcome_e emplace(
  static_vector<V, N> & vec, typename static_vector<V, N>::iterator itpos, Args &&... args
) noexcept(noexcept(detail::emplace(vec, itpos, std::forward<Args>(args)...)))
  {
  return detail::emplace(vec, itpos, std::forward<Args>(args)...);
  }

///\brief split \ref vec1 in half moving half of elements to the end of \ref vec2
///\warning this is unchecked version, insertion into full \ref vec2 container is undefined behavior
template<vector_tune_e tune = vector_tune_e::checked, typename V, uint64_t N1, uint64_t N2>
  requires(N2 >= N1 && concepts::even_size<N1> && tune == vector_tune_e::unchecked)
inline constexpr void split_by_half(
  static_vector<V, N1> & vec1, static_vector<V, N2> & vec2
) noexcept(std::is_nothrow_move_constructible_v<V>)
  {
  using size_type = typename static_vector<V, N1>::size_type;
  size_type half = static_cast<size_type>(size(vec1) >> 1);

  auto range_mid{begin(vec1) + half};
  size_type move_count = static_cast<size_type>(size(vec1) - half);
  if constexpr(std::is_nothrow_move_constructible_v<V>)
    detail::uninitialized_relocate_n(range_mid, move_count, end(vec2));
  else
    detail::uninitialized_relocate_with_copy_n(range_mid, move_count, end(vec2));
  vec1.storage_.size_ -= move_count;
  vec2.storage_.size_ += move_count;
  }

///\brief split \ref vec1 in half moving half of elements to the end of \ref vec2
///\returns if there is no space left it in \ref vec2 returns \ref static_vector_outcome_e::out_of_storage
///         else it returns \ref static_vector_outcome_e::no_error
template<vector_tune_e tune = vector_tune_e::checked, typename V, uint64_t N1, uint64_t N2>
  requires(N2 >= N1 && concepts::even_size<N1> && tune == vector_tune_e::checked)
inline constexpr vector_outcome_e split_by_half(
  static_vector<V, N1> & vec1, static_vector<V, N2> & vec2
) noexcept(std::is_nothrow_move_constructible_v<V>)
  {
  using size_type = typename static_vector<V, N1>::size_type;
  auto half = static_cast<size_type>(size(vec1) >> 1);
  if(free_space(vec2) >= half)
    {
    split_by_half<vector_tune_e::unchecked>(vec1, vec2);
    return vector_outcome_e::no_error;
    }
  else
    return vector_outcome_e::out_of_storage;
  }
  }  // namespace small_vectors::inline v3_3
