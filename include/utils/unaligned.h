#pragma once
#include <cstdint>
#include <type_traits>
#include <iterator>
#include <cstring>
#include <concepts>
#include <algorithm>
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
      template<typename value_type>
      concept trivially_copyable = std::is_trivially_copyable_v<value_type>;
      
      template<typename iterator_type>
      using iterator_value_type_t =typename std::iterator_traits<iterator_type>::value_type;
    
      template<typename iterator>
      concept forward_iterator_to_byte = (std::forward_iterator<iterator> && sizeof(iterator_value_type_t<iterator>) == 1);
      
      template<typename iterator>
      concept output_iterator_to_byte = std::output_iterator<iterator,uint8_t>;
      
      template<typename iterator>
      concept memory_location = std::same_as<std::remove_cv_t<iterator>,void *>;
  

      template<typename type>
      concept floating_point = std::is_floating_point<type>::value;
      
      template<typename type>
      concept integral = std::is_integral_v<type>;
      
      template<typename type>
      concept unsigned_integral = !std::is_signed<type>::value && integral<type>;
      
      template<typename type>
      concept signed_integral = std::is_signed<type>::value && integral<type>;
      
      template<typename type>
      concept arithmetic= requires
        {
        requires integral<type> or floating_point<type>;
        requires not std::same_as<type,bool>;
        };
      
      template<typename type>
      concept arithmetic_or_bool = arithmetic<type> or std::same_as<bool, std::remove_cv_t<type>>;
      ///\brief loads value from any forward iterator or unaligned int8_t/uint8_t memory location
      template<trivially_copyable output_type, forward_iterator_to_byte iterator>
      [[nodiscard,gnu::always_inline]]
      inline constexpr output_type unaligned_load( iterator it ) noexcept
        {
        using tmp_value_type = iterator_value_type_t<iterator>;
        static_assert(sizeof(tmp_value_type) ==1);
        
        std::aligned_storage_t<sizeof(output_type),alignof(output_type)> tmp_store;
        std::copy_n(it, sizeof(output_type), std::launder(reinterpret_cast<tmp_value_type *>(&tmp_store)) );
        return *std::launder(reinterpret_cast<output_type *>(&tmp_store) );
        }
        
      ///\brief loads value from any untyped pointer memory location
      template<trivially_copyable output_type, detail::memory_location memory_location>
      [[nodiscard,gnu::always_inline]]
      inline constexpr output_type unaligned_load( memory_location memloc ) noexcept
        {
        return detail::unaligned_load<output_type>(reinterpret_cast<uint8_t const *>(memloc));
        }
      }
    //---------------------------------------------------------------------------------------------------
    ///\brief loads value from any forward iterator or unaligned memory location
    template<detail::arithmetic_or_bool output_type, typename iterator>
    [[nodiscard,gnu::always_inline]]
    inline constexpr output_type unaligned_load( iterator it ) noexcept
      {
      return detail::unaligned_load<output_type>(it);
      }

    ///\brief loads enum value from any forward iterator or unaligned memory location with size restriction check
    ///\details specifing expected_storage_size prevents unintended breaking changes with IO to trivially_copyable underlaing type
    template<detail::trivially_copyable output_type, std::size_t expected_storage_size, typename iterator>
      requires( !detail::arithmetic_or_bool<output_type> && expected_storage_size == sizeof(output_type) )
    [[nodiscard,gnu::always_inline]]
    inline constexpr output_type unaligned_load( iterator it ) noexcept
      {
      return detail::unaligned_load<output_type>(it);
      }
    //---------------------------------------------------------------------------------------------------
    ///\brief loads value from any forward iterator or unaligned int8_t/uint8_t memory location and forwards iterator by the size of output_type and forwards the \ref it by the size was read
    ///\warning be aware that this function is prone to bugs when used in context of order of evaluation is unspecified, dont use it as function arguments, constructors because \ref it is modified when evaluated
    template<detail::arithmetic_or_bool output_type, typename iterator>
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
    template<detail::trivially_copyable output_type, std::size_t expected_storage_size, typename iterator>
      requires(!detail::arithmetic_or_bool<output_type>)
    [[nodiscard,gnu::always_inline]]
    inline constexpr output_type unaligned_load_fwd( iterator & it ) noexcept
      {
      iterator stored_at{ it };
      std::advance(it,sizeof(output_type));
      return unaligned_load<output_type,expected_storage_size>(stored_at);
      }
    //---------------------------------------------------------------------------------------------------
    ///\brief stores \param value at \param it location, input value type must match requested storage type
    template<detail::arithmetic_or_bool store_type,
             detail::output_iterator_to_byte iterator, detail::arithmetic_or_bool input_type>
      requires ( std::same_as<store_type,input_type> )
    [[gnu::always_inline]]
    inline constexpr iterator unaligned_store( iterator it, input_type value ) noexcept
       {
       return std::copy_n(std::launder(reinterpret_cast<uint8_t const *>(&value)), sizeof(store_type), it );
       }
       
    ///\brief stores \param value at \param it location, input value type must match requested storage type
    template<detail::trivially_copyable store_type, std::size_t expected_storage_size,
             detail::output_iterator_to_byte iterator, detail::trivially_copyable input_type>
      requires (std::same_as<store_type,input_type> 
             && expected_storage_size == sizeof(store_type)
             && !detail::arithmetic_or_bool<store_type> )
    [[gnu::always_inline]]
    inline constexpr iterator unaligned_store( iterator it, input_type value ) noexcept
       {
       return std::copy_n(std::launder(reinterpret_cast<uint8_t const *>(&value)), sizeof(store_type), it );
       }
       
     //---------------------------------------------------------------------------------------------------
     ///\brief cast from void * for working with deprecated software like wxwidgets storing user data as void *
     ///\returns \ref data casted by value to output_type
     template<detail::trivially_copyable output_type, typename pointer>
       requires ( sizeof(output_type) <= sizeof(uintptr_t) && std::same_as<void, pointer>)
     [[nodiscard,gnu::always_inline]]
     inline output_type void_ptr_cast( pointer const * data ) noexcept
       { return static_cast<output_type>(reinterpret_cast<uintptr_t>(data)); }
     
     ///\brief cast to void * for working with deprecated software like wxwidgets storing user data as void *
     template<detail::trivially_copyable input_type> 
       requires (!std::same_as<void *, std::remove_cvref_t<input_type>>)
     [[nodiscard,gnu::always_inline]]
     inline void * void_ptr_cast( input_type data ) noexcept
        {
        return reinterpret_cast<void *>(static_cast<uintptr_t>(data));
        }
    }
