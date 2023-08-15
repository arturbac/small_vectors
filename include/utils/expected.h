#pragma once

#include "static_call_operator.h"
#include <utility>
#include <concepts>
#include <type_traits>
#include <memory>
#include <cassert>
#include <functional>
//__cpp_lib_expected 	202211L

namespace cxx23
{
template<typename T, typename E >
class expected;

namespace concepts
  {
  template<typename T,typename V>
  concept not_same_as = !std::same_as<T,V>;
  
  template<typename T>
  concept not_expected = !std::same_as<T, expected<typename T::value_type,typename T::error_type>>;
  }


template<typename E >
class unexpected
  {
  using error_type = E;
  error_type error_;
  
  constexpr unexpected( unexpected const & ) noexcept(std::is_nothrow_copy_constructible_v<error_type>) = default;
  constexpr unexpected( unexpected && ) noexcept(std::is_nothrow_move_constructible_v<error_type>) = default;
  
	template<typename Err = E >
	requires requires
    {
    requires concepts::not_expected<std::remove_cvref_t<Err>>;
    requires concepts::not_same_as<std::remove_cvref_t<Err>,std::in_place_t>;
    requires std::constructible_from<error_type, Err>;
    }
  constexpr explicit unexpected( Err&& e )
      noexcept(noexcept(std::forward<Err>(e)))
      : error_{std::forward<Err>(e)}
    {}
	
  template<typename ... Args >
  requires std::constructible_from<E, Args...>
  constexpr explicit unexpected( std::in_place_t, Args&&... args )
      noexcept(noexcept(std::forward<Args...>(args...)))
      : error_{std::forward<Args>(args)...}
    {}
	
  template<typename U, typename ... Args >
  constexpr explicit unexpected( std::in_place_t,
                               std::initializer_list<U> il, Args&&... args )
    : error_{il, std::forward<Args>(args)...}
    {}
    
  constexpr error_type const & error() const & noexcept { return error_; }
  constexpr error_type& error() & noexcept  { return error_; }
  constexpr error_type const && error() const&& noexcept  { return std::move(error_); }
  constexpr error_type&& error() && noexcept  { return std::move(error_); }
  
  constexpr void swap( unexpected& other ) noexcept(std::is_nothrow_swappable_v<E>)
      requires std::is_swappable_v<error_type>
    { std::swap(error_,other,error_); }
  
  template<typename E2 >
  friend constexpr bool operator==( unexpected const & x, unexpected<E2> const & y )
    { return x.error() == y.error(); }
    
  friend constexpr void
  swap( unexpected& x, unexpected& y ) noexcept(noexcept(x.swap(y)))
    { x.swap(y); }
  };

template<typename E >
unexpected(E) -> unexpected<E>;

struct unexpect_t { explicit unexpect_t() noexcept = default; };
inline constexpr unexpect_t unexpect;

template<typename E>
class bad_expected_access;

template<>
class bad_expected_access<void> : public std::exception
  {
  protected:
    bad_expected_access() = default;
    bad_expected_access( bad_expected_access const & ) noexcept = default;
    bad_expected_access( bad_expected_access && ) noexcept = default;
    ~bad_expected_access() override = default;

  bad_expected_access & operator=( bad_expected_access const & r) noexcept = default;
  bad_expected_access & operator=( bad_expected_access && r) noexcept = default;
public:
  [[nodiscard]]
  char const * what() const noexcept override
    { return "access to expected without value"; }
    
  };

template<typename E>
class bad_expected_access : public bad_expected_access<void>
  {
public:
  using error_type = E;
  
  error_type error_;
  
  explicit bad_expected_access( E e ) 
    : error_{ std::move(e) }
    {}
  
  [[nodiscard]]
  error_type const & error() const & noexcept
     { return error_; }
   
  [[nodiscard]]
  error_type & error() & noexcept
    { return error_; }
  
  [[nodiscard]]
  error_type const && error() const && noexcept
    { return std::move(error_); }
  
  [[nodiscard]]
  error_type && error() && noexcept
    { return std::move(error_); }
  };

namespace detail
  {
  template<typename EX, typename F>
  constexpr auto and_then( EX && ex, F && f );
  
