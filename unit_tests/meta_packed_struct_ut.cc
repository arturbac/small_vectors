#include <utils/meta_packed_struct.h>
#include <unit_test_core.h>

using namespace utils;
using boost::ut::operator""_test;

using utils::detail::compress_value;
using utils::detail::uncompress_value;

static_assert(static_cast<uint8_t>(~bitmask_v<uint8_t, 8>) == 0u);

static_assert(static_cast<int8_t>((~uint8_t(74)) + 1) == -74);
static_assert(static_cast<int8_t>((~uint8_t(-74)) + 1) == 74);

static_assert(compress_value<4>(int8_t(-7)) == 0b1001u);
static_assert(uncompress_value<4, int8_t>(0b1001u) == -7);

static_assert(compress_value<8>(int8_t(-1)) == 0xffu);
static_assert(uncompress_value<8, int8_t>(0xffu) == -1);
static_assert(compress_value<8>(int64_t(-1)) == 0xffu);
static_assert(uncompress_value<8, int64_t>(0xffu) == -1);

static_assert(compress_value<8>(int8_t(-65)) == 0xbfu);
static_assert(uncompress_value<8, int8_t>(0xbfu) == -65);

static_assert(compress_value<8>(int8_t(65)) == 0x41u);
static_assert(uncompress_value<8, int8_t>(0x41u) == 65);

static_assert(compress_value<8>(int8_t(-126)) == 0x82u);
static_assert(uncompress_value<8, int8_t>(0x82u) == -126);

static_assert(compress_value<8>(int8_t(-127)) == 0x81u);
static_assert(uncompress_value<8, int8_t>(0x81u) == -127);

static_assert(compress_value<8>(int8_t(127)) == 0x7fu);
static_assert(uncompress_value<8, int8_t>(0x7fu) == 127);

static_assert(compress_value<8>(int8_t(-128)) == 0x80u);
static_assert(uncompress_value<8, int8_t>(0x80u) == -128);
static_assert(compress_value<8>(int64_t(-128)) == 0x80u);
static_assert(uncompress_value<8, int64_t>(0x80u) == -128);

static_assert(compress_value<7>(int8_t(-1)) == 0x7fu);
static_assert(uncompress_value<7, int8_t>(0x7fu) == -1);

static_assert(compress_value<7>(int8_t(1)) == 0x1u);
static_assert(uncompress_value<7, int8_t>(0x1u) == 1);

static_assert(compress_value<7>(int8_t(32)) == 32u);
static_assert(uncompress_value<7, int8_t>(32u) == 32);

static_assert(compress_value<7>(int8_t(-32)) == 0b1100000u);
static_assert(uncompress_value<7, int8_t>(0b1100000u) == -32);

static_assert(compress_value<5>(int8_t(15)) == 15u);
static_assert(uncompress_value<5, int8_t>(15u) == 15);

static_assert(compress_value<5>(int8_t(-15)) == 0b10001u);
static_assert(uncompress_value<5, int8_t>(0b10001u) == -15);

static_assert(compress_value<2>(int8_t(-1)) == 0b11u);
static_assert(uncompress_value<2, int8_t>(0b11u) == -1);

static_assert(compress_value<3>(int8_t(-2)) == 0b110u);
static_assert(uncompress_value<3, int8_t>(0b110u) == -2);

static_assert(compress_value<3>(int8_t(2)) == 2u);
static_assert(uncompress_value<3, int8_t>(2u) == 2);

static_assert(compress_value<3>(int8_t(0)) == 0u);
static_assert(uncompress_value<3, int8_t>(0u) == 0);

static_assert(compress_value<32>(0x7fffffff) == 0x7fffffffu);
static_assert(uncompress_value<32, int32_t>(0x7fffffffu) == 0x7fffffff);

template<typename T>
using nl = std::numeric_limits<T>;

static_assert(compress_value<8>(nl<int8_t>::max()) == unsigned(nl<int8_t>::max()));
static_assert(compress_value<16>(nl<int16_t>::max()) == unsigned(nl<int16_t>::max()));
static_assert(compress_value<32>(nl<int32_t>::max()) == unsigned(nl<int32_t>::max()));
static_assert(compress_value<64>(nl<int64_t>::max()) == uint64_t(nl<int64_t>::max()));

static_assert(compress_value<8>(nl<int8_t>::min()) == (1u << 7));
static_assert(compress_value<16>(nl<int16_t>::min()) == (1u << 15));

