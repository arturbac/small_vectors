#include <utils/meta_packed_struct.h>

#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#define CONSTEXPR_TEST( a ) \
  if( !(a) ) throw
    
namespace utils
{

enum struct acs_fields 
  {
    field_1, field_2, field_3
  };

template<acs_fields tag_value>
using acs_member = member<bool,tag_value, 1>;

using bool_bitfiled_struct = 
  meta_packed_struct<
    acs_member<acs_fields::field_1>,
    acs_member<acs_fields::field_2>,
    acs_member<acs_fields::field_3>
    >;

    
bool consteval consteval_test_metabitstruct_bool()
  {
  using enum acs_fields;
  constexpr auto fcount = filed_count<bool_bitfiled_struct>();
  CONSTEXPR_TEST(fcount == 3);
  constexpr auto s_bit_width = bit_width<bool_bitfiled_struct>();
  CONSTEXPR_TEST(s_bit_width == 3);
    {
    bool_bitfiled_struct acr;
    decltype(auto) cacr{ const_cast<bool_bitfiled_struct const &>(acr) };
    
    get<field_1>(acr) = true;
    get<field_3>(acr) = true;
    
    auto res = get<field_1>(cacr);
    CONSTEXPR_TEST(res == true );
    res = get<field_2>(acr);
    CONSTEXPR_TEST(res == false );
    
    auto packed_value = pack_value<uint8_t>(acr);
    CONSTEXPR_TEST(packed_value == 0b101 );
    }
    
    {
    bool_bitfiled_struct acr;
    decltype(auto) cacr{ const_cast<bool_bitfiled_struct const &>(acr) };
    
    get<field_1>(acr) = true;
    
    auto res = get<field_1>(cacr);
    CONSTEXPR_TEST(res == true );
    res = get<field_2>(acr);
    CONSTEXPR_TEST(res == false );
    res = get<field_3>(acr);
    CONSTEXPR_TEST(res == false );

    auto packed_value = pack_value<uint8_t>(acr);
    CONSTEXPR_TEST(packed_value == 0b001 );
    }
    {
    bool_bitfiled_struct acr;
    decltype(auto) cacr{ const_cast<bool_bitfiled_struct const &>(acr) };
    auto res = get<field_1>(cacr);
    CONSTEXPR_TEST(res == false );
    
    get<field_2>(acr) = true;
    
    CONSTEXPR_TEST(get<field_1>(acr) == false );
    CONSTEXPR_TEST(get<field_2>(acr) == true );
    CONSTEXPR_TEST(get<field_3>(acr) == false );

    auto packed_value = pack_value<uint8_t>(acr);
    CONSTEXPR_TEST(packed_value == 0b010 );
    }
  return true;
  }
  
BOOST_AUTO_TEST_CASE( test_metabitstruct_bool )
  {
  auto const_test = consteval_test_metabitstruct_bool();
  BOOST_TEST(const_test);
  
  using enum acs_fields;
  
  bool_bitfiled_struct acr;
  acr.get<field_1>() = true;
  get<field_1>(acr) = true;
  get<field_3>(acr) = true;
  
  auto res = get<field_1>(const_cast<bool_bitfiled_struct const &>(acr));
  BOOST_TEST(res == true );
  res = get<field_2>(acr);
  BOOST_TEST(res == false );
  
  constexpr auto fcount = filed_count<bool_bitfiled_struct>();
  static_assert(fcount == 3);
  auto packed_value = pack_value<uint8_t>(acr);
  BOOST_TEST(packed_value == 0b101 );
  }
  
enum struct mbs_fields 
  {
    field_1, field_2, field_3, field_4
  };
enum struct example_enum_value : uint8_t
  { value0 = 0, value1, value2, value3 };
  
using mixed_bitfiled_struct = 
  meta_packed_struct<
    member<uint8_t,mbs_fields::field_1,4>,
    member<bool,mbs_fields::field_2,1>,
    member<uint16_t,mbs_fields::field_3,16>,
    member<example_enum_value, mbs_fields::field_4,3>
    >;
    
bool consteval consteval_test_metabitstruct_mixed()
  {
  using enum mbs_fields;
  constexpr auto fcount = filed_count<mixed_bitfiled_struct>();
  CONSTEXPR_TEST(fcount == 4);
  constexpr auto s_bit_width = bit_width<mixed_bitfiled_struct>();
  CONSTEXPR_TEST(s_bit_width == 24);
    {
    mixed_bitfiled_struct mbs;
    get<field_1>(mbs) = 0b1111;
    CONSTEXPR_TEST(get<field_1>(mbs) == 0b1111 );
    CONSTEXPR_TEST(get<field_2>(mbs) == false );
    CONSTEXPR_TEST(get<field_3>(mbs) == 0 );
    CONSTEXPR_TEST(get<field_4>(mbs) == example_enum_value{} );
    
    auto packed_value = pack_value<uint32_t>(mbs);
    CONSTEXPR_TEST(packed_value == 0b1111 );
    }
    {
    mixed_bitfiled_struct mbs;
    get<field_2>(mbs) = true;
    CONSTEXPR_TEST(get<field_1>(mbs) == 0 );
    CONSTEXPR_TEST(get<field_2>(mbs) == true );
    CONSTEXPR_TEST(get<field_3>(mbs) == 0 );
    CONSTEXPR_TEST(get<field_4>(mbs) == example_enum_value{} );
    
    auto packed_value = pack_value<uint32_t>(mbs);
    CONSTEXPR_TEST(packed_value == 0b10000 );
    }
    {
    mixed_bitfiled_struct mbs;
    get<field_3>(mbs) = 0xffff;
    CONSTEXPR_TEST(get<field_1>(mbs) == 0 );
    CONSTEXPR_TEST(get<field_2>(mbs) == false );
    CONSTEXPR_TEST(get<field_3>(mbs) == 0xffff );
    CONSTEXPR_TEST(get<field_4>(mbs) == example_enum_value{} );
    
    auto packed_value = pack_value<uint32_t>(mbs);
    CONSTEXPR_TEST(packed_value == 0b00'1111111111111111'0'0000 );
    }
    {
    using enum example_enum_value;
    mixed_bitfiled_struct mbs;
    get<field_2>(mbs) = true;
    get<field_3>(mbs) = 0x0ff0;
    get<field_4>(mbs) = value2;
    CONSTEXPR_TEST(get<field_1>(mbs) == 0 );
    CONSTEXPR_TEST(get<field_2>(mbs) == true );
    CONSTEXPR_TEST(get<field_3>(mbs) == 0x0ff0 );
    CONSTEXPR_TEST(get<field_4>(mbs) == value2 );
    
    auto packed_value = pack_value<uint32_t>(mbs);
    CONSTEXPR_TEST(packed_value == 0b10'0000111111110000'1'0000 );
    }
  return true;
  }
  
BOOST_AUTO_TEST_CASE( test_metabitstruct_mixed )
  {
  auto const_test = consteval_test_metabitstruct_mixed();
  BOOST_TEST(const_test);
  }
  
bool consteval consteval_test_metabitstruct_mixed_constrcution()
  {
  using enum mbs_fields;
  using enum example_enum_value;
  
  mixed_bitfiled_struct mbs
    {
    arg<field_1> = uint8_t{1u},
    arg<field_2> = true,
    arg<field_3> = uint16_t{0x0ff0u},
    arg<field_4> = value1
    };
  CONSTEXPR_TEST(get<field_1>(mbs) == 1 );
  CONSTEXPR_TEST(get<field_2>(mbs) == true );
  CONSTEXPR_TEST(get<field_3>(mbs) == 0x0ff0 );
  CONSTEXPR_TEST(get<field_4>(mbs) == value1 );
  return true;
  }
  
BOOST_AUTO_TEST_CASE( test_metabitstruct_mixed_construction )
  {

  auto const_test = consteval_test_metabitstruct_mixed();
  BOOST_TEST(const_test);
  }
}