  template<typename EX, typename F >
  constexpr auto transform( EX && ex, F && f );
  
  template<typename EX, typename F >
  constexpr auto or_else( EX && ex, F&& f );
  
  template<typename EX, typename F>
  constexpr auto transform_error( EX && ex, F && f );
  }
  
template<typename T, typename E>
class expected
  {
  using value_type = T;
  using error_type = E;
  using unexpected_type = unexpected<E>;
  template<typename U >
  using rebind = expected<U, error_type>;
  using bad_access_exception = bad_expected_access<std::decay_t<error_type>>;
  static constexpr bool types_are_nothrow_move_constructible = std::is_nothrow_move_constructible_v<value_type> 
                                                            && std::is_nothrow_move_constructible_v<error_type>;
  static constexpr bool types_are_trivially_copy_constructible = std::is_trivially_copy_constructible_v<value_type> 
                                                            && std::is_trivially_copy_constructible_v<error_type>;
  static constexpr bool types_are_trivially_move_constructible = std::is_trivially_move_constructible_v<value_type> 
                                                            && std::is_trivially_move_constructible_v<error_type>;
  static constexpr bool value_copy_constructible = std::is_copy_constructible_v<value_type>;
  static constexpr bool value_move_constructible = std::is_move_constructible_v<value_type>;
  static constexpr bool error_copy_constructible = std::is_copy_constructible_v<error_type>;
  static constexpr bool error_move_constructible = std::is_move_constructible_v<error_type>;
  union
    {
    value_type value_;
    error_type error_;
    };
  bool has_value_;

  constexpr expected() noexcept(std::is_nothrow_default_constructible_v<T>)
  requires std::is_default_constructible_v<T>
    : value_{}, has_value_{true}
    {}
  
  constexpr expected(expected const&)
      noexcept(types_are_nothrow_move_constructible)
      requires types_are_trivially_copy_constructible
    = default;
  
  constexpr expected(expected const& rh)
      noexcept(types_are_nothrow_move_constructible)
      requires value_copy_constructible && error_copy_constructible && (!types_are_trivially_copy_constructible)
          : has_value_( rh.has_value_)
      {
      if(has_value_) [[likely]]
        std::construct_at(std::addressof(value_), rh.value_);
      else
        std::construct_at(std::addressof(error_), rh.error_);
      }
      
  constexpr expected(expected &&)
      noexcept(types_are_nothrow_move_constructible)
      requires types_are_trivially_move_constructible = default;
    
  constexpr expected(expected && rh)
      noexcept(types_are_nothrow_move_constructible)
      requires value_copy_constructible && error_copy_constructible && (!types_are_trivially_move_constructible)
          : has_value_( rh.has_value_)
      {
      if(has_value_) [[likely]]
        std::construct_at(std::addressof(value_), std::move(rh.value_));
      else
        std::construct_at(std::addressof(error_), std::move(rh.error_));
      }
      
  template<typename ... Args >
  requires std::constructible_from<E, Args...>
  constexpr explicit expected( std::in_place_t, Args&&... args )
      noexcept(noexcept(std::forward<Args...>(args...)))
      : value_{std::forward<Args>(args)...}, has_value_{true}
    {}
    
  template<typename U, typename ... Args >
  constexpr explicit expected( std::in_place_t,
                             std::initializer_list<U> il, Args&&... args )
      : value_{il, std::forward<Args>(args)...}, has_value_{true}
    {}
  
  template<typename ... Args >
  requires std::constructible_from<E, Args...>
  constexpr explicit expected( unexpect_t, Args&&... args )
      noexcept(noexcept(std::forward<Args...>(args...)))
      : error_{std::forward<Args>(args)...}, has_value_{}
    {}
  
  template<typename U, typename ... Args >
  constexpr explicit expected( unexpect_t,
                             std::initializer_list<U> il, Args&&... args )
      : error_{il, std::forward<Args>(args)...}, has_value_{}
    {}

  template<typename G >
  constexpr explicit(!std::is_convertible_v<G const&, E>)
    expected( unexpected<G> const & e )
      : error_{ e.error() }, has_value_{}
    {}
  
