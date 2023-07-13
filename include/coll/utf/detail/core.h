#pragma once

#include "concepts.h"
#include <cassert>

namespace coll::utf::detail
{
  inline constexpr u16 lead_surrogate_min = 0xd800u;
  inline constexpr u16 lead_surrogate_max = 0xdbffu;
  inline constexpr u16 trail_surrogate_min = 0xdc00u;
  inline constexpr u16 trail_surrogate_max = 0xdfffu;
  inline constexpr u16 lead_offset{ lead_surrogate_min - (0x10000u >> 10)};
  inline constexpr u32 surrogate_offset { 0x10000u - (u32(lead_surrogate_min) << 10) - u32(trail_surrogate_min) };
  
  struct utf8_code_point_size_t
    {
    [[nodiscard]]
    static constexpr u8 operator()( std::same_as<char32_t> auto cp ) noexcept
      {
      if (cp < 0x80u)
        return 1u;
      else if (cp < 0x800u)
        return 2u;
      else if (cp < 0x10000u)
        return 3u;
      else
        return 4u;
      }
    };

  inline constexpr utf8_code_point_size_t utf8_code_point_size;

  struct utf16_code_point_size_t
    {
    [[nodiscard]]
    static constexpr u8 operator()( std::same_as<char32_t> auto cp ) noexcept
      {
      return (cp < 0xffffu) ? 1 : 2;
      }
    };

  inline constexpr utf16_code_point_size_t utf16_code_point_size;

  struct utf32_code_point_size_t
    {
    [[nodiscard]]
    static constexpr u8 operator()( std::same_as<char32_t> auto ) noexcept
      {
      return 1;
      }
    };

  template<std::size_t char_type>
  struct code_point_size_selector_t
  {};
  
  template<>
  struct code_point_size_selector_t<1>
    {
    using code_point_size_t = utf8_code_point_size_t;
    };
  
  template<>
  struct code_point_size_selector_t<2>
    {
    using code_point_size_t = utf16_code_point_size_t;
    };

  template<>
  struct code_point_size_selector_t<4>
    {
    using code_point_size_t = utf32_code_point_size_t;
    };

  [[nodiscard]]
  constexpr bool lead_surrogate(std::same_as<char16_t> auto cp) noexcept
    {
    return cp >= lead_surrogate_min && cp <= lead_surrogate_max;
    }

  struct sequence_length_t
    {
    [[nodiscard]]
    static constexpr u8 operator()( concepts::char_size<1> auto in_lead ) noexcept
      {
      auto lead = static_cast<u8>(in_lead);
      if( lead < 0x80u )
          return 1u;
      else if ((lead >> 5u) == 0x6u)
          return 2u;
      else if ((lead >> 4u) == 0xeu)
          return 3u;
      else if ((lead >> 3u) == 0x1eu)
          return 4u;
      else
        {
        assert(false);
        return 0u;
        }
      }
    [[nodiscard]]
    static constexpr u8 operator()( concepts::char_size<2> auto in_lead ) noexcept
      {
      auto cp {static_cast<char16_t>(in_lead)};
      return !lead_surrogate(cp) ? 1 : 2 ;
      }
      
    [[nodiscard]]
    static constexpr u8 operator()( concepts::char_size<4> auto ) noexcept
      { return 1; }
    };

