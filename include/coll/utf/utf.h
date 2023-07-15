#pragma once

#include "detail/core.h"
#include <coll/basic_string.h>
#include <coll/ranges/accumulate.h>

namespace coll::utf
{
  template<concepts::char_iterator SourceIter>
  struct utf_forward_iterator_t
    {
    using value_type = char32_t;
    using difference_type = ptrdiff_t;
    using source_iterator = SourceIter;
    using source_value_type = std::iter_value_t<source_iterator>;
    using iterator_category = std::forward_iterator_tag;
    
    source_iterator iter_{};
    
    constexpr utf_forward_iterator_t() = default;
    constexpr explicit utf_forward_iterator_t( source_iterator src ) : iter_{src} {}
    
    [[nodiscard]]
    inline constexpr value_type operator*() const noexcept 
      {
      return detail::dereference(iter_);
      }
      
    inline constexpr utf_forward_iterator_t & operator++() noexcept
      {
      std::advance(iter_,detail::sequence_length(*iter_));
      return *this;
      }

    [[nodiscard]]
    inline constexpr utf_forward_iterator_t operator++(int) noexcept
      {
      utf_forward_iterator_t copy{iter_};
      std::advance(iter_,detail::sequence_length(*iter_));
      return copy;
      }
      
    [[nodiscard]]
    constexpr bool operator == (utf_forward_iterator_t const & rh ) const noexcept = default;
    
    [[nodiscard]]
    constexpr auto operator <=> (utf_forward_iterator_t const & rh ) const noexcept = default;
    };
    
  template<concepts::char_iterator TargetIter>
  struct utf_output_iterator_t
    {
    using value_type = char32_t;
    using difference_type = ptrdiff_t;
    using target_iterator = TargetIter;
    using iterator_category = std::output_iterator_tag;
    
    target_iterator iter_;
    
    constexpr utf_output_iterator_t() noexcept = default;
    inline constexpr explicit utf_output_iterator_t( target_iterator src ) noexcept : iter_{src} {}
    
    inline constexpr utf_output_iterator_t & operator=( std::same_as<char32_t> auto cp) noexcept
      {
      iter_  = detail::append(cp, iter_);
      return *this;
      }
      
    inline constexpr utf_output_iterator_t & operator*() noexcept
      { return *this; }
      
    inline constexpr utf_output_iterator_t & operator++() noexcept
      { return *this; }
    
    [[nodiscard]]
    inline constexpr utf_output_iterator_t operator++(int) noexcept
      { return *this; }
    };
  
  //TODO write sentinel adapter for source sentinel
  template<concepts::char_iterator SourceIter,
           std::sentinel_for<utf_forward_iterator_t<SourceIter>> Sentinel = utf_forward_iterator_t<SourceIter>>
  struct utf_input_view_t
       : public std::ranges::view_interface<utf_input_view_t<SourceIter,Sentinel>>
    {
    using source_iterator = SourceIter;
    using iterator = utf_forward_iterator_t<source_iterator>;
    using sentinel = Sentinel;
    
    iterator begin_;
    sentinel end_;
    
    inline constexpr utf_input_view_t() noexcept = default;
    
    inline constexpr utf_input_view_t( source_iterator b, source_iterator e) noexcept 
        : begin_{b}, end_{e}
      {}
    inline constexpr utf_input_view_t( iterator b, sentinel e) noexcept 
        : begin_{b}, end_{e}
      {}

    template<concepts::char_range forward_range>
    explicit inline constexpr utf_input_view_t( forward_range const & range )
        : begin_{utf_forward_iterator_t(std::ranges::begin(range))}, end_{utf_forward_iterator_t(std::ranges::end(range))}
    {}

    [[nodiscard]]
    inline constexpr iterator begin() const noexcept { return begin_; }
    
    [[nodiscard]]
    inline constexpr sentinel end() const noexcept { return end_; }
    };
    
  template< class T >
  using const_iterator_t = decltype(std::ranges::begin(std::declval<T const&>()));

  template<concepts::char_range forward_range>
  utf_input_view_t( forward_range const & r) -> utf_input_view_t<const_iterator_t<forward_range>>;
   
