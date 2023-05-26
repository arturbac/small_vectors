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

#include "detail/uninitialized_constexpr.h"
#include "detail/vector_storage.h"
#include "detail/vector_func.h"
#include "detail/string_func.h"
#include "detail/iterator.h"

namespace coll
{
using coll::detail::string::static_string_tag;
using coll::detail::string::buffered_string_tag;

template<typename V, uint64_t N, typename storage_tag>
struct basic_string_t;

namespace detail
{
  template<typename char_type>
  struct buffer_traits{};
  
  template<>
  struct buffer_traits<char>{ static constexpr uint64_t capacity = 32u; };
  
  template<>
  struct buffer_traits<char8_t>{ static constexpr uint64_t capacity = 32u; };
  
  template<>
  struct buffer_traits<char16_t>{ static constexpr uint64_t capacity = 16u; };
  
  template<>
  struct buffer_traits<char32_t>{ static constexpr uint64_t capacity = 8u; };
  
  template<>
  struct buffer_traits<wchar_t>{ static constexpr uint64_t capacity = 8u; };
};

template<typename char_type>
using basic_string = basic_string_t<char_type, detail::buffer_traits<char_type>::capacity,buffered_string_tag>;

using string = basic_string<char>;
using u8string = basic_string<char8_t>;
using u16string = basic_string<char16_t>;
using u32string = basic_string<char32_t>;
using wstring = basic_string<wchar_t>;

template<typename char_type, uint64_t capacity>
using static_basic_string = basic_string_t<char_type,capacity,static_string_tag>;

template<uint64_t capacity>
using static_string = static_basic_string<char,capacity>;

template<uint64_t capacity>
using static_u8string = static_basic_string<char8_t,capacity>;

template<uint64_t capacity>
using static_u16string = static_basic_string<char16_t,capacity>;

template<uint64_t capacity>
using static_u32string = static_basic_string<char32_t,capacity>;

template<uint64_t capacity>
using static_wstring = static_basic_string<wchar_t,capacity>;


template<typename V, uint64_t N, typename T>
struct basic_string_t
  {
  using value_type = V;
  using char_type = value_type;
  using storage_tag = T;
  using iterator = detail::adapter_iterator<char_type *>;
  using const_iterator = detail::adapter_iterator<char_type const *>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using view_type = std::basic_string_view<char_type>;
  
  static constexpr bool supports_reallocation() noexcept { return std::is_same_v<storage_tag,buffered_string_tag>; }
  
  using storage_type = std::conditional_t<supports_reallocation(),
                                             detail::small_vector_storage<value_type,uint32_t,N>,
                                             detail::static_vector_storage<value_type,N>>;

  using dynamic_storage_type =  std::conditional_t<supports_reallocation(),
                                                      detail::small_vector_storage<value_type,uint32_t,N>, void>;
  using size_type = typename storage_type::size_type;
  static constexpr size_type buffered_capacity_ = N;
  static constexpr size_type npos { std::numeric_limits<size_type>::max()};
  
  storage_type storage_;
  
  /// \returns the number of characters that can be held in in class buffered storage without allocation
  [[nodiscard]]
  inline static constexpr auto
  buffered_capacity() noexcept -> size_type
    { return buffered_capacity_ - coll::detail::string::null_termination; }

  /// \returns the number of characters that can be held in currently allocated storage 
  [[nodiscard]]
  inline static constexpr auto
  capacity() noexcept -> size_type
     requires detail::string::static_storage_tag<storage_tag>
    { return buffered_capacity_ - coll::detail::string::null_termination; }
  
  /// \returns the number of characters that can be held in currently allocated storage 
  [[nodiscard]]
  inline constexpr auto
  capacity() const noexcept -> size_type
      requires detail::string::buffered_storage_tag<storage_tag>
    { return storage_.capacity() - coll::detail::string::null_termination; }
    
  [[nodiscard]]
  inline constexpr auto
  data() const noexcept ->  value_type const *
    { return storage_.data(); }
     
  [[nodiscard]]
  inline constexpr auto
  data() noexcept -> value_type *
    { return storage_.data(); }
    
  [[nodiscard]]
  inline constexpr auto
  c_str() const noexcept ->  value_type const *
     requires (detail::string::null_terminate_string)
    { return storage_.data(); }
  
