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
#include <span>

namespace coll
{
  namespace detail
  {
  template<typename value_type, typename size_type>
  inline constexpr void sv_deallocate(value_type * p, size_type count ) noexcept;
  }
  using detail::vector_outcome_e;

  using detail::vector_tune_e;
  using detail::union_min_number_of_elements;
  
  template<typename V, std::unsigned_integral S,
           uint64_t N= union_min_number_of_elements<V,S>()>
  struct small_vector
    {
    //concepts::vector_constraints
      
    using value_type = V;
    using reference = value_type &;
    using const_reference = value_type const &;
    using size_type = S;
    using iterator = value_type *;
    using const_iterator = value_type const *;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using storage_type = detail::small_vector_storage<V,S,N>;
    using dynamic_storage_type = typename storage_type::dynamic_storage_type;
    using enum detail::small_vector_storage_type;
    using enum detail::vector_outcome_e;
    
    static constexpr bool support_reallocation() noexcept { return true; }
    
    storage_type storage_;
    
    [[nodiscard]]
    inline constexpr size_type
    capacity() const noexcept
      { return storage_.capacity(); }
      
    [[nodiscard]]
    inline static constexpr size_type
    buffered_capacity() noexcept
      { return storage_type::buffered_capacity; }
      
    [[nodiscard]]
    inline constexpr value_type const *
    data() const noexcept
      { return storage_.data(); }
       
    [[nodiscard]]
    inline constexpr value_type *
    data() noexcept
      { return storage_.data(); }
    
    [[nodiscard]]
    inline constexpr size_type
    size() const noexcept
      { return storage_.size_; }
  
    [[nodiscard]]
    inline static constexpr size_type
    max_size() noexcept
      { return std::numeric_limits<size_type>::max(); }
      
    [[nodiscard]]
    inline constexpr detail::small_vector_storage_type
    active_storage() const noexcept
      { return storage_.active_storage(); }
    
    constexpr operator std::span<value_type const>() const noexcept 
      { return std::span{data(),size()};}
    
    constexpr operator std::span<value_type>() noexcept 
      { return std::span{data(),size()};}
      
    inline constexpr small_vector() noexcept = default;
    
    constexpr explicit small_vector( size_type count )
        noexcept(noexcept(detail::resize(*this,count)))
      { detail::resize(*this,count); }
      
    template< class InputIt >
    constexpr small_vector( InputIt first, InputIt last )
      {
      auto res{ detail::insert(*this, end(), first, last) };
      detail::handle_error(res);
      }
      
    constexpr small_vector( std::initializer_list<value_type> init )
      {
      auto res{ detail::insert(*this, end(),
                               init.begin(), init.end()) };
      detail::handle_error(res);
      }
      
    constexpr small_vector( small_vector && rh )
      noexcept(std::is_nothrow_move_constructible_v<value_type>)
       {
       storage_.construct_move( std::move(rh.storage_) );
       }
      
    constexpr small_vector & operator =( small_vector && rh )
        noexcept(std::is_nothrow_move_assignable_v<value_type>)
       {
       storage_.assign_move( std::move(rh.storage_) );
       return *this;
       }
      
    ///\warning copy constructor may throw always, for dynamic as there is no other way to signalize allocation error
    constexpr small_vector( small_vector const & rh )
      {
      storage_.construct_copy(rh.storage_);
      }
      
    ///\warning copy constructor from compatibile small_vector with different buffered_size may throw always,
    //          for dynamic as there is no other way to signalize allocation error
    template<uint64_t M>
      requires ( M != N )
    explicit constexpr small_vector( small_vector<value_type,size_type,M> const & rh )
      {
      storage_.construct_copy(rh.storage_);
      }
       
    ///\warning copy assignment may throw always, for dynamic as there is no other way to signalize allocation error
    constexpr small_vector & operator =( small_vector const & rh )
      {
      storage_.assign_copy(rh.storage_);
      return *this;
      }
      
    constexpr small_vector & assign( small_vector && rh )
        noexcept(std::is_nothrow_move_assignable_v<value_type>)
       {
       storage_.assign_move( std::move(rh.storage_) );
       return *this;
       }
       
