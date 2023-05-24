#pragma once

#include "vector_func.h"
#include <algorithm>
#include <string_view>

namespace coll::detail::string
{
  struct static_string_tag { };
  struct buffered_string_tag {};
  
  template<typename tag>
  concept static_storage_tag = std::same_as<tag,static_string_tag>;

  template<typename tag>
  concept buffered_storage_tag = std::same_as<tag,buffered_string_tag>;

  template<typename storage_tag, typename storage_type>
    requires static_storage_tag<storage_tag>
  inline constexpr void storage_cleanup( storage_type &) noexcept
    {}
    
  template<typename storage_tag, typename storage_type>
    requires detail::string::buffered_storage_tag<storage_tag>
  inline constexpr void storage_cleanup(storage_type & storage) noexcept
    { storage.destroy(); }
    
  inline constexpr bool null_terminate_string = true;
  inline constexpr uint8_t null_termination = null_terminate_string ? 1u : 0u;
  
  template<typename char_type, typename size_type>
  constexpr size_type growth(size_type new_elements ) noexcept
    {
    auto byte_size = static_cast<size_type>(sizeof(char_type)*new_elements);
    return static_cast<size_type>((((byte_size + 8u )>>3)<<3)/sizeof(char_type));
    };
    
  template<typename char_type>
  inline constexpr void
  cond_null_terminate( char_type * data )
    {
    if constexpr (null_terminate_string)
      uninitialized_value_construct(data);
    }
    
  template<typename char_type, typename size_type>
  inline constexpr void
  cond_null_terminate( char_type * data, size_type new_size )
    {
    if constexpr (null_terminate_string)
      uninitialized_value_construct(data+new_size);
    }
  template<typename vector_storage>
  inline constexpr
  void clear(vector_storage & storage) noexcept
    {
    storage.size_ = 0u;
    cond_null_terminate(storage.data());
    }
  
  template<typename vector_storage>
  inline constexpr void
  shrink_to_fit(vector_storage & storage)
      requires (vector_storage::supports_reallocation)
    {
    using char_type = typename vector_storage::value_type;
    using size_type = typename vector_storage::size_type;
    if(storage.capacity() > storage.buffered_capacity )
      {
      size_type const raw_size = storage.size_ + null_termination;
      if(storage.buffered_capacity < raw_size)
        {
        auto optimal_capacity{ growth<char_type>(raw_size)};
        if(optimal_capacity != storage.capacity() )
          {
          auto old_strage_data{storage.data()};
          storage_context_t new_space{ sv_allocate<char_type>(optimal_capacity) };
          uninitialized_copy(old_strage_data, old_strage_data + storage.size_, new_space.data );
          storage_context_t old_storage{ storage.exchange_priv_(new_space, storage.size_ ) };
          assert(old_storage.data != nullptr );
          sv_deallocate(old_storage);
          }
        }
      else
        {
        size_type size { storage.size_ };
        storage_context_t old_storage{ storage.switch_static_priv_() };
        assert(old_storage.data != nullptr );
        auto data{storage.data()};
        uninitialized_copy(old_storage.data, old_storage.data + size, data );
        cond_null_terminate(data+size);
        storage.size_ = size;
        sv_deallocate(old_storage);
        }
      }
    }
    
  template<typename vector_storage, typename lambda >
  inline constexpr auto
  uninitialized_construct( typename vector_storage::size_type sz, lambda const & uninitialized_fn ) 
    -> vector_storage
    {
    vector_storage storage;
      
    using char_type = typename vector_storage::value_type;
    using size_type = typename vector_storage::size_type;
    size_type const raw_size = sz + null_termination;

    if(storage.buffered_capacity < raw_size) [[unlikely]] //optimise for small buffered strings
      {
      if constexpr( vector_storage::supports_reallocation )
        {
        auto new_capacity{ growth<char_type>(raw_size)};
        storage_context_t new_space{ sv_allocate<char_type>(new_capacity) };
        auto last = uninitialized_fn(new_space.data);
        cond_null_terminate(last);
        storage.data_ = new_space;
        storage.active_ = small_vector_storage_type::dynamic;
        }
      else
        throw std::length_error{"Out of buffer space"};
      }
    else // if( sz != 0u )
      {
      auto dest_data{storage.data()};
      //in constant evaluated buffer must be initialized even when unused
      if( std::is_constant_evaluated())
        uninitialized_default_construct(dest_data, dest_data + storage.buffered_capacity);

      auto last = uninitialized_fn(dest_data );
      cond_null_terminate(last);
    
      if constexpr( vector_storage::supports_reallocation )
        storage.active_ = small_vector_storage_type::buffered;
      }
    storage.size_ = sz;
    return storage;
    }
    