  /// \returns The largest possible number of char-like objects that can be stored in a basic_string.
  [[nodiscard]]
  inline static constexpr auto
  max_size() noexcept -> size_type
       requires detail::string::static_storage_tag<storage_tag>
    { return buffered_capacity_ - coll::detail::string::null_termination; }
  
  /// \returns The largest possible number of char-like objects that can be stored in a basic_string.
  [[nodiscard]]
  inline static constexpr auto
  max_size() noexcept -> size_type
       requires detail::string::buffered_storage_tag<storage_tag>
    { return std::numeric_limits<size_type>::max() - coll::detail::string::null_termination; }

  /// \returns A count of the number of char-like objects currently in the string.
  [[nodiscard]]
  inline constexpr auto
  size() const noexcept -> size_type
    { return storage_.size_; }
  
  /// \returns A count of the number of char-like objects currently in the string.
  [[nodiscard]]
  inline constexpr auto
  length() const noexcept -> size_type
    { return storage_.size_; }
    
  inline constexpr basic_string_t() noexcept = default;
  
  template<std::same_as<size_type> input_size_type>
  constexpr explicit basic_string_t( input_size_type count )
        : storage_{detail::string::value_construct<storage_type>(count)}
    {}

  ///\brief Constructs the string with the contents of the range [first, last)
  template<std::input_iterator input_iterator>
    requires std::convertible_to<std::iter_value_t<input_iterator>,char_type>
  constexpr basic_string_t( input_iterator first, input_iterator last )
      : storage_{ detail::string::copy_construct<storage_type>(first,last)}
    {}
    
  constexpr basic_string_t( basic_string_t && rh ) noexcept
    {
    storage_.construct_move( std::move(rh.storage_) );
    }
    
  explicit constexpr basic_string_t( view_type const & rh )
    : storage_{ detail::string::copy_construct<storage_type>(rh.begin(), rh.end())}
    {}
    
  constexpr basic_string_t( basic_string_t const & rh )
      : storage_{ detail::string::copy_construct<storage_type>(rh.begin(), rh.end())}
    {}

  ///\brief Constructs the string with count copies of character ch
  constexpr basic_string_t( std::same_as<size_type> auto count, std::same_as<char_type> auto ch )
      : storage_{ detail::string::fill_construct<storage_type>(count, ch)}
    {}
    
  inline constexpr auto
  operator =( basic_string_t && rh ) noexcept -> basic_string_t &
     {
     return assign( std::move(rh) );
     }
     
  inline constexpr auto
  operator =( basic_string_t const & rh ) -> basic_string_t &
    {
    return assign( rh );
    }
    
  // implicit assignment from view without auto convert into view
  inline constexpr auto
  operator =( std::same_as<view_type> auto const & rh ) -> basic_string_t &
    {
    return assign(rh);
    }

    
  constexpr void
  assign( std::same_as<size_type> auto count, std::same_as<char_type> auto ch )
    {  detail::string::fill_assign( storage_, count, ch ); }
    
    
  constexpr auto
  assign( basic_string_t && rh ) noexcept -> basic_string_t &
     {
     storage_.assign_move( std::move(rh.storage_) );
     detail::string::cond_null_terminate(data(),size());
     return *this;
     }
   
  constexpr auto
  assign( view_type const & rh ) -> basic_string_t &
    {
    detail::string::assign_copy(storage_, rh.begin(), rh.end());
    return *this;
    }
    
  template<std::input_iterator input_iterator>
  constexpr auto
  assign( input_iterator first, input_iterator last ) -> basic_string_t &
      requires std::convertible_to<std::iter_value_t<input_iterator>,char_type>
    {
    detail::string::assign_copy(storage_, first, last );
    return *this;
    }

  constexpr
  ~basic_string_t()
    {
    detail::string::storage_cleanup<storage_tag>(storage_);
    }
    
  [[nodiscard]]
  inline constexpr auto 
  operator[]( concepts::unsigned_arithmetic_integral auto index ) const noexcept
      -> char_type const &
    {
    return data()[index];
    }
  
  [[nodiscard]]
  inline constexpr auto
  operator[]( concepts::unsigned_arithmetic_integral  auto index ) noexcept
      -> char_type &
    {
    return data()[index];
    }
  
  [[nodiscard]]
  inline constexpr auto
  at( concepts::unsigned_arithmetic_integral auto index ) const noexcept
      -> char_type const &
    {
    return data()[index];
    }
  