    ///\warning copy assignment may throw always, for dynamic as there is no other way to signalize allocation error
    template<uint64_t M>
    constexpr small_vector & assign( small_vector<value_type,size_type, M> const & rh )
      {
      storage_.assign_copy(rh.storage_);
      return *this;
      }
      
    constexpr ~small_vector()
      {
      storage_.destroy();
      }
      
    template<concepts::unsigned_arithmetic_integral arg_size_type>
    [[nodiscard]]
    inline constexpr auto const & operator[]( arg_size_type index ) const noexcept
      {
      assert(index< size());
      return data()[index];
      }
    
    template<concepts::unsigned_arithmetic_integral arg_size_type>
    [[nodiscard]]
    inline constexpr auto & operator[]( arg_size_type index ) noexcept
      {
      assert(index< size());
      return data()[index];
      }
      
    template<concepts::unsigned_arithmetic_integral arg_size_type>
    [[nodiscard]]
    inline constexpr auto const & at( arg_size_type index ) const noexcept
      {
      assert(index< size());
      return data()[index];
      }
      
    template<concepts::unsigned_arithmetic_integral arg_size_type>
    [[nodiscard]]
    inline constexpr auto & at( arg_size_type index ) noexcept
      {
      assert(index< size());
      return data()[index];
      }
      
    //compatibility with old code
    inline constexpr bool empty() const noexcept
      { return detail::empty(*this); }
      
    inline constexpr auto begin() const noexcept 
      { return detail::begin(*this); }
      
    inline constexpr auto cbegin() const noexcept 
      { return detail::cbegin(*this); }
      
    inline constexpr auto begin() noexcept 
      { return detail::begin(*this); }
      
    inline constexpr auto end() const noexcept 
      { return detail::end(*this); }
    
    inline constexpr auto cend() const noexcept 
      { return detail::end(*this); }
      
    inline constexpr auto end() noexcept 
      { return detail::end(*this); }

    inline constexpr reverse_iterator 
    rbegin() noexcept
      { return reverse_iterator{end()}; }
    
    inline constexpr const_reverse_iterator 
    rbegin() const noexcept
      { return const_reverse_iterator{end()}; }
      
    inline constexpr const_reverse_iterator 
    crbegin() const noexcept
      { return const_reverse_iterator{end()}; }
      
    inline constexpr reverse_iterator 
    rend() noexcept
      { return reverse_iterator{begin()}; }
      
    inline constexpr const_reverse_iterator 
    rend() const noexcept
      { return const_reverse_iterator{begin()}; }
      
    inline constexpr const_reverse_iterator 
    crend() const noexcept
      { return const_reverse_iterator{begin()}; }
      
    ///\brief Appends a new element to the end of the container
    ///       if user type throw during move or construction it meets strong exception guarantee
    ///       thismethod throw bad_alloc when reallocation fails to allocate memory
    ///       standalone function not throwing may be used instead emplace_back( vector, args ... )
    template<typename ...Args>
    inline constexpr value_type &
    emplace_back( Args &&... args )
      noexcept(noexcept(detail::emplace_back(*this, std::forward<Args>(args)...)))
      {
      auto res = detail::emplace_back(*this, std::forward<Args>(args)...);
      detail::handle_error(res);
      return back();
      }
    
    ///\brief Appends a new element to the end of the container
    ///       if user type throw during move or construction it meets strong exception guarantee
    ///       thismethod throw bad_alloc when reallocation fails to allocate memory
    ///       standalone function not throwing may be used instead push_back( vector, args ... )
    template<typename T>
    inline constexpr void
    push_back(T && value )
      {
      auto res = detail::push_back(*this, std::forward<T>(value));
      detail::handle_error(res);
      }
  
    inline constexpr auto const &
    front() const noexcept
      { return detail::front(*this); }
    
    inline constexpr auto &
    front() noexcept
      { return detail::front(*this); }
      
    inline constexpr auto const &
    back() const noexcept
      { return detail::back(*this); }
      
    inline constexpr auto & 
    back() noexcept
      { return detail::back(*this); }
    
    inline constexpr void
    clear() noexcept
      { detail::clear(*this); }
    
