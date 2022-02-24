#pragma once

#include <utils/utility_cxx20.h>
#include "coll_concepts.h"
#include <memory>

namespace coll::detail
{
  //By default, only trivially copyable or trivially movable types are relocatable
  //however, is_relocatable may be specialized to mark complex types as relocatable
  //TODO
  template<typename V>
  inline constexpr bool is_relocatable_v = std::is_trivially_copyable_v<V>;
  
  template< typename iterator_type>
  using iterator_value_type_t =typename std::iterator_traits<iterator_type>::value_type;
  
  ///\return distance for growing valid range of iterator pair
  template<concepts::unsigned_arithmetic_integral size_type, typename iterator>
  inline constexpr auto
  udistance( iterator first, iterator last ) noexcept
    {
    return static_cast<size_type>(std::distance(first,last));
    }
    
  template<concepts::unsigned_arithmetic_integral size_type, typename iterator>
    requires ( not concepts::unsigned_arithmetic_integral<iterator> )
  inline constexpr auto unext( iterator ptr, size_type offset ) noexcept
    {
    return std::next( ptr, static_cast<ptrdiff_t>(offset) );
    }
  ///\brief no implicit convertion sum
  template<concepts::unsigned_arithmetic_integral size_type, concepts::unsigned_arithmetic_integral... Args>
  inline constexpr size_type nic_sum(size_type first, Args... args) noexcept
    {
    if constexpr( sizeof ...(Args) != 0u )
      return static_cast<size_type>(first + nic_sum(args...));
    return static_cast<size_type>(first);
    }
  ///\brief no implicit convertion sub
  template<concepts::unsigned_arithmetic_integral size_type, concepts::unsigned_arithmetic_integral osize_type>
  inline constexpr size_type nic_sub(size_type first, osize_type second) noexcept
    {
    return static_cast<size_type>(first-second);
    }
    
    
  template <concepts::iterator InputIterator, typename size_type>
    requires requires
      {
      requires std::is_nothrow_destructible_v<typename std::iterator_traits<InputIterator>::value_type>;
      requires concepts::unsigned_arithmetic_integral<size_type>;
      requires not std::is_trivially_destructible_v<typename std::iterator_traits<InputIterator>::value_type>;
      }
  inline constexpr void
  destroy_range( InputIterator data, size_type first, size_type last ) noexcept
    {
    std::destroy(unext(data,first), unext(data,last));
    }
    
  ///\brief used for unwinding already moved/copied range after exeption is throw in used move/copy operator
  template<bool use_nothrow, typename InputIt>
  struct range_unwinder
    {
    using value_type = typename std::iterator_traits<InputIt>::value_type;

    InputIt first_res_, last_;

    constexpr range_unwinder( InputIt first_res) noexcept : first_res_{first_res}, last_{first_res}{}

    constexpr void release()
      {
      if constexpr( use_nothrow || !std::is_trivially_destructible_v<value_type>)
        first_res_=last_;
      }

    constexpr ~range_unwinder()
    {
      if constexpr( use_nothrow || !std::is_trivially_destructible_v<value_type>)
        if(first_res_!=last_)
          std::destroy(first_res_, last_);
      }
    };

  template<typename value_type>
  constexpr void uninitialized_default_construct( value_type * store )
    noexcept(std::is_nothrow_default_constructible_v<value_type>)
    {
    if( std::is_constant_evaluated())
       std::construct_at(store);
    else
      ::new (store) value_type;
    }
    
  template<typename value_type, typename size_type>
  inline constexpr void
  uninitialized_value_construct_n(value_type * first, size_type count)
    {
    if( std::is_constant_evaluated())
      {
      for(size_type ix{}; ix != count; ++ix)
        std::construct_at(std::addressof(first[ix]));
      }
    else
      std::uninitialized_value_construct_n(first, count);
    }
    
  template<typename InputIterator>
  inline constexpr decltype(auto) deref_iter( InputIterator iter ) noexcept
    { return *iter; }
  
  template<typename InputIterator>
  inline constexpr auto && deref_iter( std::move_iterator<InputIterator> iter ) noexcept
    { return std::move(*iter); }
  
  template<concepts::input_iterator InputIterator, std::integral Size, concepts::forward_iterator ForwardIterator>
  inline constexpr void
  uninitialized_copy_n(InputIterator first, Size count, ForwardIterator result)
    noexcept(std::is_nothrow_constructible_v<iterator_value_type_t<InputIterator>>)
    {
    constexpr bool use_nothrow  = std::is_nothrow_move_constructible_v<iterator_value_type_t<InputIterator>>;
    using unwind = range_unwinder<use_nothrow,ForwardIterator>;
    unwind cur{ result };
    auto src{ first };
    for (; count > 0; --count, (void) ++src, ++cur.last_)
      std::construct_at(std::addressof(*cur.last_), *src );
    cur.release();
    }
    