  template<typename vector_storage, typename lambda>
  inline constexpr void
  uninitialized_assign( vector_storage & storage, typename vector_storage::size_type sz, lambda const & uninitialized_fn  )
    {
    using char_type = typename vector_storage::value_type;
    using size_type = typename vector_storage::size_type;
    size_type const raw_size = sz + null_termination;
    
    if constexpr( vector_storage::supports_reallocation)
      if( raw_size <= vector_storage::buffered_capacity && storage.active_ == small_vector_storage_type::dynamic ) [[unlikely]]
        {
        storage_context_t old_storage{ storage.switch_static_priv_() };
        if(old_storage.data != nullptr )
          sv_deallocate(old_storage);
        }

    if(storage.capacity() < raw_size) [[unlikely]] //optimise for small buffered strings
      {
      if constexpr( vector_storage::supports_reallocation )
        {
        auto new_capacity{ growth<char_type>(raw_size)};
        storage_context_t new_space{ sv_allocate<char_type>(new_capacity) };
        auto last = uninitialized_fn(new_space.data );
        cond_null_terminate(last);
        //deallocate old space
        storage_context_t old_storage{ storage.exchange_priv_(new_space, sz ) };
        if(old_storage.data != nullptr ) [[unlikely]]
          sv_deallocate(old_storage);
        }
      else
        throw std::length_error{"Out of buffer space"};
      }
    else
      {
      auto dest_data{storage.data()};
      auto last = uninitialized_fn(dest_data );
      cond_null_terminate(last);
      }
    storage.size_ = sz;
    }
    
  template<typename vector_storage>
  inline constexpr void swap( vector_storage & lstorage, vector_storage & rstorage)
    {
    lstorage.swap(rstorage);
    cond_null_terminate(lstorage.data()+lstorage.size_);
    cond_null_terminate(rstorage.data()+rstorage.size_);
    }
    
  template<typename vector_storage, typename input_iterator>
  inline constexpr auto
  copy_construct( input_iterator first, input_iterator last )
    -> vector_storage
    {
    using size_type = typename vector_storage::size_type;
    return uninitialized_construct<vector_storage>(static_cast<size_type>(std::distance(first,last)),
                                                   [&first, &last](auto * data) noexcept
                                                   {
                                                   return uninitialized_copy(first, last, data );
                                                  });
    }
    
  template<typename vector_storage, typename input_iterator>
  inline constexpr void
  assign_copy( vector_storage & storage, input_iterator first, input_iterator last )
    {
    using size_type = typename vector_storage::size_type;
    uninitialized_assign(storage, static_cast<size_type>(std::distance(first,last)),
                         [&first, &last](auto * data) noexcept
                         {
                         return uninitialized_copy(first, last, data );
                         });
    }
    
  template<typename vector_storage>
  inline constexpr auto
  value_construct( typename vector_storage::size_type sz )
    -> vector_storage
    {

    return uninitialized_construct<vector_storage>(sz,
                                                   [sz](auto * data) noexcept
                                                   {
                                                   return uninitialized_value_construct(data, data+sz );
                                                  });
    }
    
  template<typename vector_storage>
  inline constexpr auto
  fill_construct( typename vector_storage::size_type sz, typename vector_storage::value_type ch )
      -> vector_storage
    {
    return uninitialized_construct<vector_storage>(sz,
                                                   [sz,ch](auto * data) noexcept
                                                   {
                                                   return uninitialized_fill(data, data+sz, ch );
                                                  });
    }
    
  template<typename vector_storage>
  inline constexpr void
  fill_assign( vector_storage & storage, typename vector_storage::size_type sz, typename vector_storage::value_type ch )
    {
    uninitialized_assign(storage, sz,
                         [sz,ch](auto * data) noexcept
                         {
                         return uninitialized_fill(data, data+sz, ch );
                         });
    }
    
  template<typename vector_storage>
  inline constexpr void
  reserve( vector_storage & storage, typename vector_storage::size_type req_capacity )
    {
    using char_type = typename vector_storage::value_type;
    using size_type = typename vector_storage::size_type;
    size_type const raw_size = req_capacity + null_termination;
    if( raw_size > storage.capacity())
      {
      if constexpr( vector_storage::supports_reallocation )
        {
        auto new_capacity{ growth<char_type>(raw_size)};
        storage_context_t new_space{ sv_allocate<char_type>(new_capacity) };
        auto first { storage.data() };
        auto last { first + storage.size_ + null_termination };
        uninitialized_copy(first, last, new_space.data ); // copies conditionally null termination
        
        //deallocate old space
        storage_context_t old_storage{ storage.exchange_priv_(new_space, storage.size_ ) };
        if(old_storage.data != nullptr ) [[unlikely]]
          sv_deallocate(old_storage);
        }
      else
        throw std::length_error{"Out of buffer space"};
      }
    }
  