    inline constexpr iterator
    erase(const_iterator pos ) 
        noexcept(noexcept(detail::erase(*this, pos)))
      { return detail::erase(*this, pos); }
      
    inline constexpr iterator
    erase(const_iterator first, const_iterator last ) 
        noexcept(noexcept(detail::erase(*this, first, last)))
      { return detail::erase(*this, first, last); }
      
    inline constexpr void pop_back() noexcept 
      { detail::pop_back(*this); }

    template<concepts::random_access_iterator source_iterator>
    inline constexpr auto
    insert(const_iterator itpos, source_iterator itbeg, source_iterator itend )
        noexcept(noexcept(detail::insert(*this, itpos, itbeg, itend)))
      { return detail::insert(*this, itpos, itbeg, itend); }

    ///\returns Iterator pointing to the emplaced element. 
    template<typename ...Args>
    inline constexpr iterator
    emplace(const_iterator itpos, Args &&... args)
      { 
      auto ix { std::distance(cbegin(), itpos) };
      auto res = detail::emplace(*this, itpos, std::forward<Args>(args)...);
      detail::handle_error(res);
      return std::next(begin(), ix);
      }
      
    inline constexpr void
    reserve( size_type new_cap )
      {
      auto res = detail::reserve(*this, new_cap);
      detail::handle_error(res);
      }
      
    inline constexpr auto
    resize( size_type new_size )
      {
      auto res = detail::resize(*this, new_size);
      detail::handle_error(res);
      }
      
    //private detail use
    inline constexpr void set_size_priv_(size_type pos_ix) noexcept
      { storage_.size_ = pos_ix; }
      
    inline constexpr dynamic_storage_type exchange_priv_( dynamic_storage_type new_storage, size_type size )noexcept
      {
      return storage_.exchange_priv_(new_storage, size);
      }
      
    inline constexpr dynamic_storage_type switch_static_priv_() noexcept
        requires ( N != 0)
      {
      return storage_.switch_static_priv_();
      }
    };
    
  template<typename V, std::unsigned_integral S, uint64_t N>
  small_vector( small_vector<V,S,N> && ) -> small_vector<V,S,N>;
  
  template<typename V, std::unsigned_integral S, uint64_t N>
  small_vector( small_vector<V,S,N> const & ) -> small_vector<V,S,N>;
  
  namespace concepts
    {
    ///\brief constraint requiring type to be a static_vector or const static_vector
    template<typename small_vector_type>
    concept same_as_small_vector =
      requires
        {
        typename small_vector_type::value_type;
        typename small_vector_type::size_type;
        { small_vector_type::buffered_capacity() } noexcept -> std::same_as<typename small_vector_type::size_type>;
        requires concepts::unsigned_arithmetic_integral<typename small_vector_type::size_type>;
        requires std::same_as<std::remove_const_t<small_vector_type>,
            small_vector<typename small_vector_type::value_type,
                         typename small_vector_type::size_type,
                         small_vector_type::buffered_capacity()>
            >;
        };
    }
  using detail::at_least;
  using detail::size;
  using detail::empty;
  using detail::at;
  using detail::capacity;
  using detail::max_size;
  using detail::free_space;
  using detail::data;
  
  //explicit definition solves problem with ambuguity with std::begin
  template<concepts::same_as_small_vector small_vector_type>
  [[nodiscard]]
  inline constexpr auto
  begin( small_vector_type & vec ) noexcept
    { return detail::begin(vec); }
    
  template<typename V, typename S, uint64_t N>
  [[nodiscard]]
  inline constexpr auto
  cbegin( small_vector<V,S,N> const & vec ) noexcept
    { return detail::cbegin(vec); }

    
  template<concepts::same_as_small_vector small_vector_type>
  [[nodiscard]]
  inline constexpr auto
  end( small_vector_type & vec ) noexcept
    { return detail::end(vec); }
    
  using detail::back;
  using detail::erase_at_end;
  using detail::clear;
  using detail::erase;
  using detail::pop_back;
  using detail::insert;
  using detail::emplace;
  using detail::emplace_back;
  using detail::reserve;
  using detail::resize;
  using detail::shrink_to_fit;
}