  template<typename G >
  constexpr explicit(!std::is_convertible_v<G, E>)
    expected( unexpected<G> && e )
      : error_{ std::forward<G>(e.error()) }, has_value_{}
    {}
    
  constexpr ~expected() 
    requires std::is_trivially_destructible_v<T> && std::is_trivially_destructible_v<error_type>
    = default;
  constexpr ~expected() 
    requires (!std::is_trivially_destructible_v<T> || !std::is_trivially_destructible_v<error_type>)
      {
      if(has_value_) [[likely]]
        std::destroy_at(std::addressof(value_));
      else
        std::destroy_at(std::addressof(error_));
      }
  
  [[nodiscard]]
  constexpr value_type const * operator->() const noexcept
    {
    assert(has_value());
    return std::addressof(value_);
    }
  
  [[nodiscard]]
  constexpr value_type * operator->() noexcept
    {
    assert(has_value());
    return std::addressof(value_);
    }
  
  [[nodiscard]]
  constexpr value_type const & operator*() const & noexcept
    {
    assert(has_value());
    return value_;
    }
  
  [[nodiscard]]
  constexpr value_type & operator*() & noexcept
    {
    assert(has_value());
    return value_;
    }
  
  [[nodiscard]]
  constexpr value_type const && operator*() const && noexcept
    {
    assert(has_value());
    return std::move(value_);
    }
  
  [[nodiscard]]
  constexpr value_type && operator*() && noexcept
    {
    assert(has_value());
    return std::move(value_);
    }
  
  [[nodiscard]]
  constexpr explicit operator bool() const noexcept
    { return has_value_; }
  
  [[nodiscard]]
  constexpr bool has_value() const noexcept
    { return has_value_; }
  
  [[nodiscard]]
  constexpr value_type & value() &
    requires error_copy_constructible
    {
    if(has_value()) [[likely]]
      return value_;
    else
      throw bad_access_exception{ error_ };
    }
  
  [[nodiscard]]
  constexpr value_type const & value() const &
    requires error_copy_constructible
    {
    if(has_value()) [[likely]]
      return value_;
    else
      throw bad_access_exception{ error_ };
    }
  
  [[nodiscard]]
  constexpr value_type && value() &&
    requires error_copy_constructible || std::is_constructible_v<error_type, decltype(std::move(error_))>
    {
    if(has_value()) [[likely]]
      return value_;
    else
      throw bad_access_exception{ std::move(error_) };
    }
  
  [[nodiscard]]
  constexpr value_type const && value() const &&
    requires error_copy_constructible || std::is_constructible_v<error_type, decltype(std::move(error_))>
    {
    if(has_value()) [[likely]]
      return value_;
    else
      throw bad_access_exception{ std::move(error_) };
    }
    
  [[nodiscard]]
  constexpr error_type const & error() const & noexcept
    {
    assert(!has_value());
    return error_;
    }
    
  [[nodiscard]]
  constexpr error_type & error() & noexcept
    {
    assert(!has_value());
    return error_;
    }
    
  [[nodiscard]]
  constexpr error_type const && error() const && noexcept
    {
    assert(!has_value());
    return std::move(error_);
    }
  
  [[nodiscard]]
  constexpr error_type && error() && noexcept
    {
    assert(!has_value());
    return std::move(error_);
    }
    
  template<typename U>
  [[nodiscard]]
  constexpr value_type value_or( U && default_value ) const &
      noexcept( std::is_nothrow_copy_constructible_v<value_type> && std::is_nothrow_convertible_v<U,value_type> )
      requires value_copy_constructible && std::is_convertible_v<U,value_type>
    {
    return has_value() ? value_ : static_cast<value_type>(std::forward<U>(default_value));
    }
    
  template<typename U>
  [[nodiscard]]
  constexpr value_type value_or( U && default_value ) &&
      noexcept( std::is_nothrow_move_constructible_v<value_type> && std::is_nothrow_convertible_v<U,value_type> )
      requires value_move_constructible && std::is_convertible_v<U,value_type>
    {
    return has_value() ? std::move(value_) : static_cast<value_type>(std::forward<U>(default_value));
    }
    