  ///\brief returns code point length of utf sequence
  struct length_t
    {
    template<concepts::char_iterator source_iterator, std::sentinel_for<source_iterator> sentinel>
      requires (!concepts::u32bit_iterator<source_iterator>)
    [[nodiscard]]
    small_vector_cpp_static_call_operator
    constexpr auto operator()(source_iterator beg, sentinel end)
        small_vector_static_call_operator_const noexcept
      {
      // return static_cast<std::size_t>(std::ranges::distance(utf_input_view_t{beg, end}));
      return static_cast<std::size_t>(std::distance(utf_forward_iterator_t{beg}, utf_forward_iterator_t{end}));
      }

    template<concepts::u32bit_iterator source_iterator, std::sentinel_for<source_iterator> sentinel>
    [[nodiscard]]
    small_vector_cpp_static_call_operator
    constexpr auto operator()(source_iterator beg, sentinel end)
        small_vector_static_call_operator_const noexcept
      {
      //optimise if u32bit iterator is random access
      return static_cast<std::size_t>(std::ranges::distance(beg, end));
      }

    template<concepts::char_range forward_range>
    [[nodiscard]]
    small_vector_cpp_static_call_operator
    constexpr auto operator()( forward_range const & range )
        small_vector_static_call_operator_const noexcept
      {
      return operator()(std::ranges::begin(range), std::ranges::end(range) );
      }
    };
  inline constexpr length_t length;


  template<concepts::char_type char_type>
  struct capacity_t
    {
    template<concepts::char_iterator source_iterator, std::sentinel_for<source_iterator> sentinel>
      requires (sizeof(std::iter_value_t<source_iterator>) != sizeof(char_type))
    [[nodiscard]]
    small_vector_cpp_static_call_operator
    constexpr auto operator()(source_iterator beg, sentinel end)
        small_vector_static_call_operator_const noexcept
      {
      using code_point_size_t = typename detail::code_point_size_selector_t<sizeof(char_type)>::code_point_size_t;
      return coll::ranges::accumulate(utf_forward_iterator_t{beg}, utf_forward_iterator_t{end},
                                       std::size_t{}, [code_point_size = code_point_size_t{}]
                                       (std::size_t init, char32_t cp) noexcept
                                         { return init + code_point_size(cp);} );
      }

    template<concepts::char_iterator source_iterator, std::sentinel_for<source_iterator> sentinel>
      requires (sizeof(std::iter_value_t<source_iterator>) == sizeof(char_type))
    [[nodiscard]]
    small_vector_cpp_static_call_operator
    constexpr auto operator()(source_iterator beg, sentinel end)
        small_vector_static_call_operator_const noexcept
      {
      return static_cast<std::size_t>(std::ranges::distance(beg, end));
      }
      
    template<concepts::char_range forward_range>
    [[nodiscard]]
    small_vector_cpp_static_call_operator
    constexpr auto operator()( forward_range const & range )
        small_vector_static_call_operator_const noexcept
      {
      return operator()(std::ranges::begin(range), std::ranges::end(range) );
      }
    };
  
  inline constexpr capacity_t<char8_t> u8capacity;
  inline constexpr capacity_t<char16_t> u16capacity;
  inline constexpr capacity_t<char32_t> u32capacity;
  

  /// \brief converts any utf encoding into any other utf encoding
  struct convert_t
    {
    template<concepts::char_iterator source_iterator, std::sentinel_for<source_iterator> sentinel,
             concepts::char_iterator target_iterator>
      requires (sizeof(std::iter_value_t<source_iterator>) != sizeof(std::iter_value_t<target_iterator>))
    small_vector_cpp_static_call_operator
    constexpr auto operator()(source_iterator beg, sentinel end, target_iterator out )
        small_vector_static_call_operator_const noexcept
      {
      // auto end_it{ std::ranges::copy( utf_input_view_t{beg, end}, utf_output_iterator_t(out) )};
      auto end_it{ std::ranges::copy( utf_forward_iterator_t{beg}, utf_forward_iterator_t{end}, utf_output_iterator_t(out) )};
      return end_it.out.iter_;
      }

    template<concepts::char_range forward_range, concepts::char_iterator target_iterator>
    small_vector_cpp_static_call_operator
    constexpr auto operator()( forward_range const & range, target_iterator out  )
        small_vector_static_call_operator_const noexcept
      {
      return operator()(std::ranges::begin(range), std::ranges::end(range), out );
      }
    };
    
