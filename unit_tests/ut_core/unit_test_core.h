#pragma once

#include <span>
#include <numeric>
#include <stdexcept>

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include <iostream>

// #define CONSTEXPR_TEST( a ) 
//   if( !(a) ) throw

//BOOST_TEST_STRINGIZE( P )
    
constexpr bool constexpr_or_runtime_test( bool expression, std::string_view str_expr, std::string_view file, int line  )
  {
  if( std::is_constant_evaluated())
    {
     if( !(expression) )
       throw;
    return expression;
    }
  else
    {
    report_assertion(
        ::boost::test_tools::assertion_result(expression),
        BOOST_TEST_LAZY_MSG( str_expr ),
        ::boost::unit_test::basic_cstring<char const>(file.data(), file.size()),
        //BOOST_TEST_L(__FILE__),//
        static_cast<std::size_t>(line),
        ::boost::test_tools::tt_detail::CHECK,
        ::boost::test_tools::tt_detail::CHECK_MSG,
        0 );
    return expression;
    }
  }

#define CONSTEXPR_TEST( a ) \
  constexpr_or_runtime_test(a, BOOST_STRINGIZE( a ), __FILE__, __LINE__ )
    
// ::boost::unit_test::basic_cstring<char const> s;
void dump( auto const & result )
  {
  for( auto el : result )
    std::cout << el << ",";
  std::cout << std::endl;
  }

  
