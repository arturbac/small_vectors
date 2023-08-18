#pragma once

#include "static_call_operator.h"
#include <utility>
#include <concepts>
#include <type_traits>
#include <memory>
#include <cassert>
#include <functional>
#include <utils/static_call_operator.h>

#if !defined(SMALL_VECTORS_ENABLE_CUSTOM_EXCPECTED) \
      && defined(__cpp_lib_expected) && __cpp_lib_expected >= 202211L
#include <expected>

namespace cxx23
{
  using std::unexpected;
  using std::expected;
  using std::unexpect_t;
  using std::unexpect;
  using std::bad_expected_access;
  using std::in_place_t;
  using std::in_place;
}
#else

namespace cxx23
{
template<typename T, typename E>
class expected;
template<typename E>
class unexpected;

struct unexpect_t { explicit unexpect_t() noexcept = default; };
inline constexpr unexpect_t unexpect;

using std::in_place_t;
using std::in_place;

namespace concepts
  {
  template<typename T,typename V>
  concept not_same_as = !std::same_as<T,V>;
    
  template<typename T>
  concept is_unexpected = 
    requires
    {
    typename T::error_type;
    requires std::same_as<T, unexpected<typename T::error_type>>;
    };

  template<typename E>
  concept unexpected_constaint = 
    requires 
      {
      requires !std::is_array_v<E>;
      requires std::is_object_v<E>;
      requires !is_unexpected<E>;
      requires !std::is_const_v<E>;
      requires !std::is_volatile_v<E>;
      requires std::is_destructible_v<E>;
      };
      
  template<typename T>
  concept is_expected = 
    requires
    {
    typename T::value_type;
    typename T::error_type;
    requires std::same_as<T, expected<typename T::value_type,typename T::error_type>>;
    };
    
  template<typename T>
  concept not_expected = !is_expected<T>;
  
  template<typename T>
  concept expected_constaint = 
    requires 
      {
      requires !std::is_array_v<T>;
      requires !std::is_reference_v<T>;
      requires !std::is_function_v<T>;
      requires std::is_destructible_v<T> || std::is_void_v<std::remove_cv_t<T>>;
      requires !is_unexpected<T>;
      requires !std::same_as<T,std::in_place_t>;
      requires !std::same_as<T,unexpect_t>;
      };
      
  template<typename T, typename E>
  concept swap_constraints = 
    requires
         {
         requires std::is_swappable_v<T> || std::is_void_v<T>;
         requires std::is_swappable_v<E>;
         requires std::is_move_constructible_v<T> || std::is_void_v<T>;
         requires std::is_move_constructible_v<E>;
         requires std::is_nothrow_move_constructible_v<T> || std::is_void_v<T> || std::is_nothrow_move_constructible_v<E>;
        };
  }
namespace detail
{
  template<typename T, typename E>
  inline constexpr bool swap_no_throw =
                       (std::is_nothrow_move_constructible_v<T> || std::is_void_v<T>)
                    && std::is_nothrow_move_constructible_v<E>
                    && (std::is_nothrow_swappable_v<T> || std::is_void_v<T>)
                    && std::is_nothrow_swappable_v<E>;
}

template<typename E>
class unexpected
  {
public:
  static_assert(concepts::unexpected_constaint<E>, "not a valid type for unexpected error type");
  using error_type = E;
  
private:
  error_type error_;
  
public:
  constexpr unexpected( unexpected const & )
    noexcept(std::is_nothrow_copy_constructible_v<error_type>) = default;
    
  constexpr unexpected( unexpected && )
    noexcept(std::is_nothrow_move_constructible_v<error_type>) = default;
  
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
    
  constexpr error_type const & error() const & noexcept
     { return error_; }
     
  constexpr error_type & error() & noexcept
    { return error_; }
    
  constexpr error_type const && error() const&& noexcept
    { return std::move(error_); }
    
  constexpr error_type && error() && noexcept
    { return std::move(error_); }
  
  constexpr void swap( unexpected& other ) noexcept(std::is_nothrow_swappable_v<error_type>)
      requires std::swappable<error_type>
    { std::swap(error_,other.error_); }
  
  template<std::equality_comparable_with<error_type> E2 >
  friend constexpr bool operator==( unexpected const & x, unexpected<E2> const & y )
    noexcept(noexcept(x.error() == y.error()))
    { return x.error() == y.error(); }
    
