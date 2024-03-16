#include <small_vectors/utils/strong_type.h>
#include <small_vectors/stream/strong_type.h>
#include <small_vectors/formattable/strong_type.h>

#include <unit_test_core.h>
#include <boost_ut.h>

namespace ut = boost::ut;
using test_types = metatests::type_list<uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t>;
using namespace metatests;
using boost::ut::operator""_test;
using namespace ut::operators::terse;
using boost::ut::eq;
using boost::ut::neq;

namespace small_vectors::utils
  {
struct test_tag : public strong_type_default_traits
  {
  };

static_assert(small_vectors::concepts::stream_insertable<strong_type<int, test_tag>>);

//----------------------------------------------------------------------------------------------------------------------
namespace concepts
  {
  struct tag_with_hash
    {
    static constexpr bool enable_hash_specialization = true;
    };

  struct tag_with_hash_neg
    {
    static constexpr bool enable_hash_specialization = false;
    };

  struct tag_empty
    {
    };

  static_assert(tag_hash_specialization<tag_with_hash>);
  static_assert(!tag_hash_specialization<tag_with_hash_neg>);
  static_assert(!tag_hash_specialization<tag_empty>);

  struct tag_with_arithemtic
    {
    static constexpr bool enable_arithemtic = true;
    };

  struct tag_with_arithemtic_neg
    {
    static constexpr bool enable_arithemtic = false;
    };

  static_assert(tag_arithemtic<tag_with_arithemtic>);
  static_assert(!tag_arithemtic<tag_with_arithemtic_neg>);
  static_assert(!tag_arithemtic<tag_empty>);

  struct tag_with_comparison
    {
    static constexpr bool enable_comparison = true;
    };

  struct tag_with_comparison_neg
    {
    static constexpr bool enable_comparison = false;
    };

  static_assert(tag_comparison<tag_with_comparison>);
  static_assert(!tag_comparison<tag_with_comparison_neg>);
  static_assert(!tag_comparison<tag_empty>);

  struct tag_with_binary_operators
    {
    static constexpr bool enable_binary_operators = true;
    };

  struct tag_with_binary_operators_neg
    {
    static constexpr bool enable_binary_operators = false;
    };

  static_assert(tag_binary_operators<tag_with_binary_operators>);
  static_assert(!tag_binary_operators<tag_with_binary_operators_neg>);
  static_assert(!tag_binary_operators<tag_empty>);
  }  // namespace concepts

static ut::suite<"strong_type_formatter"> strong_type_formatter = []
{
  using namespace ut;
  using namespace std::string_view_literals;

  "Default format test"_test = []
  {
    small_vectors::utils::strong_type<int, small_vectors::utils::test_tag> strong_int{42};
    expect(eq(std::format("{}", strong_int), "42"sv));
  };

  "Custom format - width and padding test"_test = []
  {
    small_vectors::utils::strong_type<int, small_vectors::utils::test_tag> strong_int{7};
    expect(eq(std::format("{:04}", strong_int), "0007"sv));
  };

  "Negative number formatting test"_test = []
  {
    small_vectors::utils::strong_type<int, small_vectors::utils::test_tag> strong_int{-123};
    expect(eq(std::format("{}", strong_int), "-123"sv));
  };

  "Hexadecimal format test"_test = []
  {
    small_vectors::utils::strong_type<int, small_vectors::utils::test_tag> strong_int{255};
    expect(eq(std::format("{:x}", strong_int), "ff"sv));
  };

  // More tests can be added here following the same pattern
};

static void strong_type_suite()
  {
  test_result result;
  "test_strong_type_basic"_test = [&result]
  {
    auto fn_tmpl = []<typename value_type>(value_type const *) -> metatests::test_result
    {
      test_result tr;
      using test_type = strong_type<value_type, test_tag>;
      test_type tst{value_type{0x55}};
      tr |= constexpr_test(*tst == 0x55) | constexpr_test(tst.value() == 0x55);

      test_type tst2{tst};
      tr |= constexpr_test(*tst2 == 0x55);

      test_type tst3{value_type{0x15}};
      tr |= constexpr_test(*tst3 == 0x15);
      tst3 = tst2;
      tr |= constexpr_test(*tst3 == 0x55);

      test_type tst4{std::numeric_limits<value_type>::max()};
      tr |= constexpr_test(*tst4 == std::numeric_limits<value_type>::max());
      tst3 = std::move(tst4);
      tr |= constexpr_test(*tst3 == std::numeric_limits<value_type>::max());
      return tr;
    };
    result |= run_consteval_test<test_types>(fn_tmpl);
    result |= run_constexpr_test<test_types>(fn_tmpl);
  };

  //----------------------------------------------------------------------------------------------------------------------
  "test_strong_type_arithemtic"_test = [&result]
  {
    auto fn_tmpl = []<typename value_type>(value_type const *) -> metatests::test_result
    {
      using test_type = strong_type<value_type, test_tag>;

      test_type tst{value_type{0x55}};
      test_result tr = constexpr_test(*++tst == 0x56);
      tr |= constexpr_test(*tst++ == 0x56);
      tr |= constexpr_test(*tst == 0x57);

      tr |= constexpr_test(*--tst == 0x56);
      tr |= constexpr_test(*tst-- == 0x56);
      tr |= constexpr_test(*tst == 0x55);

      tr |= constexpr_test(*(tst + test_type(value_type{1})) == 0x56);
      tr |= constexpr_test(*(tst - test_type(value_type{1})) == 0x54);

      tst = test_type{value_type{13}};
      tr |= constexpr_test(*(tst * test_type(value_type{2})) == 26);

      tst = test_type{value_type{44}};
      tr |= constexpr_test(*(tst / test_type(value_type{2})) == 22);

      tst = test_type{value_type{22}};
      tr |= constexpr_test(*(tst % test_type(value_type(10))) == 2);

      tst = test_type{value_type{0x55}};
      tr |= constexpr_test(*(tst += test_type(value_type(1))) == 0x56);

      tst = test_type{value_type{0x55}};
      tr |= constexpr_test(*(tst -= test_type(value_type(1))) == 0x54);

      tst = test_type{value_type{13}};
      tr |= constexpr_test(*(tst *= test_type(value_type(2))) == 26);

      tst = test_type{value_type{44}};
      tr |= constexpr_test(*(tst /= test_type(value_type(2))) == 22);

      tst = test_type{value_type{22}};
      tr |= constexpr_test(*(tst %= test_type(value_type(10))) == 2);

      return tr;
    };
    result |= run_consteval_test<test_types>(fn_tmpl);
    result |= run_constexpr_test<test_types>(fn_tmpl);
  };

  //----------------------------------------------------------------------------------------------------------------------
  "test_strong_type_comparison"_test = [&result]
  {
    auto fn_tmpl = []<typename value_type>(value_type const *) -> metatests::test_result
    {
      using test_type = strong_type<value_type, test_tag>;

      test_type tst0{value_type{0x55}};
      test_type tst1{value_type{0x55}};
      test_type tst2{value_type{0x56}};

      test_result tr
        = constexpr_test(tst0 == tst0) | constexpr_test(tst0 == tst1) | constexpr_test(!(tst0 == tst2))

          | constexpr_test(!(tst0 != tst0)) | constexpr_test(!(tst0 != tst1)) | constexpr_test(tst0 != tst2)

          | constexpr_test((tst0 <=> tst0) == std::strong_ordering::equal) | constexpr_test(!(tst0 < tst0))
          | constexpr_test(!(tst0 < tst1)) | constexpr_test(tst0 < tst2)
          | constexpr_test((tst0 <=> tst2) == std::strong_ordering::less) | constexpr_test(!(tst2 < tst0))

          | constexpr_test(tst0 <= tst0) | constexpr_test(tst0 <= tst1) | constexpr_test(tst0 <= tst2)
          | constexpr_test(!(tst2 <= tst0))

          | constexpr_test(!(tst0 > tst0)) | constexpr_test(!(tst0 > tst1)) | constexpr_test(!(tst0 > tst2))
          | constexpr_test(tst2 > tst0) | constexpr_test((tst2 <=> tst0) == std::strong_ordering::greater)

          | constexpr_test(tst0 >= tst0) | constexpr_test(tst0 >= tst1) | constexpr_test(!(tst0 >= tst2))
          | constexpr_test(tst2 >= tst0)

          | constexpr_test((tst1 <=> tst2) == std::strong_ordering::less)
          | constexpr_test((tst1 <=> tst0) == std::strong_ordering::equal);

      return tr;
    };
    result |= run_consteval_test<test_types>(fn_tmpl);
    result |= run_constexpr_test<test_types>(fn_tmpl);
  };

  //----------------------------------------------------------------------------------------------------------------------
  "test_strong_strong_type_binary"_test = [&result]
  {
    auto fn_tmpl = []<typename value_type>(value_type const *) -> metatests::test_result
    {
      using test_type = strong_type<value_type, test_tag>;
      test_type tst0{value_type{0b01010101}};
      test_type tst1{value_type{0b00111100}};

      test_result tr = constexpr_test((tst0 ^ tst1) == test_type{value_type{0b01101001}})
                       | constexpr_test((tst0 | tst1) == test_type{value_type{0b01111101}})
                       | constexpr_test((tst0 & tst1) == test_type{value_type{0b00010100}})

                       | constexpr_test((tst0 >> 1u) == test_type{value_type{0b00101010}})
                       | constexpr_test((tst0 >> test_type{value_type{1}}) == test_type{value_type{0b00101010}});

      tst0 = test_type{value_type{0b00101010}};
      tr |= constexpr_test((tst0 << 1u) == test_type{value_type{0b01010100}})
            | constexpr_test((tst0 << test_type{value_type{1}}) == test_type{value_type{0b01010100}});

      tst0 = test_type{value_type{0b01010101}};
      tr |= constexpr_test((tst0 ^= tst1) == test_type{value_type{0b01101001}});

      tst0 = test_type{value_type{0b01010101}};
      tr |= constexpr_test((tst0 |= tst1) == test_type{value_type{0b01111101}});

      tst0 = test_type{value_type{0b01010101}};
      tr |= constexpr_test((tst0 &= tst1) == test_type{value_type{0b00010100}});

      tst0 = test_type{value_type{0b01010101}};
      tr |= constexpr_test((tst0 >>= test_type{value_type{1}}) == test_type{value_type{0b00101010}});

      tst0 = test_type{value_type{0b00101010}};
      tr |= constexpr_test((tst0 <<= test_type{value_type{1}}) == test_type{value_type{0b01010100}});

      return tr;
    };
    result |= run_consteval_test<test_types>(fn_tmpl);
    result |= run_constexpr_test<test_types>(fn_tmpl);
  };
  }

  }  // namespace small_vectors::utils

int main() { small_vectors::utils::strong_type_suite(); }
