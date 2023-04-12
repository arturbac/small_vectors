#pragma once

#include "detail/iterator.h"
#include <string_view>
#include <algorithm>
#include <array>


namespace coll
{

template <typename CharType, std::size_t N>
struct basic_fixed_string
  {
  using value_type = CharType;
  using char_type = value_type;
  using iterator = detail::adapter_iterator<char_type *>;
  using const_iterator = detail::adapter_iterator<char_type const *>;
  
  char_type data_[N + 1]{};

  [[nodiscard]]
  static constexpr std::size_t size() noexcept { return N; }
  
  [[nodiscard]]
  constexpr const_iterator begin() const noexcept { return const_iterator{&data_[0]}; }
  
  [[nodiscard]]
  constexpr iterator begin() noexcept { return iterator{&data_[0]}; }
  
  [[nodiscard]]
  constexpr char_type * data() noexcept { return &data_[0]; }
  
  [[nodiscard]]
  constexpr char_type const * data() const noexcept { return &data_[0]; }
  
  [[nodiscard]]
  constexpr const_iterator end() const noexcept { return const_iterator{&data_[N]}; }
  
  [[nodiscard]]
  constexpr iterator end() noexcept { return iterator{&data_[N]}; }

  constexpr basic_fixed_string() noexcept  = default;

  template <std::same_as<char_type> other_char_type>
  constexpr basic_fixed_string(const other_char_type (&foo)[N + 1]) noexcept
    { std::copy_n(foo, N + 1, data_); }

  template <typename other_char_type>
  requires(!std::same_as<other_char_type, char_type>)
  constexpr basic_fixed_string(const other_char_type (&foo)[N + 1]) noexcept
    {
    std::transform(foo, foo + N + 1, data_,
                   [](other_char_type c) noexcept
                   { return static_cast<char_type>(c); });
    }

  constexpr std::basic_string_view<char_type> view() const noexcept
    { return { &data_[0], N }; }
    
  constexpr operator std::basic_string_view<char_type>() const noexcept
    { return { &data_[0], N }; }

  constexpr auto operator<=>(basic_fixed_string<char_type, N> const&) const noexcept = default;

  template <std::size_t M>
  constexpr auto operator==(basic_fixed_string<char_type, M> const& r) const noexcept
    { return N == M && view() == r.view(); }
  };

template <typename char_type, std::size_t N>
basic_fixed_string(char_type const (&str)[N]) -> basic_fixed_string<char_type, N - 1>;

template <typename char_type, std::size_t N, std::size_t M>
constexpr auto concat_fixed_string(basic_fixed_string<char_type, N> l,
                                   basic_fixed_string<char_type, M> r) noexcept
  {
  basic_fixed_string<char_type, N + M> result;
  auto it{ std::copy(l.begin(), l.end(), result.begin()) };
  it = std::copy(r.begin(), r.end(), it);
  *it = {};
  return result;
  }

template <typename char_type, std::size_t N, std::size_t M, typename... U>
constexpr auto concat_fixed_string(basic_fixed_string<char_type, N> l,
                                   basic_fixed_string<char_type, M> r, U... u) noexcept
  { return concat_fixed_string(l, concat_fixed_string(r, u...)); }


template <typename decl_chr_type, typename char_type, std::size_t N>
inline consteval auto cast_fixed_string(char_type const (&str)[N]) noexcept
  { return basic_fixed_string<decl_chr_type, N - 1>(str); }

}