  [[nodiscard]]
  inline constexpr auto
  at( concepts::unsigned_arithmetic_integral auto index ) noexcept
      -> char_type &
    {
    return data()[index];
    }
    
  //compatibility with old code
  inline constexpr auto
  empty() const noexcept -> bool
    { return detail::empty(*this); }

  inline constexpr auto
  begin() const noexcept -> const_iterator
    { return const_iterator{detail::data(*this)}; }
    
  inline constexpr auto
  cbegin() const noexcept -> const_iterator
    { return begin(); }
    
  inline constexpr auto
  begin() noexcept -> iterator
    { return iterator{detail::data(*this)}; }
    
  inline constexpr auto
  end() const noexcept -> const_iterator
    { return const_iterator{detail::end(*this)}; }
  
  inline constexpr auto
  cend() const noexcept -> const_iterator
    { return end(); }
    
  inline constexpr auto
  end() noexcept -> iterator
    { return iterator{detail::end(*this)}; }

  constexpr operator view_type() const noexcept
    { return view_type{ data(), size() }; }
    
  constexpr auto view() const noexcept -> view_type
    { return view_type{ data(), size() }; }
    
  inline constexpr auto
  rbegin() noexcept -> reverse_iterator 
    { return reverse_iterator{end()}; }
  
  inline constexpr auto
  rbegin() const noexcept -> const_reverse_iterator 
    { return const_reverse_iterator{end()}; }
    
  inline constexpr auto
  crbegin() const noexcept -> const_reverse_iterator 
    { return rbegin(); }
    
  inline constexpr auto
  rend() noexcept -> reverse_iterator 
    { return reverse_iterator{begin()}; }
    
  inline constexpr auto
  rend() const noexcept -> const_reverse_iterator 
    { return const_reverse_iterator{begin()}; }
    
  inline constexpr auto
  crend() const noexcept -> const_reverse_iterator 
    { return rend(); }
    
  inline constexpr auto
  front() const noexcept -> char_type const &
    { return detail::front(*this); }
  
  inline constexpr auto
  front() noexcept -> char_type &
    { return detail::front(*this); }
    
  inline constexpr auto
  back() const noexcept -> char_type const &
    { return detail::back(*this); }
    
  inline constexpr auto
  back() noexcept -> char_type &
    { return detail::back(*this); }
  
  constexpr void
  clear() noexcept
    { detail::string::clear(storage_); }
    
  constexpr void
  reserve( size_type new_cap )
      requires detail::string::buffered_storage_tag<storage_tag>
    { detail::string::reserve(storage_, new_cap); }
    
  constexpr void
  resize( size_type new_size )
    { detail::string::resize(storage_, new_size); }
    
  constexpr void
  resize( size_type new_size, char_type ch )
    { detail::string::resize(storage_, new_size, ch); }
    
  ///\brief Resizes the string to contain at most count characters, using the user-provided operation op to modify the possibly indeterminate contents and set the length.
  template<typename Operation >
  constexpr void resize_and_overwrite( size_type new_size, Operation op )
    { detail::string::resize_and_overwrite(storage_,new_size, op); }

  constexpr void
    shrink_to_fit()
      requires detail::string::buffered_storage_tag<storage_tag>
    {
    detail::string::shrink_to_fit(storage_);
    }
  
  constexpr auto
  insert( size_type index, size_type count, char_type ch )
      -> basic_string_t & 
    {
    detail::string::insert_fill(storage_, index, count, ch );
    return *this;
    }
  
  constexpr auto
  insert( size_type index, std::convertible_to<view_type> auto const & s )
      -> basic_string_t &
    {
    view_type t{ static_cast<view_type>(s)};
    detail::string::insert_copy(storage_, index, t.begin(), t.end() );
    return *this;
    }
  
  inline constexpr auto
  insert( size_type index, std::convertible_to<view_type> auto const & s,
         size_type index_str, size_type count = npos )
      -> basic_string_t &
    {
    view_type t{ static_cast<view_type>(s).substr(index_str,count)};
    insert(index, t);
    return *this;
    }

  template<std::forward_iterator forward_it>
  constexpr auto
  insert( size_type index, forward_it first, forward_it last )
    -> basic_string_t &
    {
    detail::string::insert_copy(storage_, index, first, last );
    return *this;
    }
    
