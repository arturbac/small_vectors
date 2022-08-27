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

namespace coll
{
  using detail::vector_outcome_e;

  using detail::vector_tune_e;
    
  ///\brief vector like container with constant storage space inside struct
  ///       for trivialy constructible elements it can be used in consteval expresions with clang, with gcc is possibility to use with any objects types
  template<concepts::vector_constraints V, uint64_t N>
  struct static_vector
    {
    using value_type = V;
    using reference = value_type &;
    using const_reference = value_type const &;
    using size_type = typename detail::size_type_select_t<N>;
    using const_iterator = value_type const *;
    using iterator = value_type *;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using enum detail::vector_outcome_e;
    
    static constexpr size_type capacity_ = N;
    static constexpr bool support_reallocation() noexcept { return false; }
    
    [[no_unique_address]]
    detail::static_vector_storage<value_type,capacity_> storage_;
    
    [[no_unique_address]]
    size_type size_ = 0;
    
    [[nodiscard]]
    inline static constexpr auto
    capacity() noexcept
      { return capacity_; }
    
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
      { return size_; }
  
    [[nodiscard]]
    inline static constexpr size_type
    max_size() noexcept
      { return capacity_; }
      
    constexpr static_vector() noexcept = default;
    
    constexpr static_vector( static_vector && rh )
      noexcept(std::is_nothrow_move_constructible_v<value_type>)
        requires std::move_constructible<value_type>
      {
      if constexpr(std::is_nothrow_move_constructible_v<value_type>)
        detail::uninitialized_relocate_n( detail::begin(rh), rh.size(), detail::begin(*this) );
      else
        detail::uninitialized_relocate_with_copy_n( detail::begin(rh), rh.size(), detail::begin(*this) );
      size_ = std::exchange(rh.size_, 0);
      }
      
    constexpr static_vector( static_vector const & rh )
      noexcept(std::is_nothrow_copy_constructible_v<value_type>)
        requires std::copy_constructible<value_type>
      {
      detail::uninitialized_copy_n( detail::begin(rh), rh.size(), detail::begin(*this) );
      size_ = rh.size_;
      }
      
    constexpr static_vector & operator =( static_vector && rh )
        noexcept(std::is_nothrow_move_assignable_v<value_type>)
      requires std::movable<value_type>
      {
      if constexpr(std::is_nothrow_move_constructible_v<value_type>)
        detail::uninitialized_relocate_n( detail::begin(rh), rh.size(), detail::begin(*this) );
      else
        detail::uninitialized_relocate_with_copy_n( detail::begin(rh), rh.size(), detail::begin(*this) );
      size_ = std::exchange(rh.size_,0);
      return *this;
      }
      
