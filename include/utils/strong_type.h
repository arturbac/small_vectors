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
// Author: artur bac 2008-2022
//
#pragma once

#include <concepts>
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

  struct strong_type_default_traits
    {
    static constexpr bool enable_arithemtic = true;
    static constexpr bool enable_comparison = true;
    static constexpr bool enable_hash_specialization = true;
    static constexpr bool enable_binary_operators = true;
    static constexpr bool enable_ostream = true;
    };

  template< typename ValueType, typename Tag>
  class strong_type
    {
  public:
    using value_type = ValueType;
    using tag_type = Tag;
    using class_type = strong_type<value_type,tag_type>;
    using reference_type = value_type &;
    using const_reference_type = value_type const &;
    
  private:
    value_type            value_;
    
  public:
    constexpr const_reference_type value() const noexcept               { return value_; }
    constexpr const_reference_type operator*() const noexcept           { return value_; }
    //used only for implementing operators
    constexpr reference_type       ref_value() noexcept { return value_; }
    
    static constexpr class_type max() noexcept { return class_type(std::numeric_limits<value_type>::max()); }
    
    constexpr strong_type()
        noexcept(std::is_nothrow_default_constructible_v<value_type>) = default;

    template<typename compat_type>
      requires std::constructible_from<value_type,compat_type>
    constexpr explicit strong_type( compat_type && v )
        noexcept(std::is_nothrow_constructible_v<value_type,compat_type>)
      : value_{ value_type(std::forward<compat_type>(v)) } {}
    
    constexpr strong_type( strong_type && v) noexcept = default; 
    constexpr strong_type( strong_type const & v)
        noexcept(std::is_nothrow_copy_constructible_v<value_type>) = default;
    
    constexpr strong_type & operator =( strong_type && other) noexcept = default;
    constexpr strong_type & operator =( strong_type const & other)
        noexcept(std::is_nothrow_copy_constructible_v<value_type>) = default;
    };

    //--------------------------------------------------------------------------------------------------------------
    //
    // hash
    //
    //--------------------------------------------------------------------------------------------------------------
  namespace concepts
    {
    template<typename tag>
    concept tag_hash_specialization =
      requires
        {
        tag::enable_hash_specialization;
        requires tag::enable_hash_specialization == true;
        };
    }

  template<typename value_type, concepts::tag_hash_specialization tag>
  struct std::hash<strong_type<value_type,tag>>
    {
#if defined(__cpp_static_call_operator)
    static
#endif
    constexpr std::size_t operator()( strong_type<value_type,tag> t )
#if !defined(__cpp_static_call_operator)
       const
#endif
       noexcept
      {
      return std::hash<value_type>{}( *t );
      }
    };

  namespace concepts
    {
    template<typename tag>
    concept tag_ostream =
      requires
        {
        tag::enable_ostream;
        requires tag::enable_ostream == true;
        };
    }

  template<typename value_type, concepts::tag_ostream tag>
  inline std::ostream & operator <<( std::ostream & out, strong_type<value_type,tag> const & value )
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
    concept tag_arithemtic =
      requires
        {
        tag::enable_arithemtic;
        requires tag::enable_arithemtic == true;
        };
    }
  template<typename value_type, concepts::tag_arithemtic tag>
  constexpr strong_type<value_type,tag> const &
  operator ++( strong_type<value_type,tag> & v ) noexcept
    {
    ++v.ref_value();
    return v;
    }
    
  template<typename value_type, concepts::tag_arithemtic tag>
  constexpr strong_type<value_type,tag>
  operator ++( strong_type<value_type,tag> & v, int ) noexcept
    {
    strong_type<value_type,tag> r{v};
    ++v.ref_value();
    return r;
    }

  template<typename value_type, concepts::tag_arithemtic tag>
  constexpr strong_type<value_type,tag> const &
  operator --( strong_type<value_type,tag> & v ) noexcept
    {
    --v.ref_value();
    return v;
    }

  template<typename value_type, concepts::tag_arithemtic tag>
  constexpr strong_type<value_type,tag>
  operator --( strong_type<value_type,tag> & v, int ) noexcept
    {
    strong_type<value_type,tag> r{v};
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
    concept tag_comparison =
      requires
        {
        tag::enable_comparison;
        requires tag::enable_comparison == true;
        };
    }
  template<typename value_type, concepts::tag_comparison tag>
  [[nodiscard]]
  inline constexpr bool
  operator ==( strong_type<value_type,tag> const & lhs, strong_type<value_type,tag> const & rhs) noexcept
    { return lhs.value() == rhs.value(); }

  template<typename value_type, concepts::tag_comparison tag>
  [[nodiscard]]
  inline constexpr bool
  operator !=( strong_type<value_type,tag> const & lhs, strong_type<value_type,tag> const & rhs) noexcept
    { return lhs.value() != rhs.value(); }
    
  template<typename value_type, concepts::tag_comparison tag>
  [[nodiscard]]
  inline constexpr auto
  operator <=>( strong_type<value_type,tag> const & lhs, strong_type<value_type,tag> const & rhs) noexcept
    { return lhs.value() <=> rhs.value(); }
  //--------------------------------------------------------------------------------------------------------------
  //
  // strong_type arithmetic
  //
  //--------------------------------------------------------------------------------------------------------------
  template<typename value_type, concepts::tag_arithemtic tag>
  [[nodiscard]]
  inline constexpr strong_type<value_type,tag> 
  operator +(strong_type<value_type,tag> const & lhs, strong_type<value_type,tag> const & rhs) noexcept
    { return strong_type<value_type,tag>( static_cast<value_type>(lhs.value() + rhs.value()) ); }

  template<typename value_type, concepts::tag_arithemtic tag>
  [[nodiscard]]
  inline constexpr strong_type<value_type,tag>
  operator -(strong_type<value_type,tag> const & lhs, strong_type<value_type,tag> const & rhs) noexcept
    { return strong_type<value_type,tag>( static_cast<value_type>(lhs.value() - rhs.value()) ); }

  template<typename value_type, concepts::tag_arithemtic tag>
  [[nodiscard]]
  inline constexpr strong_type<value_type,tag>
  operator *(strong_type<value_type,tag> const & lhs, strong_type<value_type,tag> const & rhs) noexcept
    { return strong_type<value_type,tag>( static_cast<value_type>(lhs.value() * rhs.value()) ); }

  template<typename value_type, concepts::tag_arithemtic tag>
  [[nodiscard]]
  inline constexpr strong_type<value_type,tag>
  operator *(strong_type<value_type,tag> const & lhs, int rhs) noexcept
    { return strong_type<value_type,tag>{ static_cast<value_type>( lhs.value() * rhs ) }; }
  
  template<typename value_type, concepts::tag_arithemtic tag>
  [[nodiscard]]
  inline constexpr strong_type<value_type,tag>
  operator /(strong_type<value_type,tag> const & lhs, strong_type<value_type,tag> const & rhs) noexcept
    { return strong_type<value_type,tag>( static_cast<value_type>(lhs.value() / rhs.value()) ); }

  template<typename value_type, concepts::tag_arithemtic tag>
  [[nodiscard]]
  inline constexpr strong_type<value_type,tag>
  operator /(strong_type<value_type,tag> const & lhs, unsigned rhs) noexcept
    { return strong_type<value_type,tag>{ static_cast<value_type>(lhs.value() / static_cast<value_type>(rhs)) }; }
  
  template<typename value_type, concepts::tag_arithemtic tag>
  [[nodiscard]]
  inline constexpr strong_type<value_type,tag>
  operator %(strong_type<value_type,tag> const & lhs, strong_type<value_type,tag> const & rhs) noexcept
    { return strong_type<value_type,tag>( static_cast<value_type>(lhs.value() % rhs.value()) ); }

  template<typename value_type, concepts::tag_arithemtic tag>
  [[nodiscard]]
  inline constexpr strong_type<value_type,tag>
  operator %(strong_type<value_type,tag> const & lhs, unsigned rhs) noexcept
    { return strong_type<value_type,tag>{ static_cast<value_type>(lhs.value() % static_cast<value_type>(rhs)) }; }
    
  //--------------------------------------------------------------------------------------------------------------
  //
  // strong_type binary bit operations
  //
  //--------------------------------------------------------------------------------------------------------------
  namespace concepts
    {
    template<typename tag>
    concept tag_binary_operators =
      requires
        {
        tag::enable_binary_operators;
        requires tag::enable_binary_operators == true;
        };
    }
  template<typename value_type, concepts::tag_binary_operators tag>
  [[nodiscard]]
  inline constexpr strong_type<value_type,tag>
  operator ^(strong_type<value_type,tag> const & lhs, strong_type<value_type,tag> const & rhs) noexcept
    { return strong_type<value_type,tag>{ static_cast<value_type>(lhs.value() ^ rhs.value()) }; }

  template<typename value_type, concepts::tag_binary_operators tag>
  [[nodiscard]]
  inline constexpr strong_type<value_type,tag>
  operator ~(strong_type<value_type,tag> const & v) noexcept
    { return strong_type<value_type,tag>{ static_cast<value_type>(~v.value()) }; }

  template<typename value_type, concepts::tag_binary_operators tag>
  [[nodiscard]]
  inline constexpr strong_type<value_type,tag>
  operator <<(strong_type<value_type,tag> const & lhs, strong_type<value_type,tag> const & rhs) noexcept
    { return strong_type<value_type,tag>{ static_cast<value_type>( lhs.value() << rhs.value() ) }; }

  template<typename value_type, concepts::tag_binary_operators tag>
  [[nodiscard]]
  inline constexpr strong_type<value_type,tag>
  operator <<(strong_type<value_type,tag> const & lhs, unsigned rhs) noexcept
    { return strong_type<value_type,tag>{ static_cast<value_type>(lhs.value() << rhs) }; }
  
  template<typename value_type, concepts::tag_binary_operators tag>
  [[nodiscard]]
  inline constexpr strong_type<value_type,tag>
  operator >>( strong_type<value_type,tag> const & lhs, strong_type<value_type,tag> const & rhs) noexcept
    { return strong_type<value_type,tag>{ static_cast<value_type>(lhs.value() >> rhs.value()) }; }

  template<typename value_type, concepts::tag_binary_operators tag>
  [[nodiscard]]
  inline constexpr strong_type<value_type,tag>
  operator >>( strong_type<value_type,tag> const & lhs, unsigned rhs) noexcept
    { return strong_type<value_type,tag>{ static_cast<value_type>(lhs.value() >> rhs) }; }
  
  template<typename value_type, concepts::tag_binary_operators tag>
  [[nodiscard]]
  inline constexpr strong_type<value_type,tag>
  operator &( strong_type<value_type,tag> const & lhs, strong_type<value_type,tag> const & rhs) noexcept
    { return strong_type<value_type,tag>{ static_cast<value_type>(lhs.value() & rhs.value() ) }; }

  template<typename value_type, concepts::tag_binary_operators tag>
  [[nodiscard]]
  inline constexpr strong_type<value_type,tag>
  operator &( strong_type<value_type,tag> const & lhs, uint16_t rhs) noexcept
    { return strong_type<value_type,tag>{ static_cast<value_type>( lhs.value() & rhs ) }; }
  
  template<typename value_type, concepts::tag_binary_operators tag>
  [[nodiscard]]
  inline constexpr strong_type<value_type,tag>
  operator |( strong_type<value_type,tag> const & lhs, strong_type<value_type,tag> const & rhs) noexcept
    { return strong_type<value_type,tag>{ static_cast<value_type>(lhs.value() | rhs.value()) }; }

  //--------------------------------------------------------------------------------------------------------------
  //
  // strong_type arithmetic
  //
  //--------------------------------------------------------------------------------------------------------------
  template<typename value_type, concepts::tag_arithemtic tag>
  constexpr strong_type<value_type,tag> const &
  operator +=(strong_type<value_type,tag> & v, strong_type<value_type,tag> const & rhs) noexcept
    {
    v.ref_value() += rhs.value();
    return v;
    }

  template<typename value_type, concepts::tag_arithemtic tag>
  constexpr strong_type<value_type,tag> const &
  operator -=(strong_type<value_type,tag> & v, strong_type<value_type,tag> const & rhs) noexcept
    {
    v.ref_value() -= rhs.value();
    return v;
    }

  template<typename value_type, concepts::tag_arithemtic tag>
  constexpr strong_type<value_type,tag> const &
  operator *=(strong_type<value_type,tag> &v, strong_type<value_type,tag> const & rhs) noexcept
    {
    v.ref_value() *= rhs.value();
    return v;
    }

  template<typename value_type, concepts::tag_arithemtic tag>
  constexpr strong_type<value_type,tag> const &
  operator /=(strong_type<value_type,tag> &v, strong_type<value_type,tag> const & rhs) noexcept
    {
    v.ref_value() /= rhs.value();
    return v;
    }

  template<typename value_type, concepts::tag_arithemtic tag>
  constexpr strong_type<value_type,tag> const &
  operator %=(strong_type<value_type,tag> & v, strong_type<value_type,tag> const & rhs) noexcept
    {
    v.ref_value() %= rhs.value();
    return v;
    }

  //--------------------------------------------------------------------------------------------------------------
  //
  // strong_type binary bit operations
  //
  //--------------------------------------------------------------------------------------------------------------
  
  template<typename value_type, concepts::tag_binary_operators tag>
  constexpr strong_type<value_type,tag> const &
  operator ^=(strong_type<value_type,tag> & v, strong_type<value_type,tag> const & rhs) noexcept
    {
    v.ref_value() ^= rhs.value();
    return v;
    }

  template<typename value_type, concepts::tag_binary_operators tag>
  constexpr strong_type<value_type,tag> const &
  operator <<=(strong_type<value_type,tag> & v, strong_type<value_type,tag> const & rhs) noexcept
    {
    v.ref_value() <<= rhs.value();
    return v;
    }

  template<typename value_type, concepts::tag_binary_operators tag>
  constexpr strong_type<value_type,tag> const &
  operator >>=(strong_type<value_type,tag> & v, strong_type<value_type,tag> const & rhs) noexcept
    {
    v.ref_value() >>= rhs.value();
    return v;
    }

  template<typename value_type, concepts::tag_binary_operators tag>
  constexpr strong_type<value_type,tag> const &
  operator &=(strong_type<value_type,tag> & v, strong_type<value_type,tag> const & rhs) noexcept
    {
    v.ref_value() &= rhs.value();
    return v;
    }

  template<typename value_type, concepts::tag_binary_operators tag>
  constexpr strong_type<value_type,tag> const &
  operator |=(strong_type<value_type,tag> & v, strong_type<value_type,tag> const & rhs) noexcept
    {
    v.ref_value() |= rhs.value();
    return v;
    }

  //--------------------------------------------------------------------------------------------------------------
  //
  // strong_type compose support
  //
  //--------------------------------------------------------------------------------------------------------------
//   template<typename char_type, typename value_type, typename tag>
//   constexpr auto compose_preconv( strong_type<value_type,tag> const & value ) noexcept
//      { return strconv::detail::compose_preconv<char_type>(*value); }
// 
