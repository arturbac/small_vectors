// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT
// SPDX-PackageHomePage: https://github.com/arturbac/small_vectors

//
// C++ Interface: safe_typedef
//
// Description:
// \class strong_type
// \brief encapsulation of basic types guarding for invalid substitution and casting
// \code example
/// \code{
///  struct row_tag :
///      public strong_type_default_traits
///  {};
///  using row_t = strong_type<uint16_t,row_tag>;
///\endcode
// Author: artur bac 2008-2024
//
#pragma once

#include <small_vectors/concepts/concepts.h>
#include <small_vectors/concepts/hashable.h>
#include <small_vectors/concepts/stream_insertable.h>
#include <small_vectors/concepts/numeric_limits.h>
#include <utility>
#include <limits>
// Get the declaration of the primary std::hash template.
// We are not permitted to declare it ourselves.
// <typeindex> is guaranteed to provide such a declaration,
// and is much cheaper to include than <functional>.
#include <typeindex>
#include <type_traits>
#include <iostream>
#include <compare>
#include <cstdint>

namespace small_vectors::inline v3_3::utils
  {

struct strong_type_default_traits
  {
  static constexpr bool enable_arithemtic = true;
  static constexpr bool enable_comparison = true;
  static constexpr bool enable_hash_specialization = true;
  static constexpr bool enable_binary_operators = true;
  static constexpr bool enable_ostream = true;
  };

// https://clang.llvm.org/docs/AttributeReference.html#trivial-abi
// Attribute trivial_abi has no effect when the class has a non-static data member whose type is non-trivial for the
// purposes of calls
template<typename ValueType, typename Tag>
class [[clang::trivial_abi]] strong_type
  {
public:
  using value_type = ValueType;
  using tag_type = Tag;
  using class_type = strong_type<value_type, tag_type>;
  using reference_type = value_type &;
  using const_reference_type = value_type const &;

private:
  value_type value_;

public:
  constexpr auto value() const noexcept -> value_type const & { return value_; }

  constexpr auto operator*() const noexcept -> value_type const & { return value_; }

  constexpr auto operator*() noexcept -> value_type & { return value_; }

  // used only for implementing operators
  constexpr auto ref_value() noexcept -> value_type & { return value_; }

  static constexpr auto max() noexcept -> strong_type
    requires concepts::has_numeric_limits_max<value_type>
    {
    return class_type(std::numeric_limits<value_type>::max());
    }

  constexpr strong_type() noexcept(std::is_nothrow_default_constructible_v<value_type>) = default;

  template<typename compat_type>
    requires std::constructible_from<value_type, compat_type>
  constexpr explicit strong_type(compat_type && v) noexcept(std::is_nothrow_constructible_v<value_type, compat_type>) :
      value_{value_type(std::forward<compat_type>(v))}
    {
    }

  constexpr strong_type(strong_type && v) noexcept(std::is_nothrow_move_constructible_v<value_type>) = default;
  constexpr strong_type(strong_type const & v) noexcept(std::is_nothrow_copy_constructible_v<value_type>) = default;

  constexpr auto operator=(strong_type && other) noexcept(std::is_nothrow_move_assignable_v<value_type>)
    -> strong_type & = default;
  constexpr auto operator=(strong_type const & other) noexcept(std::is_nothrow_copy_assignable_v<value_type>)
    -> strong_type & = default;
  };

//--------------------------------------------------------------------------------------------------------------
//
// hash
//
//--------------------------------------------------------------------------------------------------------------
namespace concepts
  {
  template<typename tag>
  concept tag_hash_specialization = requires {
    tag::enable_hash_specialization;
    requires tag::enable_hash_specialization == true;
  };
  }  // namespace concepts
  }  // namespace small_vectors::inline v3_3::utils

template<small_vectors::concepts::hashable value_type, small_vectors::utils::concepts::tag_hash_specialization tag>
struct std::hash<small_vectors::utils::strong_type<value_type, tag>>
  {
  [[nodiscard]]
#if defined(__cpp_static_call_operator)
  static
#endif
    constexpr auto operator()(small_vectors::utils::strong_type<value_type, tag> t)
#if !defined(__cpp_static_call_operator)
      const
#endif
    noexcept -> std::size_t
    {
    return std::hash<value_type>{}(*t);
    }
  };

namespace small_vectors::inline v3_3::concepts
  {
template<typename tag>
concept tag_ostream = requires {
  tag::enable_ostream;
  requires tag::enable_ostream == true;
};
  }  // namespace small_vectors::inline v3_3::concepts