  inline constexpr auto
  insert( const_iterator pos, char_type ch )
      -> iterator
    {
    size_type upos { static_cast<size_type>(pos-begin()) };
    insert(size_type(upos), size_type(1), ch );
    return std::next(begin(),upos);
    }
  
  inline constexpr auto
  insert( const_iterator pos, size_type count, char_type ch )
      -> iterator
    {
    size_type upos { static_cast<size_type>(pos-begin()) };
    insert(size_type(upos), count, ch );
    return std::next(begin(),upos);
    }
  
  template<std::forward_iterator forward_it>
  inline constexpr auto
  insert( const_iterator pos, forward_it first, forward_it last )
    -> iterator
    {
    size_type upos { static_cast<size_type>(pos-begin()) };
    insert(upos,first,last);
    return std::next(begin(),upos);
    }
  
  constexpr auto
  append( size_type count, char_type ch )
   -> basic_string_t &
    {
    detail::string::append_fill(storage_, count, ch );
    return *this;
    }
  
  constexpr auto
  append( std::convertible_to<view_type> auto const & s )
      -> basic_string_t &
    {
    view_type t{ static_cast<view_type>(s)};
    detail::string::append_copy(storage_, t.begin(), t.end() );
    return *this;
    }
    
  inline constexpr auto
  append( std::convertible_to<view_type> auto const & s, size_type index_str, size_type count = npos )
      -> basic_string_t &
    {
    view_type t{ static_cast<view_type>(s).substr(index_str,count)};
    append(t);
    return *this;
    }
    
  template<std::forward_iterator forward_it >
  constexpr auto
  append( forward_it first, forward_it last )
    -> basic_string_t &
    {
    detail::string::append_copy(storage_, first, last );
    return *this;
    }
    
  constexpr void
  push_back( char_type ch )
    {
    detail::string::push_back(storage_, ch );
    }
    
  inline constexpr auto
  operator+=( char_type ch )
      -> basic_string_t &
    {
    push_back(ch );
    return *this;
    }
  
  inline constexpr auto
  operator+=( std::convertible_to<view_type> auto s )
      -> basic_string_t &
    {
    return append(s);
    }
    
  constexpr auto
  erase( size_type index, size_type count = npos )
    -> basic_string_t &
    {
    detail::string::erase(storage_, index, count);
    return *this;
    }
    
  constexpr auto
  erase( const_iterator pos )
    -> iterator
    {
    size_type const index { static_cast<size_type>(std::distance(cbegin(),pos)) };
    detail::string::erase(storage_, index, 1u);
    return std::next(begin(), index);
    }
    
  constexpr auto
  erase( const_iterator first, const_iterator last )
    -> iterator
    {
    size_type const index { static_cast<size_type>(std::distance(cbegin(),first)) };
    if(last != cend()) [[unlikely]]
      {
      size_type const count { static_cast<size_type>(std::distance(first, last)) };
      detail::string::erase(storage_, index, count);
      }
    else
      resize(index);

    return std::next(begin(), index);
    }
    
  constexpr void pop_back()
    {
    detail::string::pop_back(storage_);
    }
    
  inline constexpr auto
  substr( size_type pos, size_type count = npos ) const noexcept
    -> view_type
    {
    return view().substr(pos,count);
    }
  
  inline constexpr auto
  starts_with( std::convertible_to<view_type> auto const & s ) const noexcept
    -> bool
    {
    return view().starts_with(s);
    }
    
  inline constexpr auto
  starts_with( char_type ch ) const noexcept
    -> bool
    {
    return view().starts_with(ch);
    }
    
  inline constexpr auto
  ends_with( std::convertible_to<view_type> auto const & s ) const noexcept
    -> bool
    {
    return view().ends_with(s);
    }
    
  inline constexpr auto
  ends_with( char_type ch ) const noexcept
    -> bool
    {
    return view().ends_with(ch);
    }
    
  inline constexpr auto
  contains( std::convertible_to<view_type> auto const & s ) const noexcept
    -> bool
    {
    return view().find(static_cast<view_type>(s)) != view_type::npos;
    }
    
  inline constexpr auto
  contains( char_type ch ) const noexcept
    -> bool
    {
    return view().find(ch) != view_type::npos;
    }
  
