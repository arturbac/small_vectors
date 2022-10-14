#include <utils/meta_packed_struct.h>
#include <unit_test_core.h>

using namespace utils;
using boost::ut::operator""_test;

enum struct acs_fields 
  {
    field_1, field_2, field_3
  };

template<acs_fields tag_value>
using acs_member = member<bool,tag_value, 1>;
using metatests::constexpr_test;
using bool_bitfiled_struct = 
  meta_packed_struct<
    acs_member<acs_fields::field_1>,
    acs_member<acs_fields::field_2>,
    acs_member<acs_fields::field_3>
    >;

int main()
{
metatests::test_result result;
"test_metabitstruct_bool"_test = [&result]
  {
  auto fn_test = []()
    {
    metatests::test_result tr;
    using enum acs_fields;
    constexpr auto fcount = filed_count<bool_bitfiled_struct>();
    tr |= constexpr_test(fcount == 3);
    constexpr auto s_bit_width = bit_width<bool_bitfiled_struct>();
    tr |= constexpr_test(s_bit_width == 3);
      {
      bool_bitfiled_struct acr;
      decltype(auto) cacr{ const_cast<bool_bitfiled_struct const &>(acr) };

      get<field_1>(acr) = true;
      get<field_3>(acr) = true;

      auto res = get<field_1>(cacr);
      tr |=constexpr_test(res == true );
      res = get<field_2>(acr);
      tr |=constexpr_test(res == false );

      auto packed_value = pack_value<uint8_t>(acr);
      tr |=constexpr_test(packed_value == 0b101 );
      }

      {
      bool_bitfiled_struct acr;
      decltype(auto) cacr{ const_cast<bool_bitfiled_struct const &>(acr) };

      get<field_1>(acr) = true;

      auto res = get<field_1>(cacr);
      tr |=constexpr_test(res == true );
      res = get<field_2>(acr);
      tr |=constexpr_test(res == false );
      res = get<field_3>(acr);
      tr |=constexpr_test(res == false );

      auto packed_value = pack_value<uint8_t>(acr);
      tr |=constexpr_test(packed_value == 0b001 );
      }
      {
      bool_bitfiled_struct acr;
      decltype(auto) cacr{ const_cast<bool_bitfiled_struct const &>(acr) };
      auto res = get<field_1>(cacr);
      tr |=constexpr_test(res == false );

      get<field_2>(acr) = true;

      tr |=constexpr_test(get<field_1>(acr) == false );
      tr |=constexpr_test(get<field_2>(acr) == true );
      tr |=constexpr_test(get<field_3>(acr) == false );

      auto packed_value = pack_value<uint8_t>(acr);
      tr |=constexpr_test(packed_value == 0b010 );
      }
    return tr;
    };
  result |= metatests::run_constexpr_test(fn_test);
  result |= metatests::run_consteval_test(fn_test);
  };

  
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
    member<uint32_t ,mbs_fields::field_3,32>,
    member<example_enum_value, mbs_fields::field_4,3>
    >;
    
"test_metabitstruct_mixed"_test = [&result]
  {
  auto fn_test = []()
    {
    metatests::test_result tr;
    using enum mbs_fields;
    constexpr auto fcount = filed_count<mixed_bitfiled_struct>();
    tr |= constexpr_test(fcount == 4);
    constexpr auto s_bit_width = bit_width<mixed_bitfiled_struct>();
    tr |= constexpr_test(s_bit_width == 40);
      {
      mixed_bitfiled_struct mbs;
      get<field_1>(mbs) = 0b1111;
      tr |= constexpr_test(get<field_1>(mbs) == 0b1111 );
      tr |= constexpr_test(get<field_2>(mbs) == false );
      tr |= constexpr_test(get<field_3>(mbs) == 0 );
      tr |= constexpr_test(get<field_4>(mbs) == example_enum_value{} );

      auto packed_value = pack_value<uint64_t>(mbs);
      tr |= constexpr_test(packed_value == 0b1111 );
      }
      {
      mixed_bitfiled_struct mbs;
      get<field_2>(mbs) = true;
      tr |= constexpr_test(get<field_1>(mbs) == 0 );
      tr |= constexpr_test(get<field_2>(mbs) == true );
      tr |= constexpr_test(get<field_3>(mbs) == 0 );
      tr |= constexpr_test(get<field_4>(mbs) == example_enum_value{} );

      auto packed_value = pack_value<uint64_t>(mbs);
      tr |= constexpr_test(packed_value == 0b10000 );
      }
      {
      mixed_bitfiled_struct mbs;
      get<field_3>(mbs) = 0xffff;
      tr |= constexpr_test(get<field_1>(mbs) == 0 );
      tr |= constexpr_test(get<field_2>(mbs) == false );
      tr |= constexpr_test(get<field_3>(mbs) == 0xffff );
      tr |= constexpr_test(get<field_4>(mbs) == example_enum_value{} );

      auto packed_value = pack_value<uint64_t>(mbs);
      tr |= constexpr_test(packed_value == 0b00'1111111111111111'0'0000 );
      }
      {
      using enum example_enum_value;
      mixed_bitfiled_struct mbs;
      get<field_2>(mbs) = true;
      get<field_3>(mbs) = 0x0ff0;
      get<field_4>(mbs) = value2;
      tr |= constexpr_test(get<field_1>(mbs) == 0 );
      tr |= constexpr_test(get<field_2>(mbs) == true );
      tr |= constexpr_test(get<field_3>(mbs) == 0x0ff0 );
      tr |= constexpr_test(get<field_4>(mbs) == value2 );

      auto packed_value = pack_value<uint64_t>(mbs);
      tr |= constexpr_test(packed_value == 0b10'00000000000000000000111111110000'1'0000 );
      }
    return tr;
    };
  result |= metatests::run_constexpr_test(fn_test);
  result |= metatests::run_consteval_test(fn_test);
  };

"test_metabitstruct_mixed_constrcution"_test = [&result]
  {
  auto fn_test = []()
    {
    metatests::test_result tr;
    using enum mbs_fields;
    using enum example_enum_value;

    mixed_bitfiled_struct mbs
      {
      arg<field_1> = uint8_t{1u},
      arg<field_2> = true,
      arg<field_3> = uint16_t{0x0ff0u},
      arg<field_4> = value1
      };
    tr |= constexpr_test(get<field_1>(mbs) == 1 );
    tr |= constexpr_test(get<field_2>(mbs) == true );
    tr |= constexpr_test(get<field_3>(mbs) == 0x0ff0 );
    tr |= constexpr_test(get<field_4>(mbs) == value1 );
    return true;
    };
  result |= metatests::run_constexpr_test(fn_test);
  result |= metatests::run_consteval_test(fn_test);
  };
return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
