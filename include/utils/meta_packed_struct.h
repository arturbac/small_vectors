#pragma once
#include <type_traits>
#include <cstdint>
#include <utility>
#include <cassert>
#include <concepts>

namespace utils
{

  namespace detail
    {
    template<typename T>
    concept enum_struct = std::is_enum_v<T> == true;
    
    template<typename T>
    concept underlaying_unsigned = 
      std::unsigned_integral<T>
      || (enum_struct<T> && std::unsigned_integral<std::underlying_type_t<T>>);

    template<auto tag_value, underlaying_unsigned T>
      requires enum_struct<decltype(tag_value)>
    struct tag_with_value { T value; };
    
    template<underlaying_unsigned value_type, auto tag_value, uint8_t BitWidth>
      requires enum_struct<decltype(tag_value)>
             && ((sizeof(value_type)*8) >= BitWidth )
    struct member
      {
      using value_t = value_type;
      using tag_type = decltype(tag_value);
      static constexpr tag_type tag() noexcept { return tag_value; }
      static constexpr uint8_t bit_width() noexcept { return BitWidth; }
      
      value_type value{};
      
      constexpr member() noexcept = default;
      
      template<typename U>
      constexpr member( tag_with_value<tag_value,U> obj ) noexcept
        : value{ obj.value }
        {}
      };
      
    template<auto tag_value, uint8_t bit_width, typename value_type>
    constexpr auto & get( member<value_type,tag_value,bit_width> & m) noexcept
      { return m.value; }
      
    template<auto tag_value, uint8_t bit_width, typename value_type>
    constexpr auto const & get( member<value_type,tag_value,bit_width> const & m) noexcept
      { return m.value; }
      
    template<unsigned counter, typename Member, typename ...Members>
    struct sub_member_t
      {
      using member_type = Member;
      using next_member_t = sub_member_t<(counter+1), Members...>;
      
      static constexpr unsigned index() noexcept { return counter; }
      };
      
    template<unsigned counter, typename Member>
    struct sub_member_t<counter,Member>
      {
      using member_type = Member;
      using next_member_t = void;
      static constexpr unsigned index() noexcept { return counter; }
      };

    
    template<auto tag_value>
      requires enum_struct<decltype(tag_value)>
    struct arg_type
      {
      template<underlaying_unsigned T>
      constexpr auto operator=(T value) const noexcept
        {
        return tag_with_value<tag_value,T>{ value };
        }
      };
    
    template<typename ... tags_and_values>
    struct params : tags_and_values ...
      {
      constexpr params( tags_and_values ... args ) noexcept
        : tags_and_values( args ) ...
        {}
      };
    
    template<typename ... tags_and_values>
    params( tags_and_values ... args ) -> params<tags_and_values...>;
    
    template<typename ...Members>
    struct meta_packed_struct_impl : Members... 
      {
      using first_member_t = detail::sub_member_t<0,Members...>;
      
      constexpr meta_packed_struct_impl() noexcept = default;
      
      template<typename Params>
      constexpr meta_packed_struct_impl( Params p) noexcept
          : Members{ p } ...
        {}
      };

    template<typename sub_member_type >
    consteval unsigned filed_count() noexcept 
      {
      using next_member_t = typename sub_member_type::next_member_t;
      if constexpr( std::is_same_v<void,next_member_t>)
        return sub_member_type::index() + 1;
      else
        return filed_count<next_member_t>();
      }
      
    template<typename sub_member_type >
    consteval unsigned bit_width() noexcept 
      {
      using next_member_t = typename sub_member_type::next_member_t;
      using member_type = typename sub_member_type::member_type;
      unsigned my_bit_width = member_type::bit_width();
      if constexpr( std::is_same_v<void,next_member_t>)
        return my_bit_width;
      else
        return bit_width<next_member_t>() + my_bit_width;
      }
      
    /// \brief compile time bitmsk calculating
    template <std::unsigned_integral T, uint32_t N>
    struct bitmask_m {
      static T constexpr value = bitmask_m < T, ( N - 1u ) >::value | T( T(1u) << ( N - 1 ) );
      };
      
    template <std::unsigned_integral T>
    struct bitmask_m<T, 0u> {
      static T constexpr value = T( 0u );
      };
    }
    
  /// \brief compile time bitmsk calculating
  template <std::unsigned_integral T, uint32_t number_of_bits>
  constexpr T bitmask_v = detail::bitmask_m<T,number_of_bits>::value;
    