  inline constexpr convert_t convert;
  namespace detail
    {
    template<template<typename > typename basic_string_type>
    struct string_cpp_lib_string_resize_and_overwrite_t
      {
      static constexpr bool value = false;
      };
    }
    
  template<concepts::char_type target_string_char, template<typename > typename basic_string_type = coll::basic_string>
  struct to_string_t
    {
    using string_type = basic_string_type<target_string_char>;
    
    template<concepts::char_iterator source_iterator, std::sentinel_for<source_iterator> sentinel>
    [[nodiscard]]
    small_vector_cpp_static_call_operator
    constexpr auto operator()(source_iterator beg, sentinel end )
        small_vector_static_call_operator_const
        -> string_type
      {
      using size_type = typename string_type::size_type;
      using capacity_type = capacity_t<target_string_char>;
      
      auto const req_capacity{static_cast<size_type>(capacity_type{}(beg,end))};
      string_type result;
      if constexpr(detail::string_cpp_lib_string_resize_and_overwrite_t<basic_string_type>::value)
        result.resize_and_overwrite(req_capacity,
                                    [beg,end](target_string_char * out, size_type) noexcept
                                    {
                                    auto end_it{convert(beg, end, out)};
                                    return static_cast<size_type>(std::ranges::distance(out,end_it));
                                    });
      else
        {
        result.resize(req_capacity);
        auto end_it{ convert(beg, end, result.begin()) };
        result.resize(static_cast<size_type>(std::ranges::distance(result.begin(),end_it)));
        }
      return result;
      }
      
    template<concepts::char_range forward_range>
    [[nodiscard]]
    small_vector_cpp_static_call_operator
    constexpr auto operator()( forward_range const & range )
        small_vector_static_call_operator_const
        -> string_type
      {
      return operator()(std::ranges::begin(range), std::ranges::end(range) );
      }
    };
  namespace detail
    {
    template<>
    struct string_cpp_lib_string_resize_and_overwrite_t<coll::basic_string>
      {
      static constexpr bool value = true;
      };
    }
    
  inline constexpr to_string_t<char> to_string;
  inline constexpr to_string_t<char8_t> to_u8string;
  inline constexpr to_string_t<char16_t> to_u16string;
  inline constexpr to_string_t<char32_t> to_u32string;
  inline constexpr to_string_t<wchar_t> to_wstring;
  
  namespace stl
    {
    template<concepts::char_type char_type>
    using basic_string = std::basic_string<char_type>;
    
    inline constexpr to_string_t<char, coll::utf::stl::basic_string> to_string;
    inline constexpr to_string_t<char8_t,coll::utf::stl::basic_string> to_u8string;
    inline constexpr to_string_t<char16_t,coll::utf::stl::basic_string> to_u16string;
    inline constexpr to_string_t<char32_t,coll::utf::stl::basic_string> to_u32string;
    inline constexpr to_string_t<wchar_t,coll::utf::stl::basic_string> to_wstring;
    }
  namespace detail
    {
    template<>
    struct string_cpp_lib_string_resize_and_overwrite_t<coll::utf::stl::basic_string>
      {
  #if defined(__cpp_lib_string_resize_and_overwrite)
      static constexpr bool value = true;
  #else
      static constexpr bool value = false;
  #endif
      };
    }
    
  using detail::verify_status_e;
  
  struct verify_t
    {
    template<concepts::octet_iterator octet_iterator, std::sentinel_for<octet_iterator> sentinel>
    small_vector_cpp_static_call_operator
    constexpr auto operator()(octet_iterator beg, sentinel end )
        small_vector_static_call_operator_const noexcept
        -> verify_status_e
      {
      using enum verify_status_e;
      verify_status_e result{};
      while( beg != end && result <= overlength_code_point )
        std::tie(result,beg) = detail::verify_code_point(beg, end);

      return result;
      }
      
    template<concepts::char_range forward_range>
    [[nodiscard]]
    small_vector_cpp_static_call_operator
    constexpr auto operator()( forward_range const & range )
        small_vector_static_call_operator_const
        -> verify_status_e
      {
      return operator()(std::ranges::begin(range), std::ranges::end(range) );
      }
    };

  inline constexpr verify_t verify;
}