    constexpr static_vector & operator =( static_vector const & rh )
        noexcept(std::is_nothrow_copy_assignable_v<value_type>)
      requires std::copyable<value_type>
      {
      detail::uninitialized_copy_n( detail::begin(rh), rh.size(), detail::begin(*this) );
      size_ = rh.size_;
      return *this;
      }
    
    
    constexpr ~static_vector()
      {
      if constexpr (!std::is_trivially_destructible_v<value_type> )
        detail::destroy_range(data(), size_type{0}, size_);
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
      
    template<concepts::unsigned_arithmetic_integral arg_size_type>
    [[nodiscard]]
    inline constexpr auto const & operator[]( arg_size_type index ) const noexcept
      {
      assert(index< size_);
      return data()[index];
      }
    
    template<concepts::unsigned_arithmetic_integral arg_size_type>
    [[nodiscard]]
    inline constexpr auto & operator[]( arg_size_type index ) noexcept
      {
      assert(index< size_);
      return data()[index];
      }
      
    template<vector_tune_e tune = vector_tune_e::checked, typename ...Args>
    inline constexpr auto
    emplace_back( Args &&... args )
      noexcept( tune == vector_tune_e::unchecked || noexcept(detail::emplace_back(*this, std::forward<Args>(args)...)))
       {
       if constexpr(tune == vector_tune_e::checked)
         return detail::emplace_back(*this, std::forward<Args>(args)...);
       else
         return detail::emplace_back_unchecked(*this,std::forward<Args>(args)...);
       }
      
    template<vector_tune_e tune = vector_tune_e::checked, typename T>
    inline constexpr void
    push_back(T && value )
        noexcept(noexcept(emplace_back<tune>(*this, std::forward<T>(value))))
       { return emplace_back<tune>(*this, std::forward<T>(value)); }
      
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
      
    inline constexpr auto
    erase(const_iterator pos ) 
        noexcept(noexcept(detail::erase(*this, pos)))
      { return detail::erase(*this, pos); }
      
    inline constexpr auto
    erase(const_iterator first, const_iterator last ) 
        noexcept(noexcept(detail::erase(*this, first, last)))
      { return detail::erase(*this, first, last); }
      
    inline constexpr void pop_back() noexcept 
      { detail::pop_back(*this); }
      
    template<concepts::random_access_iterator source_iterator>
    inline constexpr auto
    insert(iterator itpos, source_iterator itbeg, source_iterator itend )
        noexcept(noexcept(detail::insert(*this, itpos, itbeg, itend)))
      { return detail::insert(*this, itpos, itbeg, itend); }
      
    template<vector_tune_e tune = vector_tune_e::checked, typename ...Args>
    inline constexpr auto
    emplace(iterator itpos, Args &&... args)
        noexcept(noexcept(detail::emplace(*this, itpos, std::forward<Args>(args)...)));
//       { return coll::emplace<tune>(*this, itpos, std::forward<Args>(args)...); }
      
    inline constexpr auto
    resize( size_type new_size )
        noexcept(noexcept(detail::resize(*this, new_size)))
      { return detail::resize(*this, new_size); }
      
    inline constexpr void set_size_priv_(size_type pos_ix) noexcept
      { size_ = pos_ix; }
    };
    
  namespace concepts
    {
    ///\brief constraint requiring type to be a static_vector or const static_vector
    template<typename static_vector_type>
    concept same_as_static_vector =
      requires
        {
        typename static_vector_type::value_type;
        static_vector_type::capacity_;
        requires concepts::unsigned_arithmetic_integral<decltype(static_vector_type::capacity_)>;
        requires std::same_as<std::remove_const_t<static_vector_type>,
            static_vector<typename static_vector_type::value_type,static_vector_type::capacity_>
            >;
        };
    }
    
  using detail::size;
  using detail::empty;
  using detail::at;
  using detail::capacity;
  using detail::max_size;
  using detail::free_space;
  using detail::data;

  template<concepts::same_as_static_vector static_vector_type>
  [[nodiscard]]
  inline constexpr auto
  begin( static_vector_type & vec ) noexcept
    { return detail::begin(vec); }
    
  template<typename V, uint64_t N>
  [[nodiscard]]
  inline constexpr auto
  cbegin( static_vector<V,N> const & vec ) noexcept
    { return detail::cbegin(vec); }

    
  template<concepts::same_as_static_vector static_vector_type>
  [[nodiscard]]
  inline constexpr auto
  end( static_vector_type & vec ) noexcept
    { return detail::end(vec); }
    
  using detail::back;
  using detail::erase_at_end;
  using detail::clear;
  using detail::erase;
  using detail::pop_back;
  using detail::insert;
  using detail::resize;
  
  ///\brief Appends a new element to the end of the container
  ///\warning this is unchecked version, insertion into full container is undefined behavior
  template<vector_tune_e tune = vector_tune_e::checked, typename V, uint64_t N, typename ...Args>
    requires (tune == vector_tune_e::unchecked)
  inline constexpr void
  emplace_back(static_vector<V,N> & vec, Args &&... args)
      noexcept(noexcept(detail::emplace_back_unchecked(vec,std::forward<Args>(args)...)))
    {
    detail::emplace_back_unchecked(vec,std::forward<Args>(args)...);
    }