  friend constexpr void
  swap( unexpected& x, unexpected& y )
      noexcept(std::is_nothrow_swappable_v<error_type>)
      requires std::swappable<error_type>
    { x.swap(y); }
  };

template<typename E >
unexpected(E) -> unexpected<E>;


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
    { return "access to expected value without value"; }
    
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
  
  template<concepts::is_expected EX>
  inline constexpr void swap_dispatch( EX & l, EX & r );
  
  template<typename E,typename T>
  struct expected_storage_t
    {
    union
      {
      T value;
      E error;
      };
    };
    
  template<typename E>
  struct expected_storage_t<E,void>
    {
    union
      {
      E error;
      };
    };
    
  struct swap_expected_t;
  }
  
template<typename T, typename E>
class expected
  {
public:
  static_assert(concepts::unexpected_constaint<E>, "not a valid type for expected error type");
  static_assert(concepts::expected_constaint<T>, "not a valid type for expected value type");
  using value_type = T;
  using error_type = E;
  using unexpected_type = unexpected<E>;
  template<typename U >
  using rebind = expected<U, error_type>;
  friend struct detail::swap_expected_t;
  
private:
  using bad_access_exception = bad_expected_access<std::decay_t<error_type>>;

  static constexpr bool value_nothrow_move_constructible = 
                            std::is_nothrow_move_constructible_v<value_type>;
  static constexpr bool value_trivially_copy_constructible =
                            std::is_trivially_copy_constructible_v<value_type>;
  
  static constexpr bool error_trivially_copy_constructible = std::is_trivially_copy_constructible_v<error_type>;
  static constexpr bool error_nothrow_move_constructible = std::is_nothrow_move_constructible_v<error_type>;
  
  static constexpr bool both_are_nothrow_move_constructible = 
                        value_nothrow_move_constructible && error_nothrow_move_constructible;
  
  static constexpr bool both_are_trivially_copy_constructible = 
                        value_trivially_copy_constructible && error_trivially_copy_constructible;
  
  static constexpr bool both_are_trivially_move_constructible = 
        std::is_trivially_move_constructible_v<value_type>
         && std::is_trivially_move_constructible_v<error_type>;
  static constexpr bool value_copy_constructible = std::is_copy_constructible_v<value_type>;
  static constexpr bool value_move_constructible = std::is_move_constructible_v<value_type>;
  static constexpr bool error_copy_constructible = std::is_copy_constructible_v<error_type>;
  static constexpr bool error_move_constructible = std::is_move_constructible_v<error_type>;
  
  union
    {
    T value_;
    E error_;
    };
  bool has_value_;
  
public:

  constexpr expected()
      noexcept(std::is_nothrow_default_constructible_v<value_type>)
      requires std::is_default_constructible_v<value_type>
        : value_{}, has_value_{true}
    {}

  constexpr expected(expected const&) noexcept
      requires value_trivially_copy_constructible && error_trivially_copy_constructible = default;
      
  constexpr expected(expected const& rh)
      noexcept(value_nothrow_move_constructible && error_nothrow_move_constructible)
      requires (value_copy_constructible) && error_copy_constructible 
           && (!both_are_trivially_copy_constructible)
          : has_value_( rh.has_value_)
    {
    if(has_value_) [[likely]]
      std::construct_at(std::addressof(value_), rh.value_);
    else
      std::construct_at(std::addressof(error_), rh.error_);
    }
      
  constexpr expected(expected &&) noexcept
      requires both_are_trivially_move_constructible = default;
    
  constexpr expected(expected && rh)
      noexcept(both_are_nothrow_move_constructible)
      requires value_move_constructible && error_move_constructible && (!both_are_trivially_move_constructible)
          : has_value_( rh.has_value_)
    {
    if(has_value_) [[unlikely]]
      std::construct_at(std::addressof(value_), std::move(rh.value_));
    else
      std::construct_at(std::addressof(error_), std::move(rh.error_));
    }

  template<typename U = T>
  constexpr explicit(!std::is_convertible_v<U, T>)
    expected( U && v )
        noexcept( std::is_nothrow_constructible_v<decltype(std::forward<U>(v))> )
      : value_{ std::forward<U>(v) }, has_value_{true}
    {}
    
  template<typename ... Args >
    requires std::constructible_from<value_type, Args...>
  constexpr explicit expected( std::in_place_t, Args&&... args )
      noexcept(noexcept(std::forward<Args...>(args...)))
      : value_{ std::forward<Args>(args)...}, has_value_{true}
    {}
    
  template<typename U, typename ... Args >
    requires std::constructible_from<value_type, std::initializer_list<U> &, Args...>
  constexpr explicit expected( std::in_place_t, std::initializer_list<U> il, Args&&... args )
      : value_{ value_type{il, std::forward<Args>(args)...} }, has_value_{true}
    {}

  template<typename ... Args >
    requires std::constructible_from<error_type, Args...>
  constexpr explicit expected( unexpect_t, Args&&... args )
      noexcept(noexcept(std::forward<Args...>(args...)))
      : error_{ std::forward<Args>(args)...}, has_value_{}
    {}
  
  template<typename U, typename ... Args >
    requires std::constructible_from<error_type, std::initializer_list<U> &, Args...>
  constexpr explicit expected( unexpect_t, std::initializer_list<U> il, Args&&... args )
      : error_{ error_type{il, std::forward<Args>(args)...}}, has_value_{}
    {}

  template<std::convertible_to<error_type> G >
  constexpr explicit(!std::is_convertible_v<G const&, error_type>)
    expected( unexpected<G> const & e )
      : error_{ e.error() }, has_value_{}
    {}
  
  template<std::convertible_to<error_type> G >
  constexpr explicit(!std::is_convertible_v<G, E>)
    expected( unexpected<G> && e )
      : error_{ std::forward<G>(e.error()) }, has_value_{}
    {}
    
  constexpr ~expected() 
    requires std::is_trivially_destructible_v<value_type>
              && std::is_trivially_destructible_v<error_type> = default;

  constexpr ~expected() 
    requires (!std::is_trivially_destructible_v<value_type> || !std::is_trivially_destructible_v<error_type>)
      {
      if(has_value_) [[likely]]
        std::destroy_at(std::addressof( value_ ));
      else
        std::destroy_at(std::addressof( error_ ));
      }
  
  [[nodiscard]]
  constexpr value_type const * operator->() const noexcept
    {
    assert(has_value_);
    return std::addressof(value_);
    }
  
  [[nodiscard]]
  constexpr value_type * operator->() noexcept
    {
    assert(has_value_);
    return std::addressof(value_);
    }
  
  [[nodiscard]]
  constexpr value_type const & operator*() const & noexcept
    {
    assert(has_value_);
    return value_;
    }
  
  [[nodiscard]]
  constexpr value_type & operator*() & noexcept
    {
    assert(has_value_);
    return value_;
    }
  
  [[nodiscard]]
  constexpr value_type const && operator*() const && noexcept
    {
    assert(has_value_);
    return std::move(value_);
    }
  
  [[nodiscard]]
  constexpr value_type && operator*() && noexcept
    {
    assert(has_value_);
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
    if(has_value_) [[likely]]
      return value_;
    else
      throw bad_access_exception{ error_ };
    }
  
  [[nodiscard]]
  constexpr value_type const & value() const &
    requires error_copy_constructible
    {
    if(has_value_) [[likely]]
      return value_;
    else
      throw bad_access_exception{ error_ };
    }
  
  [[nodiscard]]
  constexpr value_type && value() &&
    requires (error_copy_constructible || error_move_constructible)
    {
    if(has_value_) [[likely]]
      return std::move(value_);
    else
      throw bad_access_exception{ std::move(error_) };
    }
  
  [[nodiscard]]
  constexpr value_type const && value() const &&
    requires (error_copy_constructible || error_move_constructible)
    {
    if(has_value_) [[likely]]
      return std::move(value_);
    else
      throw bad_access_exception{ std::move(error_) };
    }

  [[nodiscard]]
  constexpr error_type const & error() const & noexcept
    {
    assert(!has_value_);
    return error_;
    }
    
  [[nodiscard]]
  constexpr error_type & error() & noexcept
    {
    assert(!has_value_);
    return error_;
    }
    
  [[nodiscard]]
  constexpr error_type const && error() const && noexcept
    {
    assert(!has_value_);
    return std::move(error_);
    }
  
  [[nodiscard]]
  constexpr error_type && error() && noexcept
    {
    assert(!has_value_);
    return std::move(error_);
    }
    
  template<typename U>
  [[nodiscard]]
  constexpr value_type value_or( U && default_value ) const &
      noexcept( std::is_nothrow_copy_constructible_v<value_type> && std::is_nothrow_convertible_v<U,value_type> )
      requires value_copy_constructible && std::is_convertible_v<U,value_type>
    {
    return has_value_ ? value_ : static_cast<value_type>(std::forward<U>(default_value));
    }
    
  template<typename U>
  [[nodiscard]]
  constexpr value_type value_or( U && default_value ) &&
      noexcept( std::is_nothrow_move_constructible_v<value_type> && std::is_nothrow_convertible_v<U,value_type> )
      requires value_move_constructible && std::is_convertible_v<U,value_type>
    {
    return has_value_ ? std::move(value_) : static_cast<value_type>(std::forward<U>(default_value));
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
    { return detail::and_then( std::move(*this),std::forward<F>(f)); }
    
  template<typename F>
  constexpr auto and_then( F && f ) const &&
      requires error_move_constructible
    { return detail::and_then(std::move(*this),std::forward<F>(f)); }
    
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
    return detail::transform(std::move(*this), std::forward<F>(f));
    }
    
  template<typename F>
  constexpr auto transform( F&& f ) const &&
      requires error_move_constructible
    {
    return detail::transform(std::move(*this), std::forward<F>(f));
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
    return detail::or_else(std::move(*this),std::forward<F>(f));
    }
    
  template<typename F>
  constexpr auto or_else( F&& f ) const &&
      requires value_move_constructible
    {
    return detail::or_else(std::move(*this),std::forward<F>(f));
    }
    
  template<typename F>
  constexpr auto transform_error( F&& f ) &
    { return detail::transform_error(*this, std::forward<F>(f)); }
    
  template<typename F>
  constexpr auto transform_error( F&& f ) const &
    { return detail::transform_error(*this, std::forward<F>(f)); }
    
  template<typename F>
  constexpr auto transform_error( F&& f ) &&
    { return detail::transform_error(std::move(*this), std::forward<F>(f)); }
    
  template<typename F>
  constexpr auto transform_error( F&& f ) const &&
    { return detail::transform_error(std::move(*this), std::forward<F>(f)); }
    
  template<typename ... Args>
  constexpr auto /*value_type &*/ emplace( Args&&... args ) noexcept
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
      has_value_ = true;
      }
    return *std::construct_at(std::addressof(value_), std::forward<Args>(args)...);
    }
  
  template<typename U, typename ... Args>
  constexpr auto /*value_type &*/ emplace( std::initializer_list<U> & il, Args &&... args ) noexcept
    requires std::is_nothrow_constructible_v<value_type, std::initializer_list<U> &, Args...>
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
      has_value_ = true;
      }
    return *std::construct_at(std::addressof(value_), il, std::forward<Args>(args)...);
    }

  constexpr void swap( expected & other )
      noexcept( detail::swap_no_throw<T,E> )
      requires concepts::swap_constraints<T,E>
    {
    detail::swap_dispatch(*this, other);
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

  template<typename T2>
  requires requires
    {
    requires concepts::not_expected<T2>;
    requires std::equality_comparable_with<value_type,T2>;
    }
  friend constexpr bool operator==( expected const & x, T2 const & val )
    noexcept( noexcept(x.value() == val))
    {
    return x.has_value() && (x.value() == val);
    }
    
  template<std::equality_comparable_with<error_type> E2 >
  friend constexpr bool operator==( expected const & x, unexpected<E2> const & e )
    {
    if(!x.has_value())
      return x.error() == e.error();
    return false;
    }

  friend constexpr void swap( expected & lhs, expected & rhs ) noexcept(noexcept(lhs.swap(rhs)))
    { lhs.swap(rhs); }
  };

