#pragma once
#include <cstdint>
#include <type_traits>
#include <iterator>
#include <cstring>
#include <aq_concepts.h>
#include <bit>
#include <utils/utility_cxx20.h>

   // -----------------------------------------------------------------------
   //
   // Functions
   //
   // -----------------------------------------------------------------------
  namespace memutil
    {
    namespace detail
      {
      template<typename iterator>
      concept forward_iterator_to_byte = (std::forward_iterator<iterator> && sizeof(aq::iterator_value_type_t<iterator>) == 1);
      
      template<typename iterator>
      concept output_iterator_to_byte = std::output_iterator<iterator,uint8_t>;
      
      template<typename iterator>
      concept memory_location = std::same_as<std::remove_cv_t<iterator>,void *>;
      
    
      ///\brief loads value from any forward iterator or unaligned int8_t/uint8_t memory location
      template<aq::trivially_copyable output_type, detail::forward_iterator_to_byte iterator>
      [[nodiscard,gnu::always_inline]]
      inline constexpr output_type unaligned_load( iterator it ) noexcept
        {
        using tmp_value_type = aq::iterator_value_type_t<iterator>;
        static_assert(sizeof(tmp_value_type) ==1);
        
        std::aligned_storage_t<sizeof(output_type),alignof(output_type)> tmp_store;
        std::copy_n(it, sizeof(output_type), std::launder(reinterpret_cast<tmp_value_type *>(&tmp_store)) );
        return *std::launder(reinterpret_cast<output_type *>(&tmp_store) );
        }
        
      ///\brief loads value from any untyped pointer memory location
      template<aq::trivially_copyable output_type, detail::memory_location memory_location>
      [[nodiscard,gnu::always_inline]]
      inline constexpr output_type unaligned_load( memory_location memloc ) noexcept
        {
        return detail::unaligned_load<output_type>(reinterpret_cast<uint8_t const *>(memloc));
        }
      }
    //---------------------------------------------------------------------------------------------------
    ///\brief loads value from any forward iterator or unaligned memory location
    template<aq::arithmetic_or_bool output_type, typename iterator>
    [[nodiscard,gnu::always_inline]]
    inline constexpr output_type unaligned_load( iterator it ) noexcept
      {
      return detail::unaligned_load<output_type>(it);
      }

    ///\brief loads enum value from any forward iterator or unaligned memory location with size restriction check
    ///\details specifing expected_storage_size prevents unintended breaking changes with IO to trivially_copyable underlaing type
    template<aq::trivially_copyable output_type, std::size_t expected_storage_size, typename iterator>
      requires( !aq::arithmetic_or_bool<output_type> && expected_storage_size == sizeof(output_type) )
    [[nodiscard,gnu::always_inline]]
    inline constexpr output_type unaligned_load( iterator it ) noexcept
      {
      return detail::unaligned_load<output_type>(it);
      }
    //---------------------------------------------------------------------------------------------------
    ///\brief loads value from any forward iterator or unaligned int8_t/uint8_t memory location and forwards iterator by the size of output_type and forwards the \param it by the size was read
    ///\warning be aware that this function is prone to bugs when used in context of order of evaluation is unspecified, dont use it as function arguments, constructors because \param it is modified when evaluated
    template<aq::arithmetic_or_bool output_type, typename iterator>
    [[nodiscard,gnu::always_inline]]
    inline constexpr output_type unaligned_load_fwd( iterator & it ) noexcept
      {
      iterator stored_at{ it };
      std::advance(it,sizeof(output_type));
      return unaligned_load<output_type>(stored_at);
      }
    ///\brief loads enum value from any forward iterator or unaligned int8_t/uint8_t memory location and forwards iterator by the size of output_type and forwards the \param it by the size was read
    ///\details specifing expected_storage_size prevents unintended breaking changes with IO to enum underlaing type
    ///\warning be aware that this function is prone to bugs when used in context of order of evaluation is unspecified, dont use it as function arguments, constructors
    template<aq::trivially_copyable output_type, std::size_t expected_storage_size, typename iterator>
      requires(!aq::arithmetic_or_bool<output_type>)
    [[nodiscard,gnu::always_inline]]
    inline constexpr output_type unaligned_load_fwd( iterator & it ) noexcept
      {
      iterator stored_at{ it };
      std::advance(it,sizeof(output_type));
      return unaligned_load<output_type,expected_storage_size>(stored_at);
      }
    //---------------------------------------------------------------------------------------------------
    ///\brief stores \param value at \param it location, input value type must match requested storage type
    template<aq::arithmetic_or_bool store_type,
             detail::output_iterator_to_byte iterator, aq::arithmetic_or_bool input_type>
      requires ( std::same_as<store_type,input_type> )
    [[gnu::always_inline]]
    inline constexpr iterator unaligned_store( iterator it, input_type value ) noexcept
       {
       return std::copy_n(reinterpret_cast<uint8_t const *>(&value), sizeof(store_type), it );
       }
       
    ///\brief stores \param value at \param it location, input value type must match requested storage type
    template<aq::trivially_copyable store_type, std::size_t expected_storage_size,
             detail::output_iterator_to_byte iterator, aq::trivially_copyable input_type>
      requires (std::same_as<store_type,input_type> 
             && expected_storage_size == sizeof(store_type)
             && !aq::arithmetic_or_bool<store_type> )
    [[gnu::always_inline]]
    inline constexpr iterator unaligned_store( iterator it, input_type value ) noexcept
       {
       return std::copy_n(reinterpret_cast<uint8_t const *>(&value), sizeof(store_type), it );
       }
       
     //---------------------------------------------------------------------------------------------------
     ///\brief cast from void * for working with deprecated software like wxwidgets storing user data as void *
     ///\returns \param data casted by value to output_type
     template<aq::trivially_copyable output_type, typename pointer>
       requires ( sizeof(output_type) <= sizeof(uintptr_t) && aq::same_as<void, pointer>)
     [[nodiscard,gnu::always_inline]]
     inline output_type void_ptr_cast( pointer const * data ) noexcept
       { return static_cast<output_type>(reinterpret_cast<uintptr_t>(data)); }
     
     ///\brief cast to void * for working with deprecated software like wxwidgets storing user data as void *
     template<aq::trivially_copyable input_type> 
       requires (!aq::same_as<void *, aq::remove_cvref_t<input_type>>)
     [[nodiscard,gnu::always_inline]]
     inline void * void_ptr_cast( input_type data ) noexcept
        {
        return reinterpret_cast<void *>(static_cast<uintptr_t>(data));
        }
    }
    
  template<typename iterator>
  using is_iterator_on_uint8_t = 
    typename std::is_same<uint8_t,aq::iterator_value_type_t<iterator>>;


  template<typename iterator>
  using enable_if_uint8_t_random_access_iterator_t =
    std::enable_if<is_iterator_on_uint8_t<iterator>::value && aq::is_random_access_iterator_v<iterator>, void>;


  template<
    typename iterator,
    typename = std::enable_if_t<is_iterator_on_uint8_t<iterator>::value, void > >
  inline uint8_t read_uint8_fwd( iterator & p )
    {
    iterator pin =p;
    std::advance(p,1);
    return *pin;
    }
    
  template<
    typename iterator,
    typename = std::enable_if_t<is_iterator_on_uint8_t<iterator>::value, void > >
  inline void write_uint8_fwd( iterator & p, uint8_t value )
    {
    *p = value;
    std::advance(p,1);
    }
    
  template<
    typename iterator,
    typename = typename enable_if_uint8_t_random_access_iterator_t<iterator>::type >
  inline uint16_t read_uint16_at( iterator p )
    {
    return static_cast<uint16_t>
            ( static_cast<uint16_t>(p[0]) | 
            ( static_cast<uint16_t>(p[1])<<8) );
    }
  
  template<typename iterator,
          typename = typename enable_if_uint8_t_random_access_iterator_t<iterator>::type >
  inline uint16_t read_uint16_fwd( iterator & p )
    {
    iterator pin =p;
    std::advance(p, 2);
    return read_uint16_at(pin);
    }
      
  template<typename iterator,
          typename = typename enable_if_uint8_t_random_access_iterator_t<iterator>::type >
  inline int16_t read_int16_at( iterator p )
    {
    return cxx20::bit_cast<int16_t>( read_uint16_at(p) );
    }
      
  template<typename iterator,
          typename = typename enable_if_uint8_t_random_access_iterator_t<iterator>::type >
  inline void write_uint16_fwd( iterator & p, uint16_t value )
    {
    p[0] = static_cast<uint8_t>(value & 0xFF);
    p[1] = static_cast<uint8_t>((value >> 8) & 0xFF);
    std::advance(p, 2);
    }
    
  template<typename iterator,
          typename = typename enable_if_uint8_t_random_access_iterator_t<iterator>::type >
  inline uint32_t read_uint24_at( iterator p )
    {
    return static_cast<uint32_t>(p[0])
        | ( static_cast<uint32_t>(p[1])<<8)
        | ( static_cast<uint32_t>(p[2])<<16);
    }

  template<typename iterator,
          typename = typename enable_if_uint8_t_random_access_iterator_t<iterator>::type >
  inline uint32_t read_uint24_fwd( iterator & p )
    {
    iterator pin =p;
    std::advance(p, 3);
    return read_uint24_at(pin);
    }
    
  template<typename iterator,
          typename = typename enable_if_uint8_t_random_access_iterator_t<iterator>::type >
  inline void write_uint24_fwd( iterator & p, uint32_t value )
    {
    p[0] = static_cast<uint8_t>(value & 0xFF);
    p[1] = static_cast<uint8_t>((value >> 8) & 0xFF);
    p[2] = static_cast<uint8_t>((value >> 16) & 0xFF);
    std::advance(p, 3);
    }
    
  template<typename iterator,
          typename = typename enable_if_uint8_t_random_access_iterator_t<iterator>::type >
  inline int32_t read_int24_at( iterator p )
    {
    return cxx20::bit_cast<int32_t>( read_uint24_at(p) );
    }
    
  template<typename iterator,
          typename = typename enable_if_uint8_t_random_access_iterator_t<iterator>::type >
  inline int32_t read_int24_fwd( iterator & p )
    {
    iterator pin = p;
    std::advance(p, 3);
    return read_int24_at(pin);
    }
      
  template<typename iterator,
          typename = typename enable_if_uint8_t_random_access_iterator_t<iterator>::type >
  inline uint32_t read_uint32_at( iterator p )
    {
    return static_cast<uint32_t>(p[0])
        | ( static_cast<uint32_t>(p[1])<<8)
        | ( static_cast<uint32_t>(p[2])<<16)
        | ( static_cast<uint32_t>(p[3])<<24);
    }

  template<typename iterator,
          typename = typename enable_if_uint8_t_random_access_iterator_t<iterator>::type >
  inline uint32_t read_uint32_fwd( iterator & p )
    {
    iterator pin =p;
    std::advance(p, 4);
    return read_uint32_at(pin);
    }

  template<typename iterator,
          typename = typename enable_if_uint8_t_random_access_iterator_t<iterator>::type >
  inline void write_uint32_fwd( iterator & p, uint32_t value )
    {
    p[0] = static_cast<uint8_t>(value & 0xFF);
    p[1] = static_cast<uint8_t>((value >> 8) & 0xFF);
    p[2] = static_cast<uint8_t>((value >> 16) & 0xFF);
    p[3] = static_cast<uint8_t>((value >> 24) & 0xFF);
    std::advance(p, 4);
    }
    
  template<typename iterator,
          typename = typename enable_if_uint8_t_random_access_iterator_t<iterator>::type >
  inline int32_t read_int32_at( iterator p )
    {
    return cxx20::bit_cast<int32_t>( read_uint32_at(p) );
    }
    
  template<typename iterator,
          typename = typename enable_if_uint8_t_random_access_iterator_t<iterator>::type >
  inline int32_t read_int32_fwd( iterator & p )
    {
    iterator pin =p;
    std::advance(p, 4);
    return read_int32_at(pin);
    }
      
  template<typename iterator,
          typename = typename enable_if_uint8_t_random_access_iterator_t<iterator>::type >
  inline uint64_t read_uint64_at( iterator p )
    {
    return static_cast<uint64_t>(p[0])
        | ( static_cast<uint64_t>(p[1])<<8)
        | ( static_cast<uint64_t>(p[2])<<16)
        | ( static_cast<uint64_t>(p[3])<<24)
        | ( static_cast<uint64_t>(p[4])<<32)
        | ( static_cast<uint64_t>(p[5])<<40)
        | ( static_cast<uint64_t>(p[6])<<48)
        | ( static_cast<uint64_t>(p[7])<<56);
    }
      
  template<typename iterator,
          typename = typename enable_if_uint8_t_random_access_iterator_t<iterator>::type >
  inline uint64_t read_uint64_fwd( iterator & p )
    {
    iterator pin =p;
    std::advance(p, 8);
    return read_uint64_at(pin);
    }
      
  template<typename iterator,
          typename = typename enable_if_uint8_t_random_access_iterator_t<iterator>::type >
  inline int64_t read_int64_at( iterator p )
    {
    return cxx20::bit_cast<int64_t>( read_uint64_at(p) );
    }
      
  template<typename iterator,
          typename = typename enable_if_uint8_t_random_access_iterator_t<iterator>::type >
  inline void write_uint64_fwd( iterator & p, uint64_t value )
    {
    p[0] = static_cast<uint8_t>(value & 0xFF);
    p[1] = static_cast<uint8_t>((value >> 8) & 0xFF);
    p[2] = static_cast<uint8_t>((value >> 16) & 0xFF);
    p[3] = static_cast<uint8_t>((value >> 24) & 0xFF);
    p[4] = static_cast<uint8_t>((value >> 32) & 0xFF);
    p[5] = static_cast<uint8_t>((value >> 40) & 0xFF);
    p[6] = static_cast<uint8_t>((value >> 48) & 0xFF);
    p[7] = static_cast<uint8_t>((value >> 56) & 0xFF);
    std::advance(p, 8);
    }
    
  inline double read_double_at( uint8_t const *p )
    {
    return cxx20::bit_cast<double>( read_uint64_at(p) );
    }