  ///\brief Appends a new element to the end of the container
  ///\returns if there is no space left it returns \ref static_vector_outcome_e::out_of_storage
  ///         else it returns \ref static_vector_outcome_e::no_error
  template<vector_tune_e tune = vector_tune_e::checked, typename V, uint64_t N, typename ...Args>
    requires (tune == vector_tune_e::checked)
  inline constexpr vector_outcome_e
  emplace_back(static_vector<V,N> & vec, Args &&... args)
      noexcept(noexcept(detail::emplace_back(vec,std::forward<Args>(args)...)))
    {
    return detail::emplace_back(vec,std::forward<Args>(args)...);
    }

  template<vector_tune_e tune = vector_tune_e::checked, typename V, uint64_t N, typename T>
  inline constexpr void
  push_back( static_vector<V,N> & vec, T && value )
      noexcept(noexcept(emplace_back<tune>(vec,std::forward<T>(value))))
    {
    return emplace_back<tune>(vec,std::forward<T>(value));
    }

  template<vector_tune_e tune = vector_tune_e::checked, typename V, uint64_t N, typename ...Args>
    requires (tune == vector_tune_e::unchecked)
  inline constexpr void
  emplace(static_vector<V,N> & vec, typename static_vector<V,N>::iterator itpos, Args &&... args)
      noexcept(noexcept(detail::emplace_unchecked(vec, itpos, std::forward<Args>(args)...)))
    {
    detail::emplace_unchecked(vec, itpos, std::forward<Args>(args)...);
    }

  template<vector_tune_e tune = vector_tune_e::checked, typename V, uint64_t N, typename ...Args>
    requires (tune == vector_tune_e::checked)
  inline constexpr vector_outcome_e
  emplace(static_vector<V,N> & vec, typename static_vector<V,N>::iterator itpos, Args &&... args)
      noexcept(noexcept(detail::emplace( vec, itpos, std::forward<Args>(args)...)))
    {
    return detail::emplace( vec, itpos, std::forward<Args>(args)...);
    }
    
  ///\brief split \ref vec1 in half moving half of elements to the end of \ref vec2
  ///\warning this is unchecked version, insertion into full \ref vec2 container is undefined behavior
  template<vector_tune_e tune = vector_tune_e::checked, typename V, uint64_t N1, uint64_t N2>
    requires(N2 >= N1 && concepts::even_size<N1> && tune == vector_tune_e::unchecked )
  inline constexpr void
  split_by_half( static_vector<V,N1> & vec1, static_vector<V,N2> & vec2 )
      noexcept(std::is_nothrow_move_constructible_v<V>)
    {
    using size_type = typename static_vector<V,N1>::size_type;
    size_type half = static_cast<size_type>(size(vec1)>>1);

    auto range_mid { begin(vec1) + half };
    size_type move_count = static_cast<size_type>(size(vec1) - half);
    if constexpr( std::is_nothrow_move_constructible_v<V> )
      detail::uninitialized_relocate_n( range_mid, move_count, end(vec2) );
    else
      detail::uninitialized_relocate_with_copy_n( range_mid, move_count, end(vec2) );
    vec1.size_ -= move_count;
    vec2.size_ += move_count;
    }

  ///\brief split \ref vec1 in half moving half of elements to the end of \ref vec2
  ///\returns if there is no space left it in \ref vec2 returns \ref static_vector_outcome_e::out_of_storage
  ///         else it returns \ref static_vector_outcome_e::no_error
  template<vector_tune_e tune = vector_tune_e::checked, typename V, uint64_t N1, uint64_t N2>
    requires(N2 >= N1 && concepts::even_size<N1> && tune == vector_tune_e::checked)
  inline constexpr vector_outcome_e
  split_by_half( static_vector<V,N1> & vec1, static_vector<V,N2> & vec2 ) 
      noexcept(std::is_nothrow_move_constructible_v<V>)
    {
    using size_type = typename static_vector<V,N1>::size_type;
    auto half = static_cast<size_type>(size(vec1)>>1);
    if( free_space(vec2) >= half )
      {
      split_by_half<vector_tune_e::unchecked>(vec1,vec2);
      return vector_outcome_e::no_error;
      }
    else
      return vector_outcome_e::out_of_storage;
    }
}
