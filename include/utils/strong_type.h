//
// C++ Interface: safe_typedef
//
// Description:
// \class strong_type
// \brief enkapsulacja typow POD chronaica przed blednym podstawieniami i rzutowaniami
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
#include <utility>
#include <limits>
// Get the declaration of the primary std::hash template.
// We are not permitted to declare it ourselves.
// <typeindex> is guaranteed to provide such a declaration, 
// and is much cheaper to include than <functional>.
 
#include <typeindex> 
#include <type_traits>
#include <iostream>
#include <strutil/strconv_numeric.h>
#include <compare>

  struct strong_type_default_traits
    {
    static constexpr bool enable_arithemtic = true;
    static constexpr bool enable_comparision = true;
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
    constexpr value_type           value() const noexcept               { return value_; }
    constexpr value_type           operator*() const noexcept           { return value_; }
    //used only for implementing operators
    constexpr reference_type       ref_value() noexcept { return value_; }
    
    static constexpr class_type max() noexcept { return class_type(std::numeric_limits<value_type>::max()); }
    
    constexpr strong_type() noexcept(noexcept(ValueType{})) = default;

    constexpr explicit strong_type( value_type v ) noexcept(noexcept(ValueType{v})) : value_{ v } {}
    
    constexpr strong_type( strong_type && v) noexcept = default; 
    constexpr strong_type( strong_type const & v) noexcept(noexcept(ValueType{v.value_})) = default; 
    
    constexpr strong_type & operator =( strong_type && other) noexcept = default;
    constexpr strong_type & operator =( strong_type const & other) noexcept(noexcept(ValueType{other.value_})) = default;
    };

    //--------------------------------------------------------------------------------------------------------------
    //
    // hash
    //
    //--------------------------------------------------------------------------------------------------------------

  template<typename value_type, typename tag>
  struct std::hash<strong_type<value_type,tag>>
    {
    using class_type = strong_type<value_type,tag>;
    
    template<std::enable_if_t<tag::enable_hash_specialization, int> = 0>
    constexpr std::size_t operator()( strong_type<value_type,tag> t ) const noexcept
      {
      return std::hash<value_type>()( *t );
      }
    };

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_ostream, int> = 0>
  inline std::ostream & operator <<( std::ostream & out, strong_type<value_type,tag> value )
    {
    out << *value;
    return out;
    }

  //--------------------------------------------------------------------------------------------------------------
  //
  // strong_type arithemtic
  //
  //--------------------------------------------------------------------------------------------------------------
  template<typename value_type, typename tag, std::enable_if_t<tag::enable_arithemtic, int> = 0>
  constexpr strong_type<value_type,tag> const &
  operator ++( strong_type<value_type,tag> & v ) noexcept
    {
    ++v.ref_value();
    return v;
    }
    
  template<typename value_type, typename tag, std::enable_if_t<tag::enable_arithemtic, int> = 0>
  constexpr strong_type<value_type,tag>
  operator ++( strong_type<value_type,tag> & v, int ) noexcept
    {
    strong_type<value_type,tag> r{v};
    ++v.ref_value();
    return r;
    }

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_arithemtic, int> = 0>
  constexpr strong_type<value_type,tag> const &
  operator --( strong_type<value_type,tag> & v ) noexcept
    {
    --v.ref_value();
    return v;
    }

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_arithemtic, int> = 0>
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
  template<typename value_type, typename tag, std::enable_if_t<tag::enable_comparision, int> = 0>
  inline constexpr bool
  operator ==( strong_type<value_type,tag> lhs, strong_type<value_type,tag> rhs) noexcept
    { return lhs.value() == rhs.value(); }

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_comparision, int> = 0>
  inline constexpr bool
  operator !=( strong_type<value_type,tag> lhs, strong_type<value_type,tag> rhs) noexcept
    { return lhs.value() != rhs.value(); }
    
  template<typename value_type, typename tag, std::enable_if_t<tag::enable_comparision, int> = 0>
  inline constexpr bool
  operator <(strong_type<value_type,tag> lhs, strong_type<value_type,tag> rhs) noexcept
    { return lhs.value() < rhs.value(); }

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_comparision, int> = 0>
  inline constexpr bool
  operator <=(strong_type<value_type,tag> lhs, strong_type<value_type,tag> rhs) noexcept
    { return lhs.value() <= rhs.value(); }

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_comparision, int> = 0>
  inline constexpr bool
  operator >(strong_type<value_type,tag> lhs, strong_type<value_type,tag> rhs) noexcept
    { return lhs.value() > rhs.value(); }

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_comparision, int> = 0>
  inline constexpr bool
  operator >=(strong_type<value_type,tag> lhs, strong_type<value_type,tag> rhs) noexcept
    { return lhs.value() >= rhs.value(); }

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_comparision, int> = 0>
  inline constexpr auto
  operator <=>( strong_type<value_type,tag> lhs, strong_type<value_type,tag> rhs) noexcept
    { return lhs.value() <=> rhs.value(); }
  //--------------------------------------------------------------------------------------------------------------
  //
  // strong_type arithmetic
  //
  //--------------------------------------------------------------------------------------------------------------
  template<typename value_type, typename tag, std::enable_if_t<tag::enable_arithemtic, int> = 0>
  inline constexpr strong_type<value_type,tag> 
  operator +(strong_type<value_type,tag> lhs, strong_type<value_type,tag> rhs) noexcept
    { return strong_type<value_type,tag>( lhs.value() + rhs.value() ); }

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_arithemtic, int> = 0>
  inline constexpr strong_type<value_type,tag>
  operator -(strong_type<value_type,tag> lhs, strong_type<value_type,tag> rhs) noexcept
    { return strong_type<value_type,tag>( lhs.value() - rhs.value() ); }

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_arithemtic, int> = 0>
  inline constexpr strong_type<value_type,tag>
  operator *(strong_type<value_type,tag> lhs, strong_type<value_type,tag> rhs) noexcept
    { return strong_type<value_type,tag>( lhs.value() * rhs.value() ); }

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_arithemtic, int> = 0>
  inline constexpr strong_type<value_type,tag>
  operator *(strong_type<value_type,tag> lhs, int rhs) noexcept 
    { return strong_type<value_type,tag>{ static_cast<typename strong_type<value_type,tag>::value_type>( lhs.value() * rhs ) }; }
  
  template<typename value_type, typename tag, std::enable_if_t<tag::enable_arithemtic, int> = 0>
  inline constexpr strong_type<value_type,tag>
  operator /(strong_type<value_type,tag> lhs, strong_type<value_type,tag> rhs) noexcept
    { return strong_type<value_type,tag>( lhs.value() / rhs.value() ); }

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_arithemtic, int> = 0>
  inline constexpr strong_type<value_type,tag>
  operator /(strong_type<value_type,tag> lhs, unsigned rhs) noexcept
    { return strong_type<value_type,tag>{ lhs.value() / static_cast<typename strong_type<value_type,tag>::value_type>(rhs) }; }
  
  template<typename value_type, typename tag, std::enable_if_t<tag::enable_arithemtic, int> = 0>
  inline constexpr strong_type<value_type,tag>
  operator %(strong_type<value_type,tag> lhs, strong_type<value_type,tag> rhs) noexcept
    { return strong_type<value_type,tag>( lhs.value() % rhs.value() ); }

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_arithemtic, int> = 0>
  inline constexpr strong_type<value_type,tag>
  operator %(strong_type<value_type,tag> lhs, unsigned rhs) noexcept
    { return strong_type<value_type,tag>{ static_cast<value_type>(lhs.value() % static_cast<value_type>(rhs)) }; }
    
  //--------------------------------------------------------------------------------------------------------------
  //
  // strong_type binary bit operations
  //
  //--------------------------------------------------------------------------------------------------------------
  template<typename value_type, typename tag, std::enable_if_t<tag::enable_binary_operators, int> = 0>
  inline constexpr strong_type<value_type,tag>
  operator ^(strong_type<value_type,tag> lhs, strong_type<value_type,tag> rhs) noexcept
    { return strong_type<value_type,tag>{ static_cast<value_type>(lhs.value() ^ rhs.value()) }; }

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_binary_operators, int> = 0>
  inline constexpr strong_type<value_type,tag>
  operator ~(strong_type<value_type,tag> v) noexcept
    { return strong_type<value_type,tag>{ ~v.value() }; }

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_binary_operators, int> = 0>
  inline constexpr strong_type<value_type,tag>
  operator <<(strong_type<value_type,tag> lhs, strong_type<value_type,tag> rhs) noexcept
    { return strong_type<value_type,tag>{ static_cast<value_type>( lhs.value() << rhs.value() ) }; }

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_binary_operators, int> = 0>
  inline constexpr strong_type<value_type,tag>
  operator <<(strong_type<value_type,tag> lhs, unsigned rhs) noexcept
    { return strong_type<value_type,tag>{ static_cast<value_type>(lhs.value() << rhs) }; }
  
  template<typename value_type, typename tag, std::enable_if_t<tag::enable_binary_operators, int> = 0>
  inline constexpr strong_type<value_type,tag>
  operator >>( strong_type<value_type,tag> lhs, strong_type<value_type,tag> rhs) noexcept
    { return strong_type<value_type,tag>{ static_cast<value_type>(lhs.value() >> rhs.value()) }; }

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_binary_operators, int> = 0>
  inline constexpr strong_type<value_type,tag>
  operator >>( strong_type<value_type,tag> lhs, unsigned rhs) noexcept
    { return strong_type<value_type,tag>{ static_cast<value_type>(lhs.value() >> rhs) }; }
  
  template<typename value_type, typename tag, std::enable_if_t<tag::enable_binary_operators, int> = 0>
  inline constexpr strong_type<value_type,tag>
  operator &( strong_type<value_type,tag> lhs, strong_type<value_type,tag> rhs) noexcept
    { return strong_type<value_type,tag>{ static_cast<value_type>(lhs.value() & rhs.value() ) }; }

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_binary_operators, int> = 0>
  inline constexpr strong_type<value_type,tag>
  operator &( strong_type<value_type,tag> lhs, uint16_t rhs) noexcept
    { return strong_type<value_type,tag>{ static_cast<value_type>( lhs.value() & rhs ) }; }
  
  template<typename value_type, typename tag, std::enable_if_t<tag::enable_binary_operators, int> = 0>
  inline constexpr strong_type<value_type,tag>
  operator |( strong_type<value_type,tag> lhs, strong_type<value_type,tag> rhs) noexcept
    { return strong_type<value_type,tag>{ static_cast<value_type>(lhs.value() | rhs.value()) }; }

  //--------------------------------------------------------------------------------------------------------------
  //
  // strong_type arithmetic
  //
  //--------------------------------------------------------------------------------------------------------------
  template<typename value_type, typename tag, std::enable_if_t<tag::enable_arithemtic, int> = 0>
  constexpr strong_type<value_type,tag> const &
  operator +=(strong_type<value_type,tag> & v, strong_type<value_type,tag> rhs) noexcept
    {
    v.ref_value() += rhs.value();
    return v;
    }

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_arithemtic, int> = 0>
  constexpr strong_type<value_type,tag> const &
  operator -=(strong_type<value_type,tag> & v, strong_type<value_type,tag> rhs) noexcept
    {
    v.ref_value() -= rhs.value();
    return v;
    }

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_arithemtic, int> = 0>
  constexpr strong_type<value_type,tag> const &
  operator *=(strong_type<value_type,tag> &v, strong_type<value_type,tag> rhs) noexcept
    {
    v.ref_value() *= rhs.value();
    return v;
    }

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_arithemtic, int> = 0>
  constexpr strong_type<value_type,tag> const &
  operator /=(strong_type<value_type,tag> &v, strong_type<value_type,tag> rhs) noexcept
    {
    v.ref_value() /= rhs.value();
    return v;
    }

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_arithemtic, int> = 0>
  constexpr strong_type<value_type,tag> const &
  operator %=(strong_type<value_type,tag> & v, strong_type<value_type,tag> rhs) noexcept
    {
    v.ref_value() %= rhs.value();
    return v;
    }

  //--------------------------------------------------------------------------------------------------------------
  //
  // strong_type binary bit operations
  //
  //--------------------------------------------------------------------------------------------------------------
  
  template<typename value_type, typename tag, std::enable_if_t<tag::enable_binary_operators, int> = 0>
  constexpr strong_type<value_type,tag> const &
  operator ^=(strong_type<value_type,tag> & v, strong_type<value_type,tag> rhs) noexcept
    {
    v.ref_value() ^= rhs.value();
    return v;
    }

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_binary_operators, int> = 0>
  constexpr strong_type<value_type,tag> const &
  operator <<=(strong_type<value_type,tag> & v, strong_type<value_type,tag> rhs) noexcept
    {
    v.ref_value() <<= rhs.value();
    return v;
    }

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_binary_operators, int> = 0>
  constexpr strong_type<value_type,tag> const &
  operator >>=(strong_type<value_type,tag> & v, strong_type<value_type,tag> rhs) noexcept
    {
    v.ref_value() >>= rhs.value();
    return v;
    }

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_binary_operators, int> = 0>
  constexpr strong_type<value_type,tag> const &
  operator &=(strong_type<value_type,tag> & v, strong_type<value_type,tag> rhs) noexcept
    {
    v.ref_value() &= rhs.value();
    return v;
    }

  template<typename value_type, typename tag, std::enable_if_t<tag::enable_binary_operators, int> = 0>
  constexpr strong_type<value_type,tag> const &
  operator |=(strong_type<value_type,tag> & v, strong_type<value_type,tag> rhs) noexcept
    {
    v.ref_value() |= rhs.value();
    return v;
    }

  //--------------------------------------------------------------------------------------------------------------
  //
  // strong_type compose support
  //
  //--------------------------------------------------------------------------------------------------------------
  template<typename char_type, typename value_type, typename tag>
  constexpr auto compose_preconv( strong_type<value_type,tag> const & value ) noexcept
     { return strconv::detail::compose_preconv<char_type>(*value); }