  template<typename F>
  constexpr auto and_then( F && f ) &
      requires error_copy_constructible
    { return detail::and_then(*this,std::forward<F>(f)); }
  
  template<typename F>
  constexpr auto and_then( F && f ) const &
      requires error_copy_constructible
    { return detail::and_then(*this,std::forward<F>(f)); }
    
  template<typename F>
  constexpr auto and_then( F && f ) &&
      requires error_move_constructible
    { return detail::and_then(*this,std::forward<F>(f)); }
    
  template<typename F>
  constexpr auto and_then( F && f ) const &&
      requires error_move_constructible
    { return detail::and_then(*this,std::forward<F>(f)); }
    
  template<typename F>
  constexpr auto transform( F&& f ) &
      requires error_copy_constructible
    {
    return detail::transform(*this, std::forward<F>(f));
    }

  template<typename F>
  constexpr auto transform( F&& f ) const &
      requires error_copy_constructible
    {
    return detail::transform(*this, std::forward<F>(f));
    }
    
  template<typename F>
  constexpr auto transform( F&& f ) &&
      requires error_move_constructible
    {
    return detail::transform(*this, std::forward<F>(f));
    }
    
  template<typename F>
  constexpr auto transform( F&& f ) const &&
      requires error_move_constructible
    {
    return detail::transform(*this, std::forward<F>(f));
    }
  
  template<typename F>
  constexpr auto or_else( F&& f ) &
      requires value_copy_constructible
    {
    return detail::or_else(*this,std::forward<F>(f));
    }
    
  template<typename F>
  constexpr auto or_else( F&& f ) const &
      requires value_copy_constructible
    {
    return detail::or_else(*this,std::forward<F>(f));
    }
  
  template<typename F>
  constexpr auto or_else( F&& f ) &&
      requires value_move_constructible
    {
    return detail::or_else(*this,std::forward<F>(f));
    }
    
  template<typename F>
  constexpr auto or_else( F&& f ) const &&
      requires value_move_constructible
    {
    return detail::or_else(*this,std::forward<F>(f));
    }
    
  template<typename F>
  constexpr auto transform_error( F&& f ) &
    { return detail::transform_error(*this, std::forward<F>(f)); }
    
  template<typename F>
  constexpr auto transform_error( F&& f ) const &
    { return detail::transform_error(*this, std::forward<F>(f)); }
    
  template<typename F>
  constexpr auto transform_error( F&& f ) &&
    { return detail::transform_error(*this, std::forward<F>(f)); }
    
  template<typename F>
  constexpr auto transform_error( F&& f ) const &&
    { return detail::transform_error(*this, std::forward<F>(f)); }
    
  template<typename ... Args>
  constexpr value_type & emplace( Args&&... args ) noexcept
      requires std::is_nothrow_constructible_v<value_type, Args...>
    {
    if(has_value_) [[likely]]
      {
      if constexpr (!std::is_trivially_destructible_v<value_type>)
        std::destroy_at(std::addressof(value_));
      }
    else
      {
      if constexpr (!std::is_trivially_destructible_v<error_type>)
        std::destroy_at(std::addressof(error_));
      }
    return *std::construct_at(std::addressof(value_), std::forward<Args>(args)...);
    }
  
  template<typename U, typename ... Args>
  constexpr value_type & emplace( std::initializer_list<U> & il, Args &&... args ) noexcept
    {
    if(has_value_) [[likely]]
      {
      if constexpr (!std::is_trivially_destructible_v<value_type>)
        std::destroy_at(std::addressof(value_));
      }
    else
      {
      if constexpr (!std::is_trivially_destructible_v<error_type>)
        std::destroy_at(std::addressof(error_));
      }
    return *std::construct_at(std::addressof(value_), il, std::forward<Args>(args)...);
    }
    
