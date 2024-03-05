#include <small_vectors/basic_fixed_string.h>
#include <small_vectors/algo/bwt.h>
#include <unit_test_core.h>
#include <iostream>

using metatests::constexpr_test;
using metatests::run_consteval_test;
using metatests::run_constexpr_test;

namespace ut = boost::ut;
using ut::operator""_test;
using namespace ut::operators::terse;
using metatests::test_result;

using value_type_list = metatests::type_list<char, char8_t, char16_t, char32_t, wchar_t>;
// using value_type_list = metatests::type_list<char>;
using small_vectors::cast_fixed_string;
using std::ranges::begin;

namespace encode_test
  {
static void do_test(test_result & result)
  {
  "encode"_test = [&]
  {
    auto fn_tmpl = []<typename char_type>(char_type const *) -> metatests::test_result
    {
      using view_type = std::basic_string_view<char_type>;
      std::array<char_type, 128> buffer;
        {
        auto constexpr text{cast_fixed_string<char_type>("banana")};
        auto constexpr expected{cast_fixed_string<char_type>("annb$aa")};
        auto outit{small_vectors::algo::bwt::encode<'$'>(text, begin(buffer))};
        view_type v{begin(buffer), outit};
        constexpr_test(v == expected);
        }
        {
        auto constexpr text{cast_fixed_string<char_type>("abracadabra")};
        auto constexpr expected{cast_fixed_string<char_type>("ard$rcaaaabb")};
        auto outit{small_vectors::algo::bwt::encode<'$'>(text, begin(buffer))};
        view_type v{begin(buffer), outit};
        constexpr_test(v == expected);
        }
      return {};
    };

    result |= run_consteval_test<value_type_list>(fn_tmpl);
    result |= run_constexpr_test<value_type_list>(fn_tmpl);
  };
  }
  }  // namespace encode_test

namespace decode_test
  {
static void do_test(test_result & result)
  {
  "encode"_test = [&]
  {
    auto fn_tmpl = []<typename char_type>(char_type const *) -> metatests::test_result
    {
      using view_type = std::basic_string_view<char_type>;
      std::array<char_type, 128> buffer;
        {
        auto constexpr text{cast_fixed_string<char_type>("annb$aa")};
        auto constexpr expected{cast_fixed_string<char_type>("banana")};
        auto outit{small_vectors::algo::bwt::decode<'$'>(text, begin(buffer))};
        view_type v{begin(buffer), outit};
        constexpr_test(v == expected);
        }
        {
        auto constexpr text{cast_fixed_string<char_type>("ard$rcaaaabb")};
        auto constexpr expected{cast_fixed_string<char_type>("abracadabra")};
        auto outit{small_vectors::algo::bwt::decode<'$'>(text, begin(buffer))};
        view_type v{begin(buffer), outit};
        constexpr_test(v == expected);
        }
      return {};
    };

    // result |= run_consteval_test<value_type_list>(fn_tmpl);
    result |= run_constexpr_test<value_type_list>(fn_tmpl);
  };
  }
  }  // namespace decode_test

int main()
  {
  test_result result;
  encode_test::do_test(result);
  decode_test::do_test(result);
  }