namespace small_vectors::inline v3_3::utils
  {
template<small_vectors::concepts::stream_insertable value_type, small_vectors::concepts::tag_ostream tag>
inline auto operator<<(std::ostream & out, strong_type<value_type, tag> const & value) -> std::ostream &
  {
  out << *value;
  return out;
  }

//--------------------------------------------------------------------------------------------------------------
//
// strong_type arithemtic
//
//--------------------------------------------------------------------------------------------------------------
namespace concepts
  {
  template<typename tag>
  concept tag_arithemtic = requires {
    tag::enable_arithemtic;
    requires tag::enable_arithemtic == true;
  };
  }  // namespace concepts

template<small_vectors::concepts::prefix_incrementable value_type, concepts::tag_arithemtic tag>
constexpr auto operator++(strong_type<value_type, tag> & v) noexcept -> strong_type<value_type, tag> &
  {
  ++v.ref_value();
  return v;
  }

template<small_vectors::concepts::prefix_incrementable value_type, concepts::tag_arithemtic tag>
constexpr auto operator++(strong_type<value_type, tag> & v, int) noexcept -> strong_type<value_type, tag>
  {
  strong_type<value_type, tag> r{v};
  ++v.ref_value();
  return r;
  }

template<small_vectors::concepts::prefix_decrementable value_type, concepts::tag_arithemtic tag>
constexpr auto operator--(strong_type<value_type, tag> & v) noexcept -> strong_type<value_type, tag> const &
  {
  --v.ref_value();
  return v;
  }

template<small_vectors::concepts::prefix_decrementable value_type, concepts::tag_arithemtic tag>
constexpr auto operator--(strong_type<value_type, tag> & v, int) noexcept -> strong_type<value_type, tag>
  {
  strong_type<value_type, tag> r{v};
  --v.ref_value();
  return r;
  }

//--------------------------------------------------------------------------------------------------------------
//
// strong_type comprision
//
//--------------------------------------------------------------------------------------------------------------
namespace concepts
  {
  template<typename tag>
  concept tag_comparison = requires {
    tag::enable_comparison;
    requires tag::enable_comparison == true;
  };
  }  // namespace concepts

template<std::equality_comparable value_type, concepts::tag_comparison tag>
[[nodiscard]]
inline constexpr auto
  operator==(strong_type<value_type, tag> const & lhs, strong_type<value_type, tag> const & rhs) noexcept -> bool
  {
  return lhs.value() == rhs.value();
  }

template<std::equality_comparable value_type, concepts::tag_comparison tag>
[[nodiscard]]
inline constexpr auto
  operator!=(strong_type<value_type, tag> const & lhs, strong_type<value_type, tag> const & rhs) noexcept -> bool
  {
  return lhs.value() != rhs.value();
  }

template<std::three_way_comparable value_type, concepts::tag_comparison tag>
[[nodiscard]]
inline constexpr auto
  operator<=>(strong_type<value_type, tag> const & lhs, strong_type<value_type, tag> const & rhs) noexcept
  {
  return lhs.value() <=> rhs.value();
  }

//--------------------------------------------------------------------------------------------------------------
//
// strong_type arithmetic
//
//--------------------------------------------------------------------------------------------------------------
template<small_vectors::concepts::addable value_type, concepts::tag_arithemtic tag>
[[nodiscard]]
inline constexpr auto
  operator+(strong_type<value_type, tag> const & lhs, strong_type<value_type, tag> const & rhs) noexcept
  -> strong_type<value_type, tag>
  {
  return strong_type<value_type, tag>(static_cast<value_type>(lhs.value() + rhs.value()));
  }

template<small_vectors::concepts::substractable value_type, concepts::tag_arithemtic tag>
[[nodiscard]]
inline constexpr auto
  operator-(strong_type<value_type, tag> const & lhs, strong_type<value_type, tag> const & rhs) noexcept
  -> strong_type<value_type, tag>
  {
  return strong_type<value_type, tag>(static_cast<value_type>(lhs.value() - rhs.value()));
  }

template<small_vectors::concepts::multiplicatable value_type, concepts::tag_arithemtic tag>
[[nodiscard]]
inline constexpr auto
  operator*(strong_type<value_type, tag> const & lhs, strong_type<value_type, tag> const & rhs) noexcept
  -> strong_type<value_type, tag>
  {
  return strong_type<value_type, tag>(static_cast<value_type>(lhs.value() * rhs.value()));
  }

template<small_vectors::concepts::multiplicatable_with<int> value_type, concepts::tag_arithemtic tag>
[[nodiscard]]
inline constexpr auto operator*(strong_type<value_type, tag> const & lhs, int rhs) noexcept
  -> strong_type<value_type, tag>
  {
  return strong_type<value_type, tag>{static_cast<value_type>(lhs.value() * rhs)};
  }

template<small_vectors::concepts::dividable value_type, concepts::tag_arithemtic tag>
[[nodiscard]]
inline constexpr auto
  operator/(strong_type<value_type, tag> const & lhs, strong_type<value_type, tag> const & rhs) noexcept
  -> strong_type<value_type, tag>
  {
  return strong_type<value_type, tag>(static_cast<value_type>(lhs.value() / rhs.value()));
  }

template<small_vectors::concepts::dividable value_type, concepts::tag_arithemtic tag>
[[nodiscard]]
inline constexpr auto operator/(strong_type<value_type, tag> const & lhs, unsigned rhs) noexcept
  -> strong_type<value_type, tag>
  {
  return strong_type<value_type, tag>{static_cast<value_type>(lhs.value() / static_cast<value_type>(rhs))};
  }

template<small_vectors::concepts::has_modulo_operator value_type, concepts::tag_arithemtic tag>
[[nodiscard]]
inline constexpr auto
  operator%(strong_type<value_type, tag> const & lhs, strong_type<value_type, tag> const & rhs) noexcept
  -> strong_type<value_type, tag>
  {
  return strong_type<value_type, tag>(static_cast<value_type>(lhs.value() % rhs.value()));
  }

template<small_vectors::concepts::has_modulo_operator value_type, concepts::tag_arithemtic tag>
[[nodiscard]]
inline constexpr auto operator%(strong_type<value_type, tag> const & lhs, unsigned rhs) noexcept
  -> strong_type<value_type, tag>
  {
  return strong_type<value_type, tag>{static_cast<value_type>(lhs.value() % static_cast<value_type>(rhs))};
  }

//--------------------------------------------------------------------------------------------------------------
//
// strong_type binary bit operations
//
//--------------------------------------------------------------------------------------------------------------
namespace concepts
  {
  template<typename tag>
  concept tag_binary_operators = requires {
    tag::enable_binary_operators;
    requires tag::enable_binary_operators == true;
  };
  }  // namespace concepts

template<small_vectors::concepts::has_bitwise_xor_operator value_type, concepts::tag_binary_operators tag>
[[nodiscard]]
inline constexpr auto
  operator^(strong_type<value_type, tag> const & lhs, strong_type<value_type, tag> const & rhs) noexcept
  -> strong_type<value_type, tag>
  {
  return strong_type<value_type, tag>{static_cast<value_type>(lhs.value() ^ rhs.value())};
  }

template<small_vectors::concepts::has_bitwise_not_operator value_type, concepts::tag_binary_operators tag>
[[nodiscard]]
inline constexpr auto operator~(strong_type<value_type, tag> const & v) noexcept -> strong_type<value_type, tag>
  {
  return strong_type<value_type, tag>{static_cast<value_type>(~v.value())};
  }

template<small_vectors::concepts::has_left_shift_operator value_type, concepts::tag_binary_operators tag>
[[nodiscard]]
inline constexpr auto
  operator<<(strong_type<value_type, tag> const & lhs, strong_type<value_type, tag> const & rhs) noexcept
  -> strong_type<value_type, tag>
  {
  return strong_type<value_type, tag>{static_cast<value_type>(lhs.value() << rhs.value())};
  }

template<typename value_type, typename operand, concepts::tag_binary_operators tag>
  requires small_vectors::concepts::has_left_shift_operator_with_integral<value_type, operand>
[[nodiscard]]
inline constexpr auto operator<<(strong_type<value_type, tag> const & lhs, operand rhs) noexcept
  -> strong_type<value_type, tag>
  {
  return strong_type<value_type, tag>{static_cast<value_type>(lhs.value() << rhs)};
  }

template<small_vectors::concepts::has_right_shift_operator value_type, concepts::tag_binary_operators tag>
[[nodiscard]]
inline constexpr auto
  operator>>(strong_type<value_type, tag> const & lhs, strong_type<value_type, tag> const & rhs) noexcept
  -> strong_type<value_type, tag>
  {
  return strong_type<value_type, tag>{static_cast<value_type>(lhs.value() >> rhs.value())};
  }

template<typename value_type, typename operand, concepts::tag_binary_operators tag>
  requires small_vectors::concepts::has_right_shift_operator_with_integral<value_type, operand>
[[nodiscard]]
inline constexpr auto operator>>(strong_type<value_type, tag> const & lhs, operand rhs) noexcept
  -> strong_type<value_type, tag>
  {
  return strong_type<value_type, tag>{static_cast<value_type>(lhs.value() >> rhs)};
  }

template<small_vectors::concepts::has_bitwise_and_operator value_type, concepts::tag_binary_operators tag>
[[nodiscard]]
inline constexpr auto
  operator&(strong_type<value_type, tag> const & lhs, strong_type<value_type, tag> const & rhs) noexcept
  -> strong_type<value_type, tag>
  {
  return strong_type<value_type, tag>{static_cast<value_type>(lhs.value() & rhs.value())};
  }

template<
  small_vectors::concepts::has_bitwise_and_operator_with<unsigned> value_type,
  concepts::tag_binary_operators tag>
[[nodiscard]]
inline constexpr auto operator&(strong_type<value_type, tag> const & lhs, uint16_t rhs) noexcept
  -> strong_type<value_type, tag>
  {
  return strong_type<value_type, tag>{static_cast<value_type>(lhs.value() & rhs)};
  }

template<small_vectors::concepts::has_bitwise_or_operator value_type, concepts::tag_binary_operators tag>
[[nodiscard]]
inline constexpr auto
  operator|(strong_type<value_type, tag> const & lhs, strong_type<value_type, tag> const & rhs) noexcept
  -> strong_type<value_type, tag>
  {
  return strong_type<value_type, tag>{static_cast<value_type>(lhs.value() | rhs.value())};
  }

//--------------------------------------------------------------------------------------------------------------
//
// strong_type arithmetic
//
//--------------------------------------------------------------------------------------------------------------
template<small_vectors::concepts::addable_assign value_type, concepts::tag_arithemtic tag>
constexpr auto operator+=(strong_type<value_type, tag> & v, strong_type<value_type, tag> const & rhs) noexcept
  -> strong_type<value_type, tag> &
  {
  v.ref_value() += rhs.value();
  return v;
  }

template<small_vectors::concepts::subtractable_assign value_type, concepts::tag_arithemtic tag>
constexpr auto operator-=(strong_type<value_type, tag> & v, strong_type<value_type, tag> const & rhs) noexcept
  -> strong_type<value_type, tag> &
  {
  v.ref_value() -= rhs.value();
  return v;
  }

template<small_vectors::concepts::multiplicatable_assign value_type, concepts::tag_arithemtic tag>
constexpr auto operator*=(strong_type<value_type, tag> & v, strong_type<value_type, tag> const & rhs) noexcept
  -> strong_type<value_type, tag> &
  {
  v.ref_value() *= rhs.value();
  return v;
  }

template<small_vectors::concepts::dividable_assign value_type, concepts::tag_arithemtic tag>
constexpr auto operator/=(strong_type<value_type, tag> & v, strong_type<value_type, tag> const & rhs) noexcept
  -> strong_type<value_type, tag> &
  {
  v.ref_value() /= rhs.value();
  return v;
  }

template<small_vectors::concepts::has_modulo_assign value_type, concepts::tag_arithemtic tag>
constexpr auto operator%=(strong_type<value_type, tag> & v, strong_type<value_type, tag> const & rhs) noexcept
  -> strong_type<value_type, tag> &
  {
  v.ref_value() %= rhs.value();
  return v;
  }

//--------------------------------------------------------------------------------------------------------------
//
// strong_type binary bit operations
//
//--------------------------------------------------------------------------------------------------------------

template<small_vectors::concepts::has_bitwise_xor_assign value_type, concepts::tag_binary_operators tag>
constexpr auto operator^=(strong_type<value_type, tag> & v, strong_type<value_type, tag> const & rhs) noexcept
  -> strong_type<value_type, tag> &
  {
  v.ref_value() ^= rhs.value();
  return v;
  }

template<small_vectors::concepts::has_left_shift_assign value_type, concepts::tag_binary_operators tag>
constexpr auto operator<<=(strong_type<value_type, tag> & v, strong_type<value_type, tag> const & rhs) noexcept
  -> strong_type<value_type, tag> &
  {
  v.ref_value() <<= rhs.value();
  return v;
  }

template<small_vectors::concepts::has_right_shift_assign value_type, concepts::tag_binary_operators tag>
constexpr auto operator>>=(strong_type<value_type, tag> & v, strong_type<value_type, tag> const & rhs) noexcept
  -> strong_type<value_type, tag> &
  {
  v.ref_value() >>= rhs.value();
  return v;
  }

template<small_vectors::concepts::has_bitwise_and_assign value_type, concepts::tag_binary_operators tag>
constexpr auto operator&=(strong_type<value_type, tag> & v, strong_type<value_type, tag> const & rhs) noexcept
  -> strong_type<value_type, tag> &
  {
  v.ref_value() &= rhs.value();
  return v;
  }

template<small_vectors::concepts::has_bitwise_or_assign value_type, concepts::tag_binary_operators tag>
constexpr auto operator|=(strong_type<value_type, tag> & v, strong_type<value_type, tag> const & rhs) noexcept
  -> strong_type<value_type, tag> &
  {
  v.ref_value() |= rhs.value();
  return v;
  }
  }  // namespace small_vectors::inline v3_3::utils

//