template<typename T, typename E>
  requires std::same_as<void,T>
class expected<T,E>
  {
public:
  using value_type = void;
  using error_type = E;
  using unexpected_type = unexpected<E>;
  template<typename U >
  using rebind = expected<U, error_type>;
  friend struct detail::swap_expected_t;
  
private:
  using bad_access_exception = bad_expected_access<std::decay_t<error_type>>;
  static constexpr bool error_trivially_copy_constructible = std::is_trivially_copy_constructible_v<error_type>;
  static constexpr bool error_nothrow_move_constructible = std::is_nothrow_move_constructible_v<error_type>;
  static constexpr bool error_trivially_move_constructible = std::is_trivially_move_constructible_v<error_type>;
  static constexpr bool error_copy_constructible = std::is_copy_constructible_v<error_type>;
  static constexpr bool error_move_constructible = std::is_move_constructible_v<error_type>;
  
  union
    {
    E error_;
    };
  bool has_value_;
  
public:
  constexpr expected() noexcept
        : has_value_{true}
    {}
    
  constexpr expected(expected const&) noexcept
      requires error_trivially_copy_constructible = default;
      
  constexpr expected(expected const& rh)
      noexcept(error_nothrow_move_constructible)
      requires error_copy_constructible && (!error_trivially_copy_constructible)
          : has_value_( rh.has_value_)
    {
    if(!has_value_) [[likely]]
      std::construct_at(std::addressof(error_), rh.error_);
    }
    
  constexpr expected(expected &&) noexcept
      requires error_trivially_move_constructible = default;
    
  constexpr expected(expected && rh)
      noexcept(error_nothrow_move_constructible)
      requires error_move_constructible && (!error_trivially_move_constructible)
          : has_value_( rh.has_value_)
    {
    if(!has_value_) [[unlikely]]
      std::construct_at(std::addressof(error_), std::move(rh.error_));
    }
  
  template< class... Args >
  constexpr explicit expected( std::in_place_t ) noexcept
      : has_value_{true}
    {}
    
  template<typename ... Args >
    requires std::constructible_from<error_type, Args...>
  constexpr explicit expected( unexpect_t, Args&&... args )
      noexcept(noexcept(std::forward<Args...>(args...)))
      : error_{ std::forward<Args>(args)...}, has_value_{}
    {}
  
  template<typename U, typename ... Args >
    requires std::constructible_from<error_type, std::initializer_list<U> &, Args...>
  constexpr explicit expected( unexpect_t, std::initializer_list<U> il, Args&&... args )
      : error_{ error_type{il, std::forward<Args>(args)...}}, has_value_{}
    {}

  template<std::convertible_to<error_type> G >
  constexpr explicit(!std::is_convertible_v<G const&, error_type>)
    expected( unexpected<G> const & e )
      : error_{ e.error() }, has_value_{}
    {}
  
  template<std::convertible_to<error_type> G >
  constexpr explicit(!std::is_convertible_v<G, E>)
    expected( unexpected<G> && e )
      : error_{ std::forward<G>(e.error()) }, has_value_{}
    {}
    
  constexpr ~expected() 
    requires std::is_trivially_destructible_v<error_type> = default;
    
  constexpr ~expected() 
    requires ( !std::is_trivially_destructible_v<error_type>)
      {
      if(!has_value_) [[unlikely]]
        std::destroy_at(std::addressof( error_ ));
      }
      
  constexpr void operator*() const noexcept
    { assert(has_value_); }

  [[nodiscard]]
  constexpr explicit operator bool() const noexcept
    { return has_value_; }
  
  [[nodiscard]]
  constexpr bool has_value() const noexcept
    { return has_value_; }
    
  constexpr void value() const &
      requires error_copy_constructible
    {
    if(!has_value_) [[unlikely]]
      throw bad_access_exception{ error_ };
    }
  
  constexpr void value() &&
    requires (error_copy_constructible || error_move_constructible)
    {
    if(!has_value_) [[unlikely]]
      throw bad_access_exception{ std::move(error_) };
    }
    
  [[nodiscard]]
  constexpr error_type const & error() const & noexcept
    {
    assert(!has_value_);
    return error_;
    }
    
  [[nodiscard]]
  constexpr error_type & error() & noexcept
    {
    assert(!has_value_);
    return error_;
    }
    
  [[nodiscard]]
  constexpr error_type const && error() const && noexcept
    {
    assert(!has_value_);
    return std::move(error_);
    }
  
  [[nodiscard]]
  constexpr error_type && error() && noexcept
    {
    assert(!has_value_);
    return std::move(error_);
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
    { return detail::and_then( std::move(*this),std::forward<F>(f)); }
    
  template<typename F>
  constexpr auto and_then( F && f ) const &&
      requires error_move_constructible
    { return detail::and_then(std::move(*this),std::forward<F>(f)); }
    
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
    return detail::transform(std::move(*this), std::forward<F>(f));
    }
    
  template<typename F>
  constexpr auto transform( F&& f ) const &&
      requires error_move_constructible
    {
    return detail::transform(std::move(*this), std::forward<F>(f));
    }
  
  template<typename F>
  constexpr auto or_else( F&& f ) &
    {
    return detail::or_else(*this,std::forward<F>(f));
    }
    
  template<typename F>
  constexpr auto or_else( F&& f ) const &
    {
    return detail::or_else(*this,std::forward<F>(f));
    }
  
  template<typename F>
  constexpr auto or_else( F&& f ) &&
    {
    return detail::or_else(std::move(*this),std::forward<F>(f));
    }
    
  template<typename F>
  constexpr auto or_else( F&& f ) const &&
    {
    return detail::or_else(std::move(*this),std::forward<F>(f));
    }
    
  template<typename F>
  constexpr auto transform_error( F&& f ) &
    { return detail::transform_error(*this, std::forward<F>(f)); }
    
  template<typename F>
  constexpr auto transform_error( F&& f ) const &
    { return detail::transform_error(*this, std::forward<F>(f)); }
    
  template<typename F>
  constexpr auto transform_error( F&& f ) &&
    { return detail::transform_error(std::move(*this), std::forward<F>(f)); }
    
  template<typename F>
  constexpr auto transform_error( F&& f ) const &&
    { return detail::transform_error(std::move(*this), std::forward<F>(f)); }
  
  constexpr void emplace() noexcept
    {
    if(!has_value_) [[likely]]
      {
      if constexpr (!std::is_trivially_destructible_v<error_type>)
        std::destroy_at(std::addressof(error_));
      has_value_ = true;
      }
    }
  
  constexpr void swap( expected & other )
      noexcept( detail::swap_no_throw<void,E> )
      requires concepts::swap_constraints<void,E>
    {
    detail::swap_dispatch(*this, other);
    }
      
  template<typename T2, typename E2>
  requires requires
    {
    requires std::is_void_v<T2>;
    requires std::equality_comparable_with<error_type,E2>;
    }
  friend constexpr bool operator==( expected const & lhs, expected<T2, E2> const & rhs )
    noexcept( noexcept(lhs.error() == rhs.error()))
    {
    if(lhs.has_value() == rhs.has_value())
      return lhs.has_value() || lhs.error() == rhs.error();
    else
      return false;
    }

  template<std::equality_comparable_with<error_type> E2 >
  friend constexpr bool operator==( expected const & x, unexpected<E2> const & e )
    {
    if(!x.has_value())
      return x.error() == e.error();
    return false;
    }

  friend constexpr void swap( expected & lhs, expected & rhs ) noexcept(noexcept(lhs.swap(rhs)))
    { lhs.swap(rhs); }
  };
  