  constexpr void swap( expected & other )
      noexcept( types_are_nothrow_move_constructible 
            && std::is_nothrow_swappable_v<value_type> && std::is_nothrow_swappable_v<error_type> )
      requires requires
       {
       requires std::is_swappable_v<value_type>;
       requires std::is_swappable_v<error_type>;
       requires value_move_constructible;
       requires error_move_constructible;
       requires std::is_nothrow_move_constructible_v<value_type> || std::is_nothrow_move_constructible_v<error_type>;
      }
    {
    if constexpr (std::is_nothrow_move_constructible_v<error_type>) 
      {
      error_type temp(std::move(other.error_));
      std::destroy_at(std::addressof(other.error_));
      try 
        {
        std::construct_at(std::addressof(other.value_), std::move(value_));
        std::destroy_at(std::addressof(value_));
        std::construct_at(std::addressof(error_), std::move(temp));
        }
      catch(...) 
        {
        std::construct_at(std::addressof(other.error_), std::move(temp));
        throw;
        }
      }
    else 
      {
      value_type temp(std::move(value_));
      std::destroy_at(std::addressof(value_));
      try 
        {
        std::construct_at(std::addressof(error_), std::move(other.error_));
        std::destroy_at(std::addressof(other.error_));
        std::construct_at(std::addressof(other.value_), std::move(temp));
        }
      catch(...) 
        {
        std::construct_at(std::addressof(value_), std::move(temp));
        throw;
        }
      }
    }
    
  template<typename T2, typename E2>
  requires requires
    {
    requires (!std::is_void_v<T2>);
    requires std::equality_comparable_with<value_type,T2>;
    requires std::equality_comparable_with<error_type,E2>;
    }
  friend constexpr bool operator==( expected const & lhs, expected<T2, E2> const & rhs )
    noexcept( noexcept(lhs.value() == rhs.value()) && noexcept(lhs.error() == rhs.error()))
    {
    if(lhs.has_value() == rhs.has_value())
      {
      if(lhs.has_value())
        return lhs.value() == rhs.value();
      else 
        return lhs.error() == rhs.error();
      }
    else
      return false;
    }
  template<std::equality_comparable_with<value_type> T2>
  friend constexpr bool operator==( expected const & x, T2 const & val )
    noexcept( noexcept(x.value() == val))
    {
    return x.has_value() && (x.value() == val);
    }
    
  friend constexpr void swap( expected & lhs, expected & rhs ) noexcept(noexcept(lhs.swap(rhs)))
    { lhs.swap(rhs); }
  };

namespace detail
  {
  template<typename EX, typename F>
  constexpr auto and_then( EX && ex, F && f )
    {
    using U = std::remove_cvref_t<std::invoke_result_t<F, decltype(std::forward<EX>(ex).value())>>;
    if(ex.has_value())
      return std::invoke(std::forward<F>(f), std::forward<EX>(ex).value());
    else
      return U(unexpect, std::forward<EX>(ex).error());
    }
    
  template<typename EX, typename F >
  constexpr auto transform( EX && ex, F && f )
    {
    using U = std::remove_cv_t<std::invoke_result_t<F, decltype(std::forward<EX>(ex).value())>>;
    static_assert(!std::is_void_v<U>, "FIXME unimplemented");
    using error_type = typename EX::error_type;
    if(ex.has_value())
      return expected<U,error_type>{std::in_place, std::invoke(std::forward<F>(f), std::forward<EX>(ex).value())};
    else
      return expected<U,error_type>(unexpect, std::forward<EX>(ex).error());
    }
    
  template<typename EX, typename F >
  constexpr auto or_else( EX && ex, F&& f )
    {
    using G = std::remove_cvref_t<std::invoke_result_t<F, decltype(std::forward<EX>(ex).error())>>;
    static_assert(std::is_same_v<typename G::value_type, typename EX::value_type>);
    if (ex.has_value())
       return G(std::in_place, std::forward<EX>(ex).value());
    else
      return std::invoke(std::forward<F>(f),  std::forward<EX>(ex).error());
    }
    
  template<typename EX, typename F>
  constexpr auto transform_error( EX && ex, F && f )
    {
    using G = std::remove_cv_t<std::invoke_result_t<F, decltype(std::forward<EX>(ex).error())>>;
    using value_type = typename EX::value_type;
    if(ex.has_value())
      return expected<value_type, G>(std::in_place, std::forward<EX>(ex).value());
    else
      return expected<value_type, G>(unexpect, std::invoke(std::forward<F>(f), std::forward<EX>(ex).error()));
    }
  }

}