  namespace detail
    {
    template<std::unsigned_integral pack_type, typename sub_member_type, typename meta_packed_struct>
    constexpr auto pack_value(unsigned offset,  meta_packed_struct const & ms )
        -> pack_type
      {
      using next_member_t = typename sub_member_type::next_member_t;
      using member_type = typename sub_member_type::member_type;
      //cast meta to exactly my self inherited type
      member_type const & self = static_cast<member_type const &>(ms);
      
      constexpr unsigned bit_width = member_type::bit_width();
      pack_type value { static_cast<pack_type>(self.value) };
      pack_type mask { bitmask_v<pack_type,bit_width> };
      if(std::is_constant_evaluated() )
        { if((mask & value) != value) throw "value outisde declared bit_width"; }
      else
        assert( (mask & value) == value );
      
      auto my_value_packed = value << offset;
      if constexpr( std::is_same_v<void,next_member_t>)
        return static_cast<pack_type>(my_value_packed);
      else
        return static_cast<pack_type>(pack_value<pack_type,next_member_t>(offset+bit_width,ms) | my_value_packed);
      }

    template<typename meta_packed_struct, typename sub_member_type, std::unsigned_integral pack_type>
    constexpr void unpack_value(meta_packed_struct& mps, unsigned offset, pack_type pack)
      {
      using next_member_t = typename sub_member_type::next_member_t;
      using member_type = typename sub_member_type::member_type;
      using value_type = typename member_type::value_t;

      constexpr unsigned bit_width = member_type::bit_width();

      pack = pack >> offset;
      auto& value{ detail::get<member_type::tag()>(mps) };
      constexpr auto mask{ bitmask_v<pack_type, bit_width> };
      value = static_cast<value_type>(pack & mask);

      if constexpr( !std::is_same_v<void, next_member_t>)
        return unpack_value<meta_packed_struct, next_member_t>(mps, bit_width, pack);
      }
    }

  using detail::member;
  template<auto tag_value>
    requires detail::enum_struct<decltype(tag_value)>
  inline constexpr auto arg = detail::arg_type<tag_value>{};
  
  template<typename ...Members>
  struct meta_packed_struct : detail::meta_packed_struct_impl<Members...> 
    {
    using base_type = detail::meta_packed_struct_impl<Members...>;
    
    constexpr meta_packed_struct() noexcept = default;
    
    template<typename ...tag_and_values>
    constexpr meta_packed_struct(tag_and_values ... args ) noexcept
      : base_type( detail::params(args...) )
    {}
    
    template<auto tag_value>
      requires detail::enum_struct<decltype(tag_value)>
    constexpr decltype(auto) get() noexcept
      {
      return detail::get<tag_value>( *this );
      }
    template<auto tag_value>
      requires detail::enum_struct<decltype(tag_value)>
    constexpr decltype(auto) get() const noexcept
      {
      return detail::get<tag_value>( *this );
      }
    };
    
  template<typename meta_packed_struct>
  consteval unsigned filed_count() noexcept 
    {
    return detail::filed_count<typename meta_packed_struct::first_member_t>();
    }
    
  template<typename meta_packed_struct>
  consteval unsigned bit_width() noexcept 
    {
    return detail::bit_width<typename meta_packed_struct::first_member_t>();
    }
    
  ///\brief packs all members into continous integral
  template<std::unsigned_integral pack_type, typename meta_packed_struct>
    requires (sizeof(pack_type)*8 >= bit_width<meta_packed_struct>())
  constexpr auto pack_value(meta_packed_struct const & ms ) noexcept
    {
    return detail::pack_value<pack_type,typename meta_packed_struct::first_member_t>(0,ms);
    }

  template<typename meta_packed_struct, std::unsigned_integral pack_type>
  constexpr auto unpack_value(pack_type pack)
    {
    meta_packed_struct mps{};
    detail::unpack_value<meta_packed_struct, typename meta_packed_struct::first_member_t>(mps, 0, pack);
    return mps;
    }
    
  template<auto tag_value, typename meta_packed_struct>
    requires detail::enum_struct<decltype(tag_value)>
  constexpr decltype(auto) get( meta_packed_struct && s )
    {
    return detail::get<tag_value>( std::forward<meta_packed_struct>(s) );
    }
    
}