enum struct acs_fields
  {
  field_1,
  field_2,
  field_3
  };

template<acs_fields tag_value>
using acs_member = member<bool, tag_value, 1>;
using metatests::constexpr_test;
using bool_bitfiled_struct = meta_packed_struct<
  acs_member<acs_fields::field_1>,
  acs_member<acs_fields::field_2>,
  acs_member<acs_fields::field_3>>;

int main()
  {
  metatests::test_result result;

  "test_bitmask"_test = [&result]
  {
    auto fn_test = []()
    {
      metatests::test_result tr;
        {
        tr |= constexpr_test(utils::bitmask_v<uint8_t, 0> == 0x0u);
        tr |= constexpr_test(utils::bitmask_v<uint8_t, 8> == 0xFFu);
        tr |= constexpr_test(utils::bitmask_v<uint8_t, 7> == 0x7Fu);
        tr |= constexpr_test(utils::bitmask_v<uint8_t, 1> == 0x1u);

        tr |= constexpr_test(utils::bitmask_v<uint16_t, 7> == 0x7Fu);
        tr |= constexpr_test(utils::bitmask_v<uint16_t, 1> == 0x1u);
        tr |= constexpr_test(utils::bitmask_v<uint16_t, 8> == 0xFFu);
        tr |= constexpr_test(utils::bitmask_v<uint16_t, 13> == (1u << 13) - 1);
        tr |= constexpr_test(utils::bitmask_v<uint16_t, 9> == (1u << 9) - 1);
        tr |= constexpr_test(utils::bitmask_v<uint16_t, 16> == 0xFFFFu);

        tr |= constexpr_test(utils::bitmask_v<uint32_t, 8> == 0xFFu);
        tr |= constexpr_test(utils::bitmask_v<uint32_t, 7> == 0x7Fu);
        tr |= constexpr_test(utils::bitmask_v<uint32_t, 1> == 0x1u);
        tr |= constexpr_test(utils::bitmask_v<uint32_t, 28> == (1u << 28) - 1);
        tr |= constexpr_test(utils::bitmask_v<uint32_t, 23> == (1u << 23) - 1);
        tr |= constexpr_test(utils::bitmask_v<uint32_t, 29> == (1u << 29) - 1);
        tr |= constexpr_test(utils::bitmask_v<uint32_t, 32> == 0xFFFFFFFFu);

        tr |= constexpr_test(utils::bitmask_v<uint64_t, 0> == 0x0u);
        tr |= constexpr_test(utils::bitmask_v<uint64_t, 8> == 0xFFu);
        tr |= constexpr_test(utils::bitmask_v<uint64_t, 7> == 0x7Fu);
        tr |= constexpr_test(utils::bitmask_v<uint64_t, 1> == 0x1u);
        tr |= constexpr_test(utils::bitmask_v<uint64_t, 28> == (1u << 28) - 1);
        tr |= constexpr_test(utils::bitmask_v<uint64_t, 23> == (1u << 23) - 1);
        tr |= constexpr_test(utils::bitmask_v<uint64_t, 29> == (1u << 29) - 1);
        tr |= constexpr_test(utils::bitmask_v<uint64_t, 32> == 0xFFFFFFFFu);
        tr |= constexpr_test(utils::bitmask_v<uint64_t, 58> == (1LLu << 58) - 1);
        tr |= constexpr_test(utils::bitmask_v<uint64_t, 53> == (1LLu << 53) - 1);
        tr |= constexpr_test(utils::bitmask_v<uint64_t, 59> == (1LLu << 59) - 1);
        tr |= constexpr_test(utils::bitmask_v<uint64_t, 64> == 0xFFFFFFFFFFFFFFFFLLu);
        }

      return tr;
    };
    result |= metatests::run_constexpr_test(fn_test);
    result |= metatests::run_consteval_test(fn_test);
  };

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
        decltype(auto) cacr{const_cast<bool_bitfiled_struct const &>(acr)};

        get<field_1>(acr) = true;
        get<field_3>(acr) = true;

        auto res = get<field_1>(cacr);
        tr |= constexpr_test(res == true);
        res = get<field_2>(acr);
        tr |= constexpr_test(res == false);

        auto packed_value = pack_value<uint8_t>(acr);
        tr |= constexpr_test(packed_value == 0b101);
        }

        {
        bool_bitfiled_struct acr;
        decltype(auto) cacr{const_cast<bool_bitfiled_struct const &>(acr)};

        get<field_1>(acr) = true;

        auto res = get<field_1>(cacr);
        tr |= constexpr_test(res == true);
        res = get<field_2>(acr);
        tr |= constexpr_test(res == false);
        res = get<field_3>(acr);
        tr |= constexpr_test(res == false);

        auto packed_value = pack_value<uint8_t>(acr);
        tr |= constexpr_test(packed_value == 0b001);
        }
        {
        bool_bitfiled_struct acr;
        decltype(auto) cacr{const_cast<bool_bitfiled_struct const &>(acr)};
        auto res = get<field_1>(cacr);
        tr |= constexpr_test(res == false);

        get<field_2>(acr) = true;

        tr |= constexpr_test(get<field_1>(acr) == false);
        tr |= constexpr_test(get<field_2>(acr) == true);
        tr |= constexpr_test(get<field_3>(acr) == false);

        auto packed_value = pack_value<uint8_t>(acr);
        tr |= constexpr_test(packed_value == 0b010);
        }
      return tr;
    };
    result |= metatests::run_constexpr_test(fn_test);
    result |= metatests::run_consteval_test(fn_test);
  };

  enum struct mbs_fields
    {
    field_1,
    field_2,
    field_3,
    field_4
    };
  enum struct example_enum_value : uint8_t
    {
    value0 = 0,
    value1,
    value2,
    value3
    };

  using mixed_bitfiled_struct = meta_packed_struct<
    member<uint8_t, mbs_fields::field_1, 4>,
    member<bool, mbs_fields::field_2, 1>,
    member<uint32_t, mbs_fields::field_3, 32>,
    member<example_enum_value, mbs_fields::field_4, 3>>;

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
        tr |= constexpr_test(get<field_1>(mbs) == 0b1111);
        tr |= constexpr_test(get<field_2>(mbs) == false);
        tr |= constexpr_test(get<field_3>(mbs) == 0);
        tr |= constexpr_test(get<field_4>(mbs) == example_enum_value{});

        auto packed_value = pack_value<uint64_t>(mbs);
        tr |= constexpr_test(packed_value == 0b1111);
        }
        {
        mixed_bitfiled_struct mbs;
        get<field_2>(mbs) = true;
        tr |= constexpr_test(get<field_1>(mbs) == 0);
        tr |= constexpr_test(get<field_2>(mbs) == true);
        tr |= constexpr_test(get<field_3>(mbs) == 0);
        tr |= constexpr_test(get<field_4>(mbs) == example_enum_value{});

        auto packed_value = pack_value<uint64_t>(mbs);
        tr |= constexpr_test(packed_value == 0b10000);
        }
        {
        mixed_bitfiled_struct mbs;
        get<field_3>(mbs) = 0xffff;
        tr |= constexpr_test(get<field_1>(mbs) == 0);
        tr |= constexpr_test(get<field_2>(mbs) == false);
        tr |= constexpr_test(get<field_3>(mbs) == 0xffff);
        tr |= constexpr_test(get<field_4>(mbs) == example_enum_value{});

        auto packed_value = pack_value<uint64_t>(mbs);
        tr |= constexpr_test(packed_value == 0b00'1111111111111111'0'0000);
        }
        {
        using enum example_enum_value;
        mixed_bitfiled_struct mbs;
        get<field_2>(mbs) = true;
        get<field_3>(mbs) = 0x0ff0;
        get<field_4>(mbs) = value2;
        tr |= constexpr_test(get<field_1>(mbs) == 0);
        tr |= constexpr_test(get<field_2>(mbs) == true);
        tr |= constexpr_test(get<field_3>(mbs) == 0x0ff0);
        tr |= constexpr_test(get<field_4>(mbs) == value2);

        auto packed_value = pack_value<uint64_t>(mbs);
        tr |= constexpr_test(packed_value == 0b10'00000000000000000000111111110000'1'0000);
        }
      return tr;
    };
    result |= metatests::run_constexpr_test(fn_test);
    result |= metatests::run_consteval_test(fn_test);
  };

  using mixed_bitfiled_struct2 = meta_packed_struct<
    member<uint8_t, mbs_fields::field_1, 4>,
    member<bool, mbs_fields::field_2, 1>,
    member<uint16_t, mbs_fields::field_3, 16>,
    member<example_enum_value, mbs_fields::field_4, 3>>;
  "test_metabitstruct_mixed2"_test = [&result]
  {
    auto fn_test = []()
    {
      metatests::test_result tr;
      using enum mbs_fields;
      constexpr auto fcount = filed_count<mixed_bitfiled_struct2>();
      tr |= constexpr_test(fcount == 4);
      constexpr auto s_bit_width = bit_width<mixed_bitfiled_struct2>();
      tr |= constexpr_test(s_bit_width == 24);
        {
        mixed_bitfiled_struct2 mbs;
        get<field_1>(mbs) = 0b1111;
        tr |= constexpr_test(get<field_1>(mbs) == 0b1111);
        tr |= constexpr_test(get<field_2>(mbs) == false);
        tr |= constexpr_test(get<field_3>(mbs) == 0);
        tr |= constexpr_test(get<field_4>(mbs) == example_enum_value{});

        auto packed_value = pack_value<uint32_t>(mbs);
        tr |= constexpr_test(packed_value == 0b1111);
        }
        {
        mixed_bitfiled_struct2 mbs;
        get<field_2>(mbs) = true;
        tr |= constexpr_test(get<field_1>(mbs) == 0);
        tr |= constexpr_test(get<field_2>(mbs) == true);
        tr |= constexpr_test(get<field_3>(mbs) == 0);
        tr |= constexpr_test(get<field_4>(mbs) == example_enum_value{});

        auto packed_value = pack_value<uint32_t>(mbs);
        tr |= constexpr_test(packed_value == 0b10000);
        }
        {
        mixed_bitfiled_struct2 mbs;
        get<field_3>(mbs) = 0xffff;
        tr |= constexpr_test(get<field_1>(mbs) == 0);
        tr |= constexpr_test(get<field_2>(mbs) == false);
        tr |= constexpr_test(get<field_3>(mbs) == 0xffff);
        tr |= constexpr_test(get<field_4>(mbs) == example_enum_value{});

        auto packed_value = pack_value<uint32_t>(mbs);
        tr |= constexpr_test(packed_value == 0b00'1111111111111111'0'0000);
        }
        {
        using enum example_enum_value;
        mixed_bitfiled_struct2 mbs;
        get<field_2>(mbs) = true;
        get<field_3>(mbs) = 0x0ff0;
        get<field_4>(mbs) = value2;
        tr |= constexpr_test(get<field_1>(mbs) == 0);
        tr |= constexpr_test(get<field_2>(mbs) == true);
        tr |= constexpr_test(get<field_3>(mbs) == 0x0ff0);
        tr |= constexpr_test(get<field_4>(mbs) == value2);

        auto packed_value = pack_value<uint32_t>(mbs);
        tr |= constexpr_test(packed_value == 0b10'0000111111110000'1'0000);
        }
      return tr;
    };
    result |= metatests::run_constexpr_test(fn_test);
    result |= metatests::run_consteval_test(fn_test);
  };

  using mixed_bitfiled_struct3 = meta_packed_struct<
    member<uint8_t, mbs_fields::field_1, 4>,
    member<bool, mbs_fields::field_2, 1>,
    member<uint64_t, mbs_fields::field_3, 56>,
    member<example_enum_value, mbs_fields::field_4, 3>>;
  "test_metabitstruct_mixed3"_test = [&result]
  {
    auto fn_test = []()
    {
      metatests::test_result tr;
      using enum mbs_fields;
      constexpr auto fcount = filed_count<mixed_bitfiled_struct3>();
      tr |= constexpr_test(fcount == 4);
      constexpr auto s_bit_width = bit_width<mixed_bitfiled_struct3>();
      tr |= constexpr_test(s_bit_width == 64);
        {
        mixed_bitfiled_struct3 mbs;
        get<field_1>(mbs) = 0b1111;
        tr |= constexpr_test(get<field_1>(mbs) == 0b1111);
        tr |= constexpr_test(get<field_2>(mbs) == false);
        tr |= constexpr_test(get<field_3>(mbs) == 0);
        tr |= constexpr_test(get<field_4>(mbs) == example_enum_value{});

        auto packed_value = pack_value<uint64_t>(mbs);
        tr |= constexpr_test(packed_value == 0b1111);
        }
        {
        mixed_bitfiled_struct3 mbs;
        get<field_2>(mbs) = true;
        tr |= constexpr_test(get<field_1>(mbs) == 0);
        tr |= constexpr_test(get<field_2>(mbs) == true);
        tr |= constexpr_test(get<field_3>(mbs) == 0);
        tr |= constexpr_test(get<field_4>(mbs) == example_enum_value{});

        auto packed_value = pack_value<uint64_t>(mbs);
        tr |= constexpr_test(packed_value == 0b10000);
        }
        {
        mixed_bitfiled_struct3 mbs;
        get<field_3>(mbs) = (0x1llu << 56) - 1;
        tr |= constexpr_test(get<field_1>(mbs) == 0);
        tr |= constexpr_test(get<field_2>(mbs) == false);
        tr |= constexpr_test(get<field_3>(mbs) == (0x1llu << 56) - 1);
        tr |= constexpr_test(get<field_4>(mbs) == example_enum_value{});

        auto packed_value = pack_value<uint64_t>(mbs);
        tr |= constexpr_test(packed_value == 0b00'11111111111111111111111111111111111111111111111111111111'0'0000);
        }
        {
        using enum example_enum_value;
        mixed_bitfiled_struct3 mbs;
        get<field_2>(mbs) = true;
        get<field_3>(mbs) = 0b01111111111111111111111111111111111111111111111111111110;
        get<field_4>(mbs) = value2;
        tr |= constexpr_test(get<field_1>(mbs) == 0);
        tr |= constexpr_test(get<field_2>(mbs) == true);
        tr |= constexpr_test(get<field_3>(mbs) == 0b01111111111111111111111111111111111111111111111111111110);
        tr |= constexpr_test(get<field_4>(mbs) == value2);

        auto packed_value = pack_value<uint64_t>(mbs);
        tr |= constexpr_test(packed_value == 0b10'01111111111111111111111111111111111111111111111111111110'1'0000);
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

      mixed_bitfiled_struct mbs{
        arg<field_1> = uint8_t{1u}, arg<field_2> = true, arg<field_3> = uint16_t{0x0ff0u}, arg<field_4> = value1
      };
      tr |= constexpr_test(get<field_1>(mbs) == 1);
      tr |= constexpr_test(get<field_2>(mbs) == true);
      tr |= constexpr_test(get<field_3>(mbs) == 0x0ff0);
      tr |= constexpr_test(get<field_4>(mbs) == value1);
      return tr;
    };
    result |= metatests::run_constexpr_test(fn_test);
    result |= metatests::run_consteval_test(fn_test);
  };

  "test_metabitstruct2_mixed_unpack"_test = [&result]
  {
    auto fn_test = []()
    {
      metatests::test_result tr;
      using enum mbs_fields;
      using enum example_enum_value;

      uint32_t packed_value{0b011000011111111000010010};
      auto mbs{unpack_value<mixed_bitfiled_struct2>(packed_value)};

      tr |= constexpr_test(get<field_1>(mbs) == 0x02);
      tr |= constexpr_test(get<field_2>(mbs) == true);
      tr |= constexpr_test(get<field_3>(mbs) == 0x0ff0);
      tr |= constexpr_test(get<field_4>(mbs) == value3);

      return true;
    };
    result |= metatests::run_consteval_test(fn_test);
    result |= metatests::run_constexpr_test(fn_test);
  };

  "test_metabitstruct_mixed_unpack"_test = [&result]
  {
    auto fn_test = []()
    {
      metatests::test_result tr;
      using enum mbs_fields;
      using enum example_enum_value;

      mixed_bitfiled_struct mbs;
      get<field_1>(mbs) = 0b1011;
      get<field_2>(mbs) = false;
      get<field_3>(mbs) = 0b10101010101010101010101010101010;
      get<field_4>(mbs) = value1;

      auto packed_value{pack_value<uint64_t>(mbs)};

      auto mbs_unpacked{unpack_value<decltype(mbs)>(packed_value)};
      tr |= constexpr_test(get<field_1>(mbs) == get<field_1>(mbs_unpacked));
      tr |= constexpr_test(get<field_2>(mbs) == get<field_2>(mbs_unpacked));
      tr |= constexpr_test(get<field_3>(mbs) == get<field_3>(mbs_unpacked));
      tr |= constexpr_test(get<field_4>(mbs) == get<field_4>(mbs_unpacked));

      return true;
    };
    result |= metatests::run_consteval_test(fn_test);
    result |= metatests::run_constexpr_test(fn_test);
  };

  using mixed_signed_struct = meta_packed_struct<
    member<int8_t, mbs_fields::field_1, 4>,
    member<int64_t, mbs_fields::field_2, 20>,
    member<int32_t, mbs_fields::field_3, 24>,
    member<int16_t, mbs_fields::field_4, 16>>;

  "test_metabitstruct_mixed_signed_unpack"_test = [&result]
  {
    auto fn_test = []()
    {
      metatests::test_result tr;
      using enum mbs_fields;
      using enum example_enum_value;
        {
        mixed_signed_struct mbs;
        get<field_1>(mbs) = -7;
        get<field_2>(mbs) = -128;
        get<field_3>(mbs) = -128;
        get<field_4>(mbs) = -128;

        tr |= constexpr_test(get<field_1>(mbs) == -7);
        tr |= constexpr_test(get<field_3>(mbs) == -128);
        tr |= constexpr_test(get<field_3>(mbs) == -128);
        tr |= constexpr_test(get<field_4>(mbs) == -128);

        auto packed_value{pack_value<uint64_t>(mbs)};
        auto mbs_unpacked{unpack_value<decltype(mbs)>(packed_value)};

        tr |= constexpr_test(get<field_1>(mbs) == get<field_1>(mbs_unpacked));
        tr |= constexpr_test(get<field_2>(mbs) == get<field_2>(mbs_unpacked));
        tr |= constexpr_test(get<field_3>(mbs) == get<field_3>(mbs_unpacked));
        tr |= constexpr_test(get<field_4>(mbs) == get<field_4>(mbs_unpacked));
        }
        {
        mixed_signed_struct mbs;
        get<field_1>(mbs) = -8;
        get<field_2>(mbs) = -0x7FFFF;
        get<field_3>(mbs) = -0x7FFFFF;
        get<field_4>(mbs) = -32768;
        auto packed_value{pack_value<uint64_t>(mbs)};
        auto mbs_unpacked{unpack_value<decltype(mbs)>(packed_value)};
        tr |= constexpr_test(-8 == get<field_1>(mbs_unpacked));
        tr |= constexpr_test(-0x7FFFF == get<field_2>(mbs_unpacked));
        tr |= constexpr_test(-0x7FFFFF == get<field_3>(mbs_unpacked));
        tr |= constexpr_test(-32768 == get<field_4>(mbs_unpacked));
        }
        {
        mixed_signed_struct mbs;
        get<field_1>(mbs) = 7;
        get<field_2>(mbs) = 0x7FFFF;
        get<field_3>(mbs) = 0x7FFFFF;
        get<field_4>(mbs) = 32767;
        auto packed_value{pack_value<uint64_t>(mbs)};
        auto mbs_unpacked{unpack_value<decltype(mbs)>(packed_value)};
        tr |= constexpr_test(7 == get<field_1>(mbs_unpacked));
        tr |= constexpr_test(0x7FFFF == get<field_2>(mbs_unpacked));
        tr |= constexpr_test(0x7FFFFF == get<field_3>(mbs_unpacked));
        tr |= constexpr_test(32767 == get<field_4>(mbs_unpacked));
        }
      return true;
    };
    result |= metatests::run_consteval_test(fn_test);
    result |= metatests::run_constexpr_test(fn_test);
  };

  using mixed_small_struct = meta_packed_struct<
    member<uint8_t, mbs_fields::field_1, 5>,
    member<bool, mbs_fields::field_2, 1>,
    member<example_enum_value, mbs_fields::field_3, 2>>;

  "test_metabitstruct_mixed_signed_unpack"_test = [&result]
  {
    auto fn_test = []()
    {
      metatests::test_result tr;
      using enum mbs_fields;
      using enum example_enum_value;
        {
        mixed_small_struct mbs;
        get<field_1>(mbs) = 7;
        get<field_2>(mbs) = true;
        get<field_3>(mbs) = example_enum_value::value2;
        auto packed_value{pack_value<uint8_t>(mbs)};
        auto mbs_unpacked{unpack_value<mixed_small_struct>(packed_value)};
        tr |= constexpr_test(7 == get<field_1>(mbs_unpacked));
        tr |= constexpr_test(true == get<field_2>(mbs_unpacked));
        tr |= constexpr_test(example_enum_value::value2 == get<field_3>(mbs_unpacked));
        }
      return true;
    };
    result |= metatests::run_consteval_test(fn_test);
    result |= metatests::run_constexpr_test(fn_test);
  };
  return result ? EXIT_SUCCESS : EXIT_FAILURE;
  }