  inline constexpr auto
  find( char_type ch, size_type pos = 0u ) const noexcept
      -> size_type
    {
    return static_cast<size_type>(view().find(ch,pos));
    }
  
  inline constexpr auto
  find( std::convertible_to<view_type> auto const & s, size_type pos = 0u ) const noexcept
      -> size_type
    {
    return static_cast<size_type>(view().find(static_cast<view_type>(s),pos));
    }
    
  inline constexpr auto
  find( std::convertible_to<view_type> auto const & s, size_type pos, size_type count ) const noexcept
      -> size_type
    {
    auto vs{ static_cast<view_type>(s).substr(0u,count)};
    return static_cast<size_type>(view().find(vs,pos));
    }
    
  inline constexpr auto
  rfind( char_type ch, size_type pos = npos ) const noexcept
      -> size_type
    {
    return static_cast<size_type>(view().rfind(ch,pos));
    }
    
  inline constexpr auto
  rfind( std::convertible_to<view_type> auto const & s, size_type pos = npos ) const noexcept
    -> size_type
    {
    return static_cast<size_type>(view().rfind(static_cast<view_type>(s),pos));
    }
    
  inline constexpr auto
  rfind( std::convertible_to<view_type> auto const & s, size_type pos, size_type count ) const noexcept
      -> size_type
    {
    auto vs{ static_cast<view_type>(s).substr(0u,count)};
    return static_cast<size_type>(view().rfind(vs,pos));
    }
    
  inline constexpr auto
  compare( std::convertible_to<view_type> auto const & s ) const noexcept
      -> int
    {
    return view().compare(static_cast<view_type>(s));
    }
    
  inline constexpr auto
  compare( size_type pos1, size_type count1,
           std::convertible_to<view_type> auto const & s ) const noexcept
      -> int
    {
    return view().compare(pos1,count1,static_cast<view_type>(s));
    }
  inline constexpr auto
  compare( size_type pos1, size_type count1,
           std::convertible_to<view_type> auto const & s,
           size_type pos2, size_type count2 = npos ) const noexcept
      -> int
    {
    return view().compare(pos1,count1,static_cast<view_type>(s),pos2,count2);
    }
    
  inline constexpr auto
  find_first_of(  std::convertible_to<view_type> auto const & v, size_type pos = 0u ) const noexcept
      -> size_type
    {
    return static_cast<size_type>(view().find_first_of(static_cast<view_type>(v), pos));
    }
    
  constexpr auto
  replace( size_type pos, size_type count, std::convertible_to<view_type> auto const & v )
      -> basic_string_t &
    {
    detail::string::replace_copy(storage_, pos, count, v.begin(), v.end() );
    return *this;
    }
  
  template<std::forward_iterator iterator>
    requires std::same_as<char_type, std::iter_value_t<iterator>>
  inline constexpr auto
  replace( const_iterator first, const_iterator last, iterator first2, iterator last2 )
      -> basic_string_t &
    {
    auto pos {static_cast<size_type>(std::distance(cbegin(),first))};
    auto count{static_cast<size_type>(std::distance(first,last))};
    detail::string::replace_copy(storage_, pos, count, first2, last2 );
    return *this;
    }
    
  inline constexpr auto
  replace( const_iterator first, const_iterator last, std::convertible_to<view_type> auto const & v )
      -> basic_string_t &
    {
    auto pos {static_cast<size_type>(std::distance(cbegin(),first))};
    auto count{static_cast<size_type>(std::distance(first,last))};
    return replace(pos,count,v);
    }
      
  inline constexpr auto
  replace( size_type pos, size_type count, std::convertible_to<view_type> auto const & s,
           size_type pos2, size_type count2 = npos )
      -> basic_string_t &
    {
    return replace(pos,count, static_cast<view_type>(s).substr(pos2,count2));
    }
      
  constexpr auto
  replace( size_type pos, size_type count, size_type count2, char_type ch )
      -> basic_string_t &
    {
    detail::string::replace_fill(storage_,pos,count,count2,ch);
    return *this;
    }
      
  inline constexpr auto
  replace( const_iterator first, const_iterator last, size_type count2, char_type ch )
      -> basic_string_t &
    {
    auto pos {static_cast<size_type>(std::distance(cbegin(),first))};
    auto count{static_cast<size_type>(std::distance(first,last))};
    return replace(pos,count,count2,ch);
    }
  
