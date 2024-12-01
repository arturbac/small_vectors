#pragma once

#include <small_vectors/detail/safe_buffers.h>
#include <small_vectors/concepts/concepts.h>
#include <small_vectors/concepts/integral_or_byte.h>
#include <small_vectors/detail/adapter_iterator.h>
#include <string_view>
#include <algorithm>
#include <array>

namespace small_vectors::inline v3_0
  {

template<concepts::integral_or_byte CharType, std::size_t N>
struct basic_fixed_string
  {
  using value_type = CharType;
  using char_type = value_type;
  using iterator = detail::adapter_iterator<char_type *>;
  using const_iterator = detail::adapter_iterator<char_type const *>;

  char_type data_[N + 1]{};

  [[nodiscard]]
  static constexpr auto size() noexcept -> std::size_t
    {
    return N;
    }

  [[nodiscard]]
  constexpr auto begin() const noexcept -> const_iterator
    {
    return const_iterator{&data_[0]};
    }

  [[nodiscard]]
  constexpr auto begin() noexcept -> iterator
    {
    return iterator{&data_[0]};
    }

  [[nodiscard]]
  constexpr auto data() noexcept -> char_type *
    {
    return &data_[0];
    }

  [[nodiscard]]
  constexpr auto data() const noexcept -> char_type const *
    {
    return &data_[0];
    }

  [[nodiscard]]
  constexpr auto end() const noexcept -> const_iterator
    {
    small_vectors_clang_unsafe_buffer_usage_begin  //
      return const_iterator{&data_[N]};
    small_vectors_clang_unsafe_buffer_usage_end  //
    }

  [[nodiscard]]
  constexpr auto end() noexcept -> iterator
    {
    small_vectors_clang_unsafe_buffer_usage_begin  //
      return iterator{&data_[N]};
    small_vectors_clang_unsafe_buffer_usage_end  //
    }

  [[nodiscard]]
  inline constexpr auto operator[](concepts::unsigned_arithmetic_integral auto index) const noexcept
    -> char_type const &
    {
    small_vectors_clang_unsafe_buffer_usage_begin  //
      if constexpr(detail::check_valid_element_access)
      {
      if(N <= index) [[unlikely]]
        detail::report_invalid_element_access("out of bounds element access ", N, index);
      }
    return data_[index];
    small_vectors_clang_unsafe_buffer_usage_end  //
    }

  [[nodiscard]]
  inline constexpr auto operator[](concepts::unsigned_arithmetic_integral auto index) noexcept -> char_type &
    {
    small_vectors_clang_unsafe_buffer_usage_begin  //
      if constexpr(detail::check_valid_element_access)
      {
      if(N <= index) [[unlikely]]
        detail::report_invalid_element_access("out of bounds element access ", N, index);
      }
    return data_[index];
    small_vectors_clang_unsafe_buffer_usage_end  //
    }

  [[nodiscard]]
  inline constexpr auto at(concepts::unsigned_arithmetic_integral auto index) const noexcept -> char_type const &
    {
    small_vectors_clang_unsafe_buffer_usage_begin  //
      if constexpr(detail::check_valid_element_access)
      {
      if(N <= index) [[unlikely]]
        detail::report_invalid_element_access("out of bounds element access ", N, index);
      }
    return data_[index];
    small_vectors_clang_unsafe_buffer_usage_end  //
    }

  [[nodiscard]]
  inline constexpr auto at(concepts::unsigned_arithmetic_integral auto index) noexcept -> char_type &
    {
    small_vectors_clang_unsafe_buffer_usage_begin  //
      if constexpr(detail::check_valid_element_access)
      {
      if(N <= index) [[unlikely]]
        detail::report_invalid_element_access("out of bounds element access ", N, index);
      }
    return data_[index];
    small_vectors_clang_unsafe_buffer_usage_end  //
    }

  constexpr basic_fixed_string() noexcept = default;

  template<std::same_as<char_type> other_char_type>
  constexpr basic_fixed_string(other_char_type const (&foo)[N + 1]) noexcept
    {
    std::copy_n(foo, N + 1, data_);
    }

  template<typename other_char_type>
    requires(!std::same_as<other_char_type, char_type>)
  constexpr basic_fixed_string(other_char_type const (&foo)[N + 1]) noexcept
    {
    small_vectors_clang_unsafe_buffer_usage_begin  //
      std::transform(foo, foo + N + 1, data_, [](other_char_type c) noexcept { return static_cast<char_type>(c); });
    small_vectors_clang_unsafe_buffer_usage_end  //
    }

  constexpr auto view() const noexcept -> std::basic_string_view<char_type> { return {&data_[0], N}; }

  /// this is for use cases where basic_fixed_string is used as buffor with extra length and length must be determined
  /// by "\0"
  constexpr auto null_terminated_buffor_view() const noexcept -> std::basic_string_view<char_type>
    {
    return {&data_[0], std::char_traits<char_type>::length(&data_[0])};
    }

  constexpr operator std::basic_string_view<char_type>() const noexcept { return {&data_[0], N}; }

  constexpr auto operator<=>(basic_fixed_string<char_type, N> const &) const noexcept = default;

  template<std::size_t M>
  constexpr auto operator==(basic_fixed_string<char_type, M> const & r) const noexcept -> bool
    {
    return N == M && view() == r.view();
    }
  };

template<typename char_type, std::size_t N>
basic_fixed_string(char_type const (&str)[N]) -> basic_fixed_string<char_type, N - 1>;

template<typename char_type, std::size_t N, std::size_t M>
constexpr auto concat_fixed_string(basic_fixed_string<char_type, N> l, basic_fixed_string<char_type, M> r) noexcept
  -> basic_fixed_string<char_type, N + M>
  {
  basic_fixed_string<char_type, N + M> result;
  auto it{std::copy(l.begin(), l.end(), result.begin())};
  it = std::copy(r.begin(), r.end(), it);
  *it = {};
  return result;
  }

template<typename char_type, std::size_t N, std::size_t M, typename... U>
constexpr auto
  concat_fixed_string(basic_fixed_string<char_type, N> l, basic_fixed_string<char_type, M> r, U... u) noexcept
  {
  return concat_fixed_string(l, concat_fixed_string(r, u...));
  }

template<typename char_type, std::size_t N, std::size_t M>
constexpr auto operator+(basic_fixed_string<char_type, N> l, basic_fixed_string<char_type, M> r) noexcept
  {
  return concat_fixed_string(l, r);
  }

template<typename char_type, std::size_t N, std::size_t M>
constexpr auto operator+(basic_fixed_string<char_type, N> l, char_type const (&r)[M]) noexcept
  {
  return concat_fixed_string(l, basic_fixed_string{r});
  }

template<typename char_type, std::size_t N, std::size_t M>
constexpr auto operator+(char_type const (&l)[N], basic_fixed_string<char_type, M> r) noexcept
  {
  return concat_fixed_string(basic_fixed_string{l}, r);
  }

template<typename decl_chr_type, typename char_type, std::size_t N>
inline consteval auto cast_fixed_string(char_type const (&str)[N]) noexcept -> basic_fixed_string<decl_chr_type, N - 1>
  {
  return basic_fixed_string<decl_chr_type, N - 1>(str);
  }

  }  // namespace small_vectors::inline v3_0