struct non_trivial
  {
  int value_;
  
  constexpr non_trivial() noexcept : value_{0x1fff'ffff} {}
  
  constexpr non_trivial( int value ) noexcept : value_{value} {}
  
  constexpr non_trivial( non_trivial && r ) noexcept
      : value_{std::exchange(r.value_, 0x1fff'ffff)}
    { }
    
  constexpr non_trivial( non_trivial const & r) noexcept
      : value_{r.value_}
    { }
    
  constexpr non_trivial & operator =( non_trivial && r ) noexcept
    {
    value_ = std::exchange(r.value_, 0x1fff'ffff);
    return *this;
    }
    
  constexpr non_trivial & operator =( non_trivial const & r ) noexcept
    {
    value_ = r.value_;
    return *this;
    }
  
  constexpr non_trivial & operator ++() noexcept { ++value_; return *this; }
  constexpr ~non_trivial() {}
  constexpr bool operator ==( non_trivial const & r ) const noexcept = default;
  constexpr bool operator ==( int r ) const noexcept { return value_ == r; }
  };
  

inline std::ostream & operator <<( std::ostream & stream, non_trivial const & v )
  {
  stream << v.value_;
  return stream;
  }
  
struct non_trivial_ptr
  {
  int * value_;
  
  constexpr non_trivial_ptr() noexcept : value_{} {}
  
  constexpr non_trivial_ptr ( int value ) noexcept : value_{ new int{value} } {}
  
  constexpr non_trivial_ptr ( non_trivial_ptr && r ) noexcept
      : value_{std::exchange(r.value_, nullptr)}
    { }
    
  constexpr non_trivial_ptr ( non_trivial_ptr const & r) noexcept
   : value_{}
    { 
    if(r.value_ != nullptr )
      value_ = new int( *r.value_);
    }
    
  constexpr non_trivial_ptr & operator =( non_trivial_ptr && r ) noexcept
    {
    if(value_ !=nullptr)
      delete value_;

    value_ = std::exchange(r.value_, nullptr);
    return *this;
    }
    
  constexpr non_trivial_ptr & operator =( non_trivial_ptr const & r ) noexcept
    {
    if(value_ !=nullptr)
      {
      delete value_;
      value_ = nullptr;
      }
    if(r.value_ != nullptr )
      value_ = new int( *r.value_ );
    return *this;
    }
  
  constexpr non_trivial_ptr & operator ++() noexcept { assert(value_ != nullptr); ++*value_; return *this; }
  constexpr ~non_trivial_ptr()
    {
    if( value_ != nullptr )
      delete value_;
    }
    
  constexpr bool operator ==( non_trivial_ptr const & r ) const noexcept 
    {
    if( value_ != nullptr && r.value_ != nullptr )
      return *value_ == *r.value_;
      
    return value_ == nullptr && r.value_ == nullptr;
    }
  constexpr bool operator ==( int r ) const noexcept 
    {
    if( value_ != nullptr )
      return *value_ == r; 
    return false;
    }
  };

struct non_trivial_ptr_except
  {
  non_trivial_ptr obj;
  constexpr non_trivial_ptr_except() noexcept(false) = default;
  constexpr non_trivial_ptr_except ( int value ) noexcept(false) : obj{value} {}
  constexpr non_trivial_ptr_except ( non_trivial_ptr_except && r ) noexcept(false) = default;
  constexpr non_trivial_ptr_except ( non_trivial_ptr_except const & r) noexcept(false) = default;
  constexpr non_trivial_ptr_except & operator =( non_trivial_ptr_except && r ) noexcept(false) = default;
  constexpr non_trivial_ptr_except & operator =( non_trivial_ptr_except const & r ) noexcept(false) = default;
  constexpr ~non_trivial_ptr_except() = default;
  
  constexpr non_trivial_ptr_except & operator ++() noexcept { ++obj; return *this; }
  constexpr bool operator ==( non_trivial_ptr_except const & ) const noexcept = default;
  };
  
struct non_trivial_ptr_except_copy
  {
  non_trivial_ptr obj;
  constexpr non_trivial_ptr_except_copy() noexcept(false) = default;
  constexpr non_trivial_ptr_except_copy ( int value ) noexcept(false) : obj{value} {}
  constexpr non_trivial_ptr_except_copy ( non_trivial_ptr_except_copy const & r) noexcept(false) = default;
  constexpr non_trivial_ptr_except_copy & operator =( non_trivial_ptr_except_copy const & r ) noexcept(false) = default;
  constexpr ~non_trivial_ptr_except_copy() = default;
  
  constexpr non_trivial_ptr_except_copy & operator ++() noexcept { ++obj; return *this; }
  constexpr bool operator ==( non_trivial_ptr_except_copy const & ) const noexcept = default;
  };
  
//for testing strong exception guarantee
template<typename Super, int throw_at_value>
struct non_trivial_throwing_tmpl : public Super
  {
  using super_type = Super;
  constexpr non_trivial_throwing_tmpl()
    noexcept(std::is_nothrow_default_constructible_v<super_type>) = default;
  
  constexpr non_trivial_throwing_tmpl ( int value )
    noexcept(std::is_nothrow_constructible_v<super_type,int>)
    : super_type{value} 
    {}
    
  constexpr non_trivial_throwing_tmpl ( non_trivial_throwing_tmpl const & r)
      : super_type{r}
    {
    if constexpr(not noexcept(super_type{r}))
      if( operator==(throw_at_value))
        throw std::runtime_error{""};
    }
  
  constexpr non_trivial_throwing_tmpl ( non_trivial_throwing_tmpl && r)
    noexcept(noexcept(super_type{std::move(r)}))
      : super_type{std::move(r)}
    {
    if constexpr(not noexcept(super_type{std::move(r)}))
      if( operator==(throw_at_value))
        throw std::runtime_error{""};
    }
    
  using super_type::operator==;
  using super_type::operator++;
  
  void set_value( int value ) 
    { super_type::operator =(value); }
    
  constexpr non_trivial_throwing_tmpl & operator =( non_trivial_throwing_tmpl const & r )
    {
    super_type::operator=(r);
    if constexpr(not noexcept(super_type::operator=(r)))
      if( operator==(throw_at_value))
        throw std::runtime_error{""};
    return *this;
    }
    
  constexpr non_trivial_throwing_tmpl & operator =( non_trivial_throwing_tmpl && r )
    noexcept(noexcept(super_type::operator=(std::move(r))))
    {
    super_type::operator=(r);
    if constexpr(not noexcept(super_type::operator=(std::move(r))))
      if( operator==(throw_at_value))
        throw std::runtime_error{""};
    return *this;
    }
  constexpr ~non_trivial_throwing_tmpl() = default;
  };

template<int throw_at_value>
using non_trivial_throwing = non_trivial_throwing_tmpl<non_trivial_ptr_except,throw_at_value>;

template<int throw_at_value>
using non_trivial_throwing_copy = non_trivial_throwing_tmpl<non_trivial_ptr_except_copy,throw_at_value>;

std::ostream & operator <<( std::ostream & stream, non_trivial_ptr const & v );
std::ostream & operator <<( std::ostream & stream, non_trivial_ptr_except const & v );
std::ostream & operator <<( std::ostream & stream, non_trivial_ptr_except_copy const & v );
