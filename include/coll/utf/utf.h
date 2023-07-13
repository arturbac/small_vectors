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
    
    inline constexpr iterator begin() const noexcept { return begin_; }
    inline constexpr sentinel end() const noexcept { return end_; }
    };
  
  ///\brief returns code point length of utf sequence
  struct length_t
    {
    template<concepts::char_iterator source_iterator, std::sentinel_for<source_iterator> sentinel>
      requires (!concepts::u32bit_iterator<source_iterator>)
    static constexpr auto operator()(source_iterator beg, sentinel end) noexcept
      {
      return static_cast<std::size_t>(std::ranges::distance(utf_input_view_t{beg, end}));
      }

    template<concepts::u32bit_iterator source_iterator, std::sentinel_for<source_iterator> sentinel>
    static constexpr auto operator()(source_iterator beg, sentinel end) noexcept
      {
      //optimise if u32bit iterator is random access
      return static_cast<std::size_t>(std::ranges::distance(beg, end));
      }

    template<concepts::char_range forward_range>
    static constexpr auto operator()( forward_range const & range ) noexcept
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
    static constexpr auto operator()(source_iterator beg, sentinel end) noexcept
      {
      using code_point_size_t = typename detail::code_point_size_selector_t<sizeof(char_type)>::code_point_size_t;
      utf_input_view_t v{beg, end};
      return coll::ranges::accumulate( std::ranges::begin(v), std::ranges::end(v),
                                       std::size_t{}, [code_point_size = code_point_size_t{}]
                                       (std::size_t init, char32_t cp) noexcept
                                         { return init + code_point_size(cp);} );
      }

    template<concepts::char_iterator source_iterator, std::sentinel_for<source_iterator> sentinel>
      requires (sizeof(std::iter_value_t<source_iterator>) == sizeof(char_type))
    static constexpr auto operator()(source_iterator beg, sentinel end) noexcept
      {
      return static_cast<std::size_t>(std::ranges::distance(beg, end));
      }
      
    template<concepts::char_range forward_range>
    static constexpr auto operator()( forward_range const & range ) noexcept
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
    static constexpr auto operator()(source_iterator beg, sentinel end, target_iterator out ) noexcept
      {
      auto end_it{ std::ranges::copy( utf_input_view_t{beg, end}, utf_output_iterator_t(out) )};
      return end_it.out.iter_;
      }

    template<concepts::char_range forward_range, concepts::char_iterator target_iterator>
    static constexpr auto operator()( forward_range const & range, target_iterator out  ) noexcept
      {
      return operator()(std::ranges::begin(range), std::ranges::end(range), out );
      }
      

    };
    
  inline constexpr convert_t convert;
  
  template<concepts::char_type target_string_char, template<typename > typename basic_string_type = coll::basic_string>
  struct to_string_t
    {
    using string_type = basic_string_type<target_string_char>;
    
    template<concepts::char_iterator source_iterator, std::sentinel_for<source_iterator> sentinel>
    static constexpr auto operator()(source_iterator beg, sentinel end )
        -> string_type
      {
      using size_type = typename string_type::size_type;
      using capacity_type = capacity_t<target_string_char>;
      auto const req_capacity{static_cast<size_type>(capacity_type{}(beg,end))};
      string_type result;
      result.resize_and_overwrite(req_capacity,
                                  [beg,end](target_string_char * out, size_type capacity) noexcept
                                  {
                                  auto end_it{convert(beg, end, out)};
                                  return static_cast<size_type>(std::ranges::distance(out,end_it));
                                  });
      return result;
      }
      
    template<concepts::char_range forward_range>
    static constexpr auto operator()( forward_range const & range )
        -> string_type
      {
      return operator()(std::ranges::begin(range), std::ranges::end(range) );
      }
    };
    
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
}