  ///\brief replaces all occurrences of \ref what with \ref with
  inline constexpr auto
  find_and_replace( std::convertible_to<view_type> auto const & what,
                    std::convertible_to<view_type> auto const & with)
      -> size_type
    {
    size_type const what_size = static_cast<size_type>(what.size());
    size_type const with_size = static_cast<size_type>(with.size());
    size_type replacments_count {};
    if( what_size != 0 )
      {
      size_type fpos = find( what, 0 );
      while( fpos != npos )
         {
         replace( fpos, what_size, with );
         ++replacments_count;
         fpos = find( what, static_cast<size_type>(fpos + with_size) );
         }
      }
    return replacments_count;
    }
  
  inline constexpr void
  swap( basic_string_t & other ) noexcept
    {
    detail::string::swap(storage_, other.storage_);
    }
  };
  
  using detail::size;
  using detail::empty;
  using detail::at;
  using detail::capacity;
  using detail::data;
  
  namespace concepts
    {
    ///\brief constraint requiring type to be a basic_string_type or const basic_string_type
    template<typename basic_string_type>
    concept same_as_basic_string =
      requires
        {
        typename basic_string_type::char_type;
        typename basic_string_type::storage_tag;
        basic_string_type::buffered_capacity_;
        requires std::same_as<std::remove_const_t<basic_string_type>,
            basic_string_t<typename basic_string_type::char_type,
                           basic_string_type::buffered_capacity_,
                           typename basic_string_type::storage_tag>
            >;
        };
    }
  template<concepts::same_as_basic_string basic_string_type>
  [[nodiscard]]
  inline constexpr auto
  begin( basic_string_type & str ) noexcept
    { return str.begin(); }
    
  template<typename V, uint64_t N, typename T>
  [[nodiscard]]
  inline constexpr auto
  cbegin( basic_string_t<V,N,T> const & str ) noexcept
    { return str.begin(); }

    
  template<concepts::same_as_basic_string basic_string_type>
  [[nodiscard]]
  inline constexpr auto
  end( basic_string_type & str ) noexcept
    { return str.end(); }
    
  template<typename V, uint64_t N, typename T>
  constexpr auto operator ==( basic_string_t<V,N,T> const & l, basic_string_t<V,N,T> const & r ) noexcept
      -> bool
    {
    return l.view() == r.view();
    }
    
  template<typename V, uint64_t N, typename T>
  constexpr auto operator ==( basic_string_t<V,N,T> const & l, std::basic_string_view<V> r ) noexcept
      -> bool
    {
    return l.view() == r;
    }
    
  template<typename V, uint64_t N, typename T>
  constexpr auto operator <=>( basic_string_t<V,N,T> const & l, basic_string_t<V,N,T> const & r ) noexcept
      -> std::strong_ordering
    {
    return l.view() <=> r.view();
    }
    
  template<typename V, uint64_t N, typename T>
  constexpr auto operator <=>( basic_string_t<V,N,T> const & l, std::basic_string_view<V> r ) noexcept
      -> std::strong_ordering
    {
    return l.view() <=> r;
    }
    
  template<typename V, uint64_t N, typename T>
  inline constexpr void
  swap( basic_string_t<V,N,T> & lhs, basic_string_t<V,N,T> & rhs) noexcept
    {
    lhs.swap(rhs);
    }
    
  template<typename V, uint64_t N, typename T>
  inline constexpr auto
  hash( coll::basic_string_t<V,N,T> const & str ) noexcept
    {
    using char_type = V;
    if (std::is_constant_evaluated())
      return std::accumulate(begin(str), end(str), std::size_t{5381},
                      [](std::size_t init, char_type c)
                      { return init * (c ? static_cast<std::size_t>(c) + 33 : 5381); });
    else
      return std::hash<std::basic_string_view<V>>()(str.view());
    }
}

namespace std
{
  template<typename V, uint64_t N, typename T>
  struct hash<coll::basic_string_t<V,N,T>>
    {
#if defined(__cpp_static_call_operator)
    static
#endif
    inline constexpr auto
    operator()(coll::basic_string_t<V,N,T> const & str )
#if !defined(__cpp_static_call_operator)
       const
#endif
    noexcept
      {
      return coll::hash(str);
      }
    };
}