  template<typename vector_storage, typename Operation >
  inline constexpr void resize_and_overwrite( vector_storage & storage,
                                              typename vector_storage::size_type sz, Operation op )
    {
    using char_type = typename vector_storage::value_type;
    using size_type = typename vector_storage::size_type;
    size_type const raw_size = sz + null_termination;

    if( raw_size <= storage.capacity())
      {
      auto data{storage.data()};
      typename vector_storage::size_type new_size{ op( data, storage.capacity() )};
      cond_null_terminate(data+new_size);
      storage.size_ = new_size;
      }
    else
      {
      if constexpr( vector_storage::supports_reallocation )
        {
        auto new_capacity{ growth<char_type>(raw_size)};
        storage_context_t new_space{ sv_allocate<char_type>(new_capacity) };
        auto data{storage.data()};
        uninitialized_copy(data, data + storage.size_, new_space.data );
        if (std::is_constant_evaluated())
          {
          // for constant evaluated mode we have to initialize storage to allow user operator assignment
          // in constant evaluated mode assignment to uninitialized store is not allowed
          uninitialized_value_construct( new_space.data + storage.size_, new_space.data + new_capacity );
          }
        typename vector_storage::size_type new_size{ op( new_space.data, new_capacity - null_termination )};
        cond_null_terminate(new_space.data+new_size);

        //deallocate old space
        storage_context_t old_storage{ storage.exchange_priv_(new_space, new_size ) };
        if(old_storage.data != nullptr ) [[unlikely]]
          sv_deallocate(old_storage);
        }
      else
        throw std::length_error{"Out of buffer space"};
      }
    }

  template<typename vector_storage>
  inline constexpr void
  resize( vector_storage & storage, typename vector_storage::size_type sz )
    {
    using char_type = typename vector_storage::value_type;
    using size_type = typename vector_storage::size_type;
    
    resize_and_overwrite(storage, sz, 
                           [old_size = storage.size_, sz](char_type * data, size_type /*buff_cap*/) noexcept -> size_type
                           {
                           if(sz>old_size)
                             uninitialized_value_construct( data + old_size, data + sz );
                           return sz;
                           });
    }
  template<typename vector_storage>
  inline constexpr void
  resize( vector_storage & storage, typename vector_storage::size_type sz, typename vector_storage::value_type ch )
    {
    using char_type = typename vector_storage::value_type;
    using size_type = typename vector_storage::size_type;
    
    resize_and_overwrite(storage, sz, 
                           [old_size = storage.size_, sz, ch](char_type * data, size_type /*buff_cap*/) noexcept -> size_type
                           {
                           if(sz>old_size)
                             uninitialized_fill( data + old_size, data + sz, ch );
                           return sz;
                           });
    }
    
    
  template<typename vector_storage, typename Operation >
  inline constexpr void replace_aux( vector_storage & storage,
                                     typename vector_storage::size_type pos,
                                     typename vector_storage::size_type old_size,
                                     typename vector_storage::size_type new_size,
                                     Operation op )
    {
    using char_type = typename vector_storage::value_type;
    using size_type = typename vector_storage::size_type;
    
    if( pos > storage.size_) [[unlikely]]
      throw std::out_of_range("index is out of range");
    
    size_type str_new_size = storage.size_ - old_size + new_size;
    size_type const raw_size = str_new_size + null_termination;
    if( raw_size <= storage.capacity())
      {
      // aaaaaRRRRRaaaaaa
      // aaaaaNNNNNaaaaaa no copy
      // aaaaaNNNaaaaaa shrinking
      // aaaaaNNNNNNNaaaaaa extending, case with 0 old size is just insert
      char_type * data{storage.data()};
       if( old_size == new_size )
        {}
      else if( old_size > new_size )
        uninitialized_copy( data + pos + old_size, data + storage.size_, data + pos + new_size );
      else
        {
        auto ito_end{ std::reverse_iterator{data + pos + old_size} };
        auto ito_begin{ std::reverse_iterator{data + storage.size_} };
        auto itn_begin{ std::reverse_iterator{data + storage.size_ - old_size + new_size } };
        uninitialized_copy( ito_begin, ito_end, itn_begin );
        }
      op( data + pos );
      cond_null_terminate(data + str_new_size);
      storage.size_ = str_new_size;
      }
    else
      {
      if constexpr( vector_storage::supports_reallocation )
        {
        size_type new_capacity{ growth<char_type>(raw_size)};
        storage_context_t new_space{ sv_allocate<char_type>(new_capacity) };
        char_type * data{storage.data()};
        uninitialized_copy(data, data + pos, new_space.data );
        op( new_space.data + pos );
        uninitialized_copy(data + pos + old_size, data + storage.size_, new_space.data + pos + new_size );
        cond_null_terminate(new_space.data+str_new_size);
        //deallocate old space
        storage_context_t old_storage{ storage.exchange_priv_(new_space, str_new_size ) };
        if(old_storage.data != nullptr ) [[unlikely]]
          sv_deallocate(old_storage);
        }
      else
        throw std::length_error{"Out of buffer space"};
      }
    }
    