namespace detail
  {
  template<typename F, typename T>
  struct and_then_invoke_result
    {
    using type = std::remove_cvref_t<std::invoke_result_t<F, decltype(std::declval<T>())>>;
    };
  template<typename F>
  struct and_then_invoke_result<F,void>
    {
    using type = std::remove_cvref_t<std::invoke_result_t<F>>;
    };
    
  template<typename F, typename T>
  using and_then_invoke_result_t = typename and_then_invoke_result<F,T>::type;
  
  template<typename EX, typename F>
  constexpr auto and_then( EX && ex, F && f )
    {
    using expected_type = std::remove_cvref_t<EX>;
    using U = and_then_invoke_result_t<F, decltype(std::forward<EX>(ex).value())>;
    if(ex.has_value())
      {
      if constexpr(std::is_void_v<typename expected_type::value_type>)
        return std::invoke(std::forward<F>(f));
      else
        return std::invoke(std::forward<F>(f), std::forward<EX>(ex).value());
      }
    else
      return U(unexpect, std::forward<EX>(ex).error());
    }
    
  template<typename F, typename T>
  struct transform_invoke_result
    {
    using type = std::remove_cv_t<std::invoke_result_t<F, decltype(std::declval<T>())>>;
    };
  template<typename F>
  struct transform_invoke_result<F,void>
    {
    using type = std::remove_cv_t<std::invoke_result_t<F>>;
    };
  template<typename F, typename T>
  using transform_invoke_result_t = typename transform_invoke_result<F,T>::type;
  
  template<typename EX, typename F >
  constexpr auto transform( EX && ex, F && f )
    {
    using expected_type = std::remove_cvref_t<EX>;
    using U = transform_invoke_result_t<F, decltype(std::forward<EX>(ex).value())>;

    using error_type = typename expected_type::error_type;
    if(ex.has_value())
      {
      if constexpr(std::is_void_v<typename expected_type::value_type>)
        {
        std::invoke(std::forward<F>(f));
        return expected<U,error_type>{std::in_place};
        }
      else
        return expected<U,error_type>{std::in_place, std::invoke(std::forward<F>(f), std::forward<EX>(ex).value())};
      }
    else
      return expected<U,error_type>(unexpect, std::forward<EX>(ex).error());
    }
    
  template<typename EX, typename F >
  constexpr auto or_else( EX && ex, F&& f )
    {
    using G = std::remove_cvref_t<std::invoke_result_t<F, decltype(std::forward<EX>(ex).error())>>;
    static_assert(std::is_same_v<typename G::value_type, typename EX::value_type>);
    if (ex.has_value())
      {
      if constexpr(std::is_void_v<typename EX::value_type>)
        return G();
      else
        return G(std::in_place, std::forward<EX>(ex).value());
      }
    else
      return std::invoke(std::forward<F>(f), std::forward<EX>(ex).error());
    }
    
  template<typename EX, typename F>
  constexpr auto transform_error( EX && ex, F && f )
    {
    using expected_type = std::remove_cvref_t<EX>;
    using G = std::remove_cv_t<std::invoke_result_t<F, decltype(std::forward<EX>(ex).error())>>;
    using value_type = typename expected_type::value_type;
    if(ex.has_value())
      {
      if constexpr(std::is_void_v<typename expected_type::value_type>)
        return expected<value_type, G>();
      else
        return expected<value_type, G>(std::in_place, std::forward<EX>(ex).value());
      }
    else
      return expected<value_type, G>(unexpect, std::invoke(std::forward<F>(f), std::forward<EX>(ex).error()));
    }

  template<bool use_noexcept, typename T>
  struct revert_if_except_t
    {
    struct empty_t {};
    T value;
    std::conditional_t<use_noexcept,empty_t, T *> release_address;
    
    constexpr explicit revert_if_except_t( T && v, T * release_addr ) 
      : value{ std::move(v) }
      {
      if constexpr(!use_noexcept)
        release_address = release_addr;
      }
    constexpr ~revert_if_except_t() 
      requires use_noexcept = default;
    
    constexpr T && release() noexcept 
      {
      if constexpr(!use_noexcept)
        release_address = nullptr;
      return std::move(value);
      }
      
    constexpr ~revert_if_except_t()
      {
      if constexpr(!use_noexcept)
        if(release_address != nullptr)
          std::construct_at(release_address, std::move(value));
      }
    };

  struct swap_expected_t
    {
    template<typename T, typename E>
    small_vector_static_call_operator
    constexpr void operator()( expected<T,E> & l, expected<T,E> & r )
        small_vector_static_call_operator_const
        noexcept( detail::swap_no_throw<T,E> )
        requires concepts::swap_constraints<T,E>
      {
      if(l.has_value() && r.has_value())
        {
        if constexpr (!std::is_void_v<T>)
          std::swap(l.value(), r.value());
        }
      else if(!l.has_value() && !r.has_value())
        {
        std::swap(l.error(), r.error());
        }
      else if( !l.has_value() && r.has_value())
        operator()(r,l);
      else
        {
        if constexpr( std::is_void_v<T>)
          {
          std::construct_at(std::addressof(l.error_), std::move(r.error()));
          std::destroy_at(std::addressof(r.error_));
          std::swap(l.has_value_, r.has_value_);
          }
        else if constexpr (std::is_nothrow_move_constructible_v<E>) 
          {
          using revert = revert_if_except_t<std::is_nothrow_move_constructible_v<T>,E>;
          revert temp{std::move(r.error()), std::addressof(r.error_)};
          std::destroy_at(std::addressof(r.error_));
          std::construct_at(std::addressof(r.value_), std::move(l.value()));
          std::destroy_at(std::addressof(l.value_));
          std::construct_at(std::addressof(l.error_), temp.release());
          std::swap(l.has_value_, r.has_value_);
          }
        else 
          {
          using revert = revert_if_except_t<false,T>;
          revert temp{std::move(l.value()),std::addressof(l.value_)};
          std::destroy_at(std::addressof(l.value_));
          std::construct_at(std::addressof(l.error_), std::move(r.error()));
          std::destroy_at(std::addressof(r.error_));
          std::construct_at(std::addressof(r.value_), temp.release());
          std::swap(l.has_value_, r.has_value_);
          }
        }
      }
    };

  template<concepts::is_expected EX>
  inline constexpr void swap_dispatch( EX & l, EX & r )
    {
    swap_expected_t{}(l,r);
    }
  }


}
#endif