  template <concepts::input_iterator InputIterator, std::integral Size, concepts::forward_iterator ForwardIterator>
  inline constexpr void
  uninitialized_move_n(InputIterator first, Size count, ForwardIterator result)
      noexcept(std::is_nothrow_move_constructible_v<iterator_value_type_t<InputIterator>>)
    {
    constexpr bool use_nothrow  = std::is_nothrow_move_constructible_v<iterator_value_type_t<InputIterator>>;
    using unwind = range_unwinder<use_nothrow,ForwardIterator>;
    unwind cur{ result };
    auto src{ std::make_move_iterator(first) };
    for (; count > 0; --count, (void) ++src, ++cur.last_)
      std::construct_at(std::addressof(*cur.last_), std::move(*src));
    cur.release();
    }

  template<typename iterator>
  inline constexpr bool move_if_noexcept_cond = std::is_nothrow_move_constructible_v<iterator>
                                             || !std::is_copy_constructible_v<iterator>;

  template<typename iterator>
  inline constexpr auto
  make_move_if_noexcept_iterator(iterator it )
    {
    using value_type = typename std::iterator_traits<iterator>::value_type;
    using return_type = std::conditional_t<move_if_noexcept_cond<value_type>,
                                           std::move_iterator<iterator>,
                                           iterator>;
    return return_type(it);
    }
    
  template<typename value_type>
  inline constexpr auto
  make_move_if_noexcept_iterator(value_type * it )
    {
    using return_type = std::conditional_t<move_if_noexcept_cond<value_type>,
                                           std::move_iterator<value_type *>,
                                           value_type const *>;
    return return_type(it);
    }
    
  //for strong exception guarantee
  //https://en.cppreference.com/w/cpp/utility/move_if_noexcept
  template<concepts::input_iterator InputIterator, std::integral size_type, concepts::forward_iterator ForwardIterator>
  inline constexpr void
  uninitialized_move_if_noexcept_n(InputIterator first, size_type count, ForwardIterator result)
      noexcept(std::is_nothrow_move_constructible_v<iterator_value_type_t<InputIterator>>)
    {
    constexpr bool use_nothrow = std::is_nothrow_move_constructible_v<iterator_value_type_t<InputIterator>>;
    if constexpr(use_nothrow)
      uninitialized_move_n(first,count, result);
    else
      uninitialized_copy_n(first, count, result);
    }

  template <concepts::iterator InputIterator, std::integral size_type, concepts::forward_iterator ForwardIterator>
    requires (true == std::is_nothrow_move_constructible_v<iterator_value_type_t<InputIterator>>)
  inline constexpr void
  uninitialized_relocate_n(InputIterator first, size_type count, ForwardIterator result) noexcept
    {
    using value_type = iterator_value_type_t<InputIterator>;
    //TODO update to relocate with c++23 or later when it became available
    InputIterator cur{ result };
    auto src{ first };
    for (; count > 0; --count, (void) ++src, ++cur)
      {
      //construct and destroy in the same loop for better cache utilization
      std::construct_at(std::addressof(*cur), std::move(*src));
      if constexpr(!std::is_trivially_destructible_v<value_type>)
        std::destroy_at(std::addressof(*src));
      }
    }
    
  template <concepts::iterator InputIterator, std::integral size_type, concepts::forward_iterator ForwardIterator>
  inline constexpr void
  uninitialized_relocate_if_noexcept_n(InputIterator first, size_type count, ForwardIterator result)
        noexcept(std::is_nothrow_move_constructible_v<iterator_value_type_t<InputIterator>>)
    {
    using value_type = iterator_value_type_t<InputIterator>;

    constexpr bool use_nothrow = std::is_nothrow_move_constructible_v<iterator_value_type_t<InputIterator>>;
    if constexpr(use_nothrow)
      uninitialized_relocate_n(first,count, result);
    else
      {
      uninitialized_copy_n(first, count, result);
      if constexpr (!std::is_trivially_destructible_v<value_type> )
        destroy_range( first, size_type(0u),  count );
      }
    }
    
  template <concepts::iterator InputIterator, std::integral size_type, concepts::forward_iterator ForwardIterator>
  inline constexpr void
  uninitialized_relocate_with_copy_n(InputIterator first, size_type count, ForwardIterator result)
        noexcept(std::is_nothrow_copy_constructible_v<iterator_value_type_t<InputIterator>>)
    {
    using value_type = iterator_value_type_t<InputIterator>;
    uninitialized_copy_n(first,count,result);
    if constexpr (!std::is_trivially_destructible_v<value_type> )
      destroy_range( first, size_type(0u),  count );

    }
}