  template<typename vector_storage, std::forward_iterator iterator>
  inline constexpr void replace_copy( vector_storage & storage,
                                      typename vector_storage::size_type pos,
                                      typename vector_storage::size_type count,
                                      iterator first, iterator last )
    {
    using size_type = typename vector_storage::size_type;
    using char_type = typename vector_storage::value_type;
    size_type new_count{ static_cast<size_type>(std::distance(first,last)) };
    replace_aux( storage, pos, count, new_count,
                [&first,&last]( char_type * data ) noexcept
                {
                uninitialized_copy(first,last, data );
                });
    }
    
  template<typename vector_storage>
  inline constexpr void replace_fill( vector_storage & storage,
                                 typename vector_storage::size_type pos,
                                 typename vector_storage::size_type count,
                                 typename vector_storage::size_type count2,
                                 typename vector_storage::value_type ch )
    {
    using char_type = typename vector_storage::value_type;
    replace_aux( storage, pos, count, count2,
                [count2,ch]( char_type * data ) noexcept
                {
                uninitialized_fill( data, data + count2, ch );
                });
    }

  template<typename vector_storage>
  inline constexpr void insert_fill( vector_storage & storage,
                                 typename vector_storage::size_type pos,
                                 typename vector_storage::size_type count,
                                 typename vector_storage::value_type ch )
    {
    using char_type = typename vector_storage::value_type;

    using size_type = typename vector_storage::size_type;
    replace_aux( storage, pos, size_type(0u), count,
                [count,ch]( char_type * data ) noexcept
                {
                uninitialized_fill( data, data + count, ch );
                });
    }
    
  template<typename vector_storage, std::forward_iterator iterator>
  inline constexpr void insert_copy(  vector_storage & storage,
                                      typename vector_storage::size_type pos,
                                      iterator first, iterator last )
    {
    using size_type = typename vector_storage::size_type;
    using char_type = typename vector_storage::value_type;
    size_type count{ static_cast<size_type>(std::distance(first,last)) };
    replace_aux( storage, pos, size_type(0u), count,
                [&first,&last]( char_type * data ) noexcept
                {
                uninitialized_copy(first,last, data );
                });
    }

  template<typename vector_storage>
  inline constexpr void append_fill(  vector_storage & storage,
                                 typename vector_storage::size_type count,
                                 typename vector_storage::value_type ch )
    {
    using char_type = typename vector_storage::value_type;
    using size_type = typename vector_storage::size_type;
    size_type new_size { static_cast<size_type>(storage.size_ + count) };
    resize_and_overwrite(storage, new_size, 
                           [old_size = storage.size_, new_size, ch](char_type * data, size_type /*buff_cap*/) noexcept -> size_type
                           {
                           uninitialized_fill( data + old_size, data + new_size, ch );
                           return new_size;
                           });
    }
    
  template<typename vector_storage, std::forward_iterator iterator>
  inline constexpr void append_copy(  vector_storage & storage,
                                      iterator first, iterator last )
    {
    using char_type = typename vector_storage::value_type;
    using size_type = typename vector_storage::size_type;
    size_type count{ static_cast<size_type>(std::distance(first,last)) };
    size_type new_size { static_cast<size_type>(storage.size_ + count) };
    resize_and_overwrite(storage, new_size, 
                           [old_size = storage.size_, new_size,first,last](char_type * data, size_type /*buff_cap*/) noexcept -> size_type
                           {
                           uninitialized_copy(first,last, data+ old_size );
                           return new_size;
                           });
    }
    
  template<typename vector_storage>
  inline constexpr void push_back(  vector_storage & storage, typename vector_storage::value_type ch )
    {
    append_fill(storage,1u,ch);
    }

    
  template<typename vector_storage>
  inline constexpr void 
  erase( vector_storage & storage,
         typename vector_storage::size_type index,
         typename vector_storage::size_type count )
    {
    using size_type = typename vector_storage::size_type;
    using char_type = typename vector_storage::value_type;
    
    count = std::min<size_type>(count, storage.size_ - index);
    size_type rpos = index + count;
    char_type * data{storage.data()};
    //if(rpos < storage.size_ )
      std::copy(data+rpos, data+storage.size_, data + index );
    storage.size_ -= count;
    cond_null_terminate(data + storage.size_);
    }
    
  template<typename vector_storage>
  inline constexpr void 
  pop_back( vector_storage & storage)
    {
    using char_type = typename vector_storage::value_type;
    char_type * data{storage.data()};
    storage.size_ -= 1u;
    cond_null_terminate(data + storage.size_);
    }

}

