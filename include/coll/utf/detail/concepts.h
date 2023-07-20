#pragma once

#include <concepts>
#include <iterator>
#include <ranges>
#include <utils/static_call_operator.h>

#if !defined(__cpp_unicode_characters)
#error "missing __cpp_unicode_characters support"
#endif
#if !defined(__cpp_char8_t)
#error "missing __cpp_char8_t support"
#endif

namespace coll::utf
{
  using u8 = std::uint8_t;
  using u16 = std::uint16_t;
  using u32 = std::uint32_t;
  
  namespace concepts
    {
    template<typename value_type>
    concept char_1b_type = std::same_as<std::remove_cvref_t<value_type>,char>
                          || std::same_as<std::remove_cvref_t<value_type>,char8_t>;

    template<typename value_type>
    concept char_type = char_1b_type<value_type>
                      || std::same_as<std::remove_cvref_t<value_type>,char16_t>
                      || std::same_as<std::remove_cvref_t<value_type>,char32_t>
                      || std::same_as<std::remove_cvref_t<value_type>,wchar_t>;
    
    template<typename value_type, size_t req_char_size>
    concept char_size = char_type<value_type> && sizeof(value_type) == req_char_size;
    
    template<typename iterator_type>
    concept char_iterator = std::forward_iterator<iterator_type> && char_type<std::iter_value_t<iterator_type>>;
    
    template<typename iterator_type, size_t req_char_size>
    concept sized_char_iterator = char_iterator<iterator_type> && char_size<std::iter_value_t<iterator_type>,req_char_size>;
    
    template<typename iterator_type>
    concept octet_iterator = sized_char_iterator<iterator_type,1>;
    
    template<typename iterator_type>
    concept u16bit_iterator = sized_char_iterator<iterator_type,2>;
    
    template<typename iterator_type>
    concept u32bit_iterator = sized_char_iterator<iterator_type,4>;
    
    template<typename range>
    concept char_range = std::ranges::forward_range<range> && char_type<std::ranges::range_value_t<range>>;
    }
}