  inline constexpr sequence_length_t sequence_length;
  inline constexpr u32 mask_3b {0b111u};
  inline constexpr u32 mask_4b {0b1111u};
  inline constexpr u32 mask_5b {0b1'1111u};
  inline constexpr u32 mask_6b {0b11'1111u};

  struct dereference_t
    {
    [[nodiscard]]
    static constexpr char32_t operator()(concepts::octet_iterator auto it) noexcept
      {
      switch(sequence_length(*it))
        {
        case 1:
          {
          //u+0000 	u+007f 	0xxxxxxx
          return static_cast<char32_t>(*it);
          }
        case 2:
          {
          //u+0080 	u+07ff 	110xxxxx 	10xxxxxx
          auto v0{static_cast<u32>(*it)}; ++it;
          auto v1{static_cast<u32>(*it)};
          return char32_t( ( (v0 & mask_5b) << 6) | (v1 & mask_6b));
          }
        case 3:
          {
          //u+0800 	u+ffff 	1110xxxx 	10xxxxxx 	10xxxxxx
          auto v0{static_cast<u32>(*it) }; ++it;
          auto v1{static_cast<u32>(*it)};  ++it;
          auto v2{static_cast<u32>(*it)};
          return char32_t(((v0 & mask_4b) << 12) | ((v1 & mask_6b) << 6) | (v2 & mask_6b));
          }
        case 4:
          {
          //u+10000 	[b]u+10ffff 	11110xxx 	10xxxxxx 	10xxxxxx 	10xxxxxx
          auto v0{ static_cast<u32>(*it) }; ++it;
          auto v1{static_cast<u32>(*it)};  ++it;
          auto v2{static_cast<u32>(*it)}; ++it;
          auto v3{static_cast<u32>(*it)};
          return char32_t(((v0 & mask_3b ) << 18) | ((v1 & mask_6b) << 12) | ((v2 & mask_6b) << 6) | (v3 & mask_6b));
          }
        }
      return {};
      }

    [[nodiscard]]
    static constexpr char32_t operator()(concepts::u16bit_iterator auto it) noexcept
      {
      auto cp {static_cast<char16_t>(*it)};
      if(lead_surrogate(cp))[[unlikely]]
        {
        ++it;
        auto trail_surrogate {static_cast<char32_t>(*it)};
        return (char32_t(cp) << 10) + trail_surrogate + surrogate_offset;
        }
      else
        return cp;
      }

    [[nodiscard]]
    static constexpr char32_t operator()(concepts::u32bit_iterator auto it) noexcept
      {
      return static_cast<char32_t>(*it);
      }
    };
    
  inline constexpr dereference_t dereference;

  inline constexpr u32 mask_10b  {0b11'1111'1111u};
  inline constexpr u32 lead_10   {0b10'000000};
  inline constexpr u32 lead_110  {0b110'00000};
  inline constexpr u32 lead_1110 {0b1110'0000};
  inline constexpr u32 lead_11110{0b11110'000};

  struct append_t
    {
    static constexpr auto operator()(std::same_as<char32_t> auto c, concepts::octet_iterator auto result) noexcept
      {
      using char_type = std::iter_value_t<decltype(result)>;
      auto cp{ static_cast<u32>(c)};
      if(cp < 0x80)
        {
        //u+0000 	u+007f 	0xxxxxxx
        *result = static_cast<char_type>(cp); ++result;
        }
      else if(cp < 0x800)
        {
        //u+0080 	u+07ff 	110xxxxx 	10xxxxxx
        *result = static_cast<char_type>(lead_110 | (cp >> 6) ); ++result;
        *result = static_cast<char_type>(lead_10 | (cp & mask_6b)); ++result;
        }
      else if(cp < 0x10000)
        {
        //u+0800 	u+ffff 	1110xxxx 	10xxxxxx 	10xxxxxx
        *result = static_cast<char_type>(lead_1110 | (cp >> 12)); ++result;
        *result = static_cast<char_type>(lead_10 | ((cp >> 6) & mask_6b)); ++result;
        *result = static_cast<char_type>(lead_10 | (cp & mask_6b)); ++result;
        }
      else
        {
        *result = static_cast<char_type>(lead_11110 | (cp >> 18)); ++result;
        *result = static_cast<char_type>(lead_10 | ((cp >> 12) & mask_6b)); ++result;
        *result = static_cast<char_type>(lead_10 | ((cp >> 6) & mask_6b)); ++result;
        *result = static_cast<char_type>(lead_10 | (cp & mask_6b)); ++result;
        }
      return result;
      }
      
    [[nodiscard]]
    static constexpr auto operator()(std::same_as<char32_t> auto cp, concepts::u16bit_iterator auto result ) noexcept
      {
      using char_type = std::iter_value_t<decltype(result)>;
      if (cp > 0xffffu)
        {
        *result = static_cast<char_type>((cp >> 10)   + lead_offset); ++result;
        *result = static_cast<char_type>((cp & mask_10b) + trail_surrogate_min); ++result;
        }
      else
        {
        *result = static_cast<char_type>(cp); ++result;
        }
      return result;
      }
      
    [[nodiscard]]
    static constexpr auto operator()(std::same_as<char32_t> auto cp, concepts::u32bit_iterator auto result ) noexcept
      {
      using char_type = std::iter_value_t<decltype(result)>;
      *result = static_cast<char_type>(cp); ++result;
      return result;
      }
    };
    
  inline constexpr append_t append;
}
