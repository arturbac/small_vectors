#include <unit_test_core.h>
#include <coll/basic_fixed_string.h>
#include <coll/basic_string.h>

#include <iostream>

using s_string = coll::static_string<256>;
#define debug_only_static 0

using buffered_string_type_list
  = metatests::type_list<coll::string, coll::u8string, coll::u16string, coll::u32string, coll::wstring>;

#if !debug_only_static

using string_type_list = metatests::type_list<
  coll::string,
  coll::u8string,
  coll::u16string,
  coll::u32string,
  coll::wstring,
  coll::static_string<256>,
  coll::static_u8string<256>,
  coll::static_u16string<256>,
  coll::static_u32string<256>,
  coll::static_wstring<256>>;
#else
using string_type_list = metatests::type_list<
  // coll::static_string<256>
  coll::wstring>;

#endif
static_assert(std::same_as<uint8_t, coll::static_string<10>::size_type>);
static_assert(std::same_as<uint16_t, coll::static_string<256>::size_type>);

using metatests::constexpr_test;
using metatests::run_consteval_test;
using metatests::run_constexpr_test;
namespace ut = boost::ut;
using ut::operator""_test;
using namespace ut::operators::terse;
using metatests::test_result;

namespace coll
  {

consteval bool verify_basic_string()
  {
  metatests::test_result tr;
  using metatests::constexpr_test;

  using namespace std::string_view_literals;
    {
    basic_fixed_string s{"12"};
    tr |= constexpr_test(s == "12"sv);
    }
    {
    auto s{cast_fixed_string<wchar_t>("123")};
    tr |= constexpr_test(s == L"123"sv);
    }
    {
    auto s{concat_fixed_string(basic_fixed_string{"12"}, basic_fixed_string{"34"}, cast_fixed_string<char>("aBc"))};
    tr |= constexpr_test(s == "1234aBc"sv);
    }
    {
    auto s{basic_fixed_string{"12"} + "34" + cast_fixed_string<char>("aBc")};
    tr |= constexpr_test(s == "1234aBc"sv);
    }
    {
    auto s{"12" + basic_fixed_string{"34"} + cast_fixed_string<char>("aBc")};
    tr |= constexpr_test(s == "1234aBc"sv);
    }
    {
    using uchar = unsigned char;
    auto s{cast_fixed_string<uchar>("12")};
    constexpr uchar expected[]{uchar('1'), uchar('2'), uchar('\0')};
    tr |= constexpr_test(s == std::basic_string_view(&expected[0]));
    }
  return static_cast<bool>(tr);
  }

template<typename char_type, std::size_t N>
consteval auto test_basic_fixed_string_as_buffor(char_type const (&str)[N])
  {
  basic_fixed_string const src{str};
  basic_fixed_string<char_type, N * 2> result{};
  std::copy(src.begin(), src.end(), result.begin());
  result[0u] = 'T';
  result[1u] = 'e';
  result[2u] = 's';
  result[3u] = 't';
  // 4
  // 5
  result[6u] = '\0';
  return result;
  }

static constexpr auto test_buffer = test_basic_fixed_string_as_buffor("01234567890");
static constexpr auto test_buffer_view = test_buffer.null_terminated_buffor_view();
static_assert(test_buffer_view == std::string_view{"Test45"});

static_assert(verify_basic_string());
  }  // namespace coll

using namespace coll;

constexpr bool is_null_termianted(auto str) noexcept
  {
  if constexpr(coll::detail::string::null_terminate_string)
    return str[size(str)] == '\0';
  else
    return true;
  }

template<typename iterator>
constexpr auto all_elememnts_equal_to(iterator first, iterator last, std::iter_value_t<iterator> ch) noexcept
  {
  return last == std::find_if(first, last, [ch](auto l) noexcept { return l != ch; });
  }

int main()
  {
  test_result result;
  "basic_string_constr"_test = [&]
  {
    auto fn_tmpl = []<typename string_type>(string_type const *) -> metatests::test_result
    {
      using st = string_type;
      using size_type = typename string_type::size_type;
      using char_type = typename string_type::char_type;
      auto constexpr text_long{
        cast_fixed_string<char_type>("Lorem ipsum dolor sit amet, consectetur adipiscing elit."
                                     " Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.")
      };
      auto constexpr text_short{cast_fixed_string<char_type>("1234")};
      test_result tr;

      st v;
      tr |= constexpr_test(size(v) == 0u) | constexpr_test(empty(v)) | constexpr_test(begin(v) == end(v))
            | constexpr_test(capacity(v) > 0u) | constexpr_test(data(v) != nullptr);

      // const & , view
      st vs{text_short.view()};
      constexpr_test(vs == text_short.view());
      constexpr_test(size(vs) == text_short.size());
      constexpr_test(capacity(vs) >= text_short.size());
      constexpr_test(is_null_termianted(vs));

      st vl{text_long.view()};
      constexpr_test(vl == text_long.view());
      constexpr_test(size(vl) == text_long.size());
      constexpr_test(capacity(vl) >= text_long.size());
      constexpr_test(is_null_termianted(vl));
        {
        st v2{v};
        constexpr_test(empty(v2));
        constexpr_test(size(v2) == 0u);
        constexpr_test(capacity(v2) == st::buffered_capacity());
        constexpr_test(is_null_termianted(v2));
        }
        {
        st v2{vs};
        constexpr_test(v2 == text_short.view());
        constexpr_test(size(v2) == text_short.size());
        constexpr_test(capacity(v2) >= text_short.size());
        constexpr_test(is_null_termianted(v2));
        }
        {
        st v2{vl};
        constexpr_test(v2 == text_long.view());
        constexpr_test(size(v2) == text_long.size());
        constexpr_test(capacity(v2) >= text_long.size());
        constexpr_test(is_null_termianted(v2));
        }
        // &&
        {
        st v2{std::move(vs)};
        constexpr_test(v2 == text_short.view());
        constexpr_test(size(v2) == text_short.size());
        constexpr_test(capacity(v2) >= text_short.size());
        constexpr_test(is_null_termianted(v2));

        constexpr_test(empty(vs));
        constexpr_test(size(vs) == 0u);
        constexpr_test(capacity(vs) == st::buffered_capacity());
        constexpr_test(is_null_termianted(vs));
        }
        {
        st v2{std::move(vl)};
        constexpr_test(v2 == text_long.view());
        constexpr_test(size(v2) == text_long.size());
        constexpr_test(capacity(v2) >= text_long.size());
        constexpr_test(is_null_termianted(v2));

        constexpr_test(empty(vl));
        constexpr_test(size(vl) == 0u);
        constexpr_test(capacity(vl) == st::buffered_capacity());
        constexpr_test(is_null_termianted(vl));
        }
        // size
        {
        st v1{size_type(0)};
        constexpr_test(empty(v1));
        constexpr_test(size(v1) == 0u);
        constexpr_test(capacity(v1) == st::buffered_capacity());
        constexpr_test(is_null_termianted(v1));
        }
        {
        st v1{size_type(4)};
        constexpr_test(!empty(v1));
        constexpr_test(size(v1) == 4u);
        constexpr_test(capacity(v1) == st::buffered_capacity());
        auto constexpr default_txt{cast_fixed_string<char_type>("\0\0\0\0")};
        constexpr_test(v1 == default_txt.view());
        constexpr_test(is_null_termianted(v1));
        }
        {
        st v1{size_type(121)};
        constexpr_test(!empty(v1));
        constexpr_test(size(v1) == 121u);
        constexpr_test(all_elememnts_equal_to(begin(v1), end(v1), '\0'));
        constexpr_test(capacity(v1) >= 121);
        constexpr_test(is_null_termianted(v1));
        }
        {
        st v1{size_type(121), char_type('a')};
        constexpr_test(all_elememnts_equal_to(begin(v1), end(v1), 'a'));
        constexpr_test(!empty(v1));
        constexpr_test(size(v1) == 121u);
        constexpr_test(capacity(v1) >= 121);
        constexpr_test(is_null_termianted(v1));
        }
      return tr;
    };

    result |= run_consteval_test<string_type_list>(fn_tmpl);
    result |= run_constexpr_test<string_type_list>(fn_tmpl);
  };

  "basic_string_assign"_test = [&]
  {
    auto fn_tmpl = []<typename string_type>(string_type const *) -> metatests::test_result
    {
      using st = string_type;
      using char_type = typename string_type::char_type;
      using size_type = typename string_type::size_type;
      using view_type = typename string_type::view_type;
      auto constexpr text_long{
        cast_fixed_string<char_type>("Lorem ipsum dolor sit amet, consectetur adipiscing elit."
                                     " Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.")
      };
      auto constexpr text_short{cast_fixed_string<char_type>("1234")};

      st vs{text_short.view()};
      st vl{text_long.view()};
      auto sub_view{text_long.view().substr(7u)};
      st vx{sub_view};
      vl = vx;
      constexpr_test(vl == sub_view);
      constexpr_test(size(vl) == sub_view.size());
      constexpr_test(capacity(vl) >= sub_view.size());
      constexpr_test(is_null_termianted(vl));

      vl = vs;
      constexpr_test(vl == text_short.view());
      constexpr_test(capacity(vl) == vl.buffered_capacity());
      constexpr_test(is_null_termianted(vl));

      // implicit assignment from exactly view type
      vl = sub_view;
      constexpr_test(vl == sub_view);
      constexpr_test(is_null_termianted(vl));
      auto sub_view2{text_long.view().substr(14u)};
      vl.assign(sub_view2);
      constexpr_test(vl == sub_view2);
      constexpr_test(is_null_termianted(vl));
      vl.assign(view_type{});
      vl.assign(view_type{});
      constexpr_test(size(vl) == 0u);
      constexpr_test(is_null_termianted(vl));

      vl.assign(size_type(4), char_type('a'));
      constexpr_test(size(vl) == 4u);
      constexpr_test(capacity(vl) == vl.buffered_capacity());
      constexpr_test(all_elememnts_equal_to(begin(vl), end(vl), 'a'));
      constexpr_test(is_null_termianted(vl));

      vl.assign(size_type(124), char_type('a'));
      constexpr_test(size(vl) == 124u);
      constexpr_test(all_elememnts_equal_to(begin(vl), end(vl), 'a'));
      constexpr_test(is_null_termianted(vl));

      vs.clear();
      vs = std::move(vl);
      constexpr_test(size(vs) == 124u);
      constexpr_test(all_elememnts_equal_to(begin(vs), end(vs), 'a'));
      constexpr_test(is_null_termianted(vs));
      constexpr_test(size(vl) == 0u);

      vl.assign(std::move(vs));
      constexpr_test(size(vl) == 124u);
      constexpr_test(all_elememnts_equal_to(begin(vl), end(vl), 'a'));
      constexpr_test(is_null_termianted(vl));
      constexpr_test(size(vs) == 0u);

      vl.assign(text_short.begin(), text_short.end());
      constexpr_test(vl == text_short.view());
      constexpr_test(is_null_termianted(vl));
      return {};
    };
    result |= run_consteval_test<string_type_list>(fn_tmpl);
    result |= run_constexpr_test<string_type_list>(fn_tmpl);
  };

  "basic_string_swap"_test = [&]
  {
    auto fn_tmpl = []<typename string_type>(string_type const *) -> metatests::test_result
    {
      using st = string_type;
      using char_type = typename string_type::char_type;
      auto constexpr text_long1{
        cast_fixed_string<char_type>("Lorem ipsum dolor sit amet, consectetur adipiscing elit."
                                     " Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.")
      };
      auto constexpr text_long2{
        cast_fixed_string<char_type>("Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.")
      };
      auto constexpr text_short1{cast_fixed_string<char_type>("1234")};
      auto constexpr text_short2{cast_fixed_string<char_type>("abc")};

        {
        st vs1;
        st vs2;
        vs1.swap(vs2);
        constexpr_test(empty(vs1));
        constexpr_test(empty(vs2));
        constexpr_test(is_null_termianted(vs1));
        constexpr_test(is_null_termianted(vs2));
        }
        {
        st vs1;
        st vs2{text_short2.view()};
        vs1.swap(vs2);
        constexpr_test(vs1 == text_short2.view());
        constexpr_test(empty(vs2));
        constexpr_test(is_null_termianted(vs1));
        constexpr_test(is_null_termianted(vs2));
        swap(vs1, vs2);
        constexpr_test(vs2 == text_short2.view());
        constexpr_test(empty(vs1));
        std::swap(vs1, vs2);
        constexpr_test(vs1 == text_short2.view());
        constexpr_test(empty(vs2));
        constexpr_test(is_null_termianted(vs1));
        constexpr_test(is_null_termianted(vs2));
        }
        {
        st vs1{text_short1.view()};
        st vs2;
        vs1.swap(vs2);
        constexpr_test(empty(vs1));
        constexpr_test(vs2 == text_short1.view());
        constexpr_test(is_null_termianted(vs1));
        constexpr_test(is_null_termianted(vs2));
        }
        {
        st vs1{text_short1.view()};
        st vs2{text_short2.view()};
        vs1.swap(vs2);
        constexpr_test(vs1 == text_short2.view());
        constexpr_test(vs2 == text_short1.view());
        constexpr_test(is_null_termianted(vs1));
        constexpr_test(is_null_termianted(vs2));
        }
        {
        st vs1{text_short2.view()};
        st vs2{text_short1.view()};
        vs1.swap(vs2);
        constexpr_test(vs1 == text_short1.view());
        constexpr_test(vs2 == text_short2.view());
        constexpr_test(is_null_termianted(vs1));
        constexpr_test(is_null_termianted(vs2));
        }
        {
        st vs1{text_long1.view()};
        st vs2{text_short1.view()};
        vs1.swap(vs2);
        constexpr_test(vs1 == text_short1.view());
        constexpr_test(vs2 == text_long1.view());
        constexpr_test(is_null_termianted(vs1));
        constexpr_test(is_null_termianted(vs2));
        }
        {
        st vs1{text_short1.view()};
        st vs2{text_long1.view()};
        vs1.swap(vs2);
        constexpr_test(vs1 == text_long1.view());
        constexpr_test(vs2 == text_short1.view());
        constexpr_test(is_null_termianted(vs1));
        constexpr_test(is_null_termianted(vs2));
        }
        {
        st vs1{text_long1.view()};
        st vs2{text_long2.view()};
        vs1.swap(vs2);
        constexpr_test(vs1 == text_long2.view());
        constexpr_test(vs2 == text_long1.view());
        constexpr_test(is_null_termianted(vs1));
        constexpr_test(is_null_termianted(vs2));
        }
      return {};
    };
    result |= run_consteval_test<string_type_list>(fn_tmpl);
    result |= run_constexpr_test<string_type_list>(fn_tmpl);
  };

  "basic_string_at"_test = [&]
  {
    auto fn_tmpl = []<typename string_type>(string_type const *) -> metatests::test_result
    {
      using st = string_type;
      using char_type = typename string_type::char_type;
      auto constexpr text_long{
        cast_fixed_string<char_type>("Lorem ipsum dolor sit amet, consectetur adipiscing elit."
                                     " Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.")
      };
      auto constexpr text_short{cast_fixed_string<char_type>("1234")};
      st vs{text_short.view()};
      st vl{text_long.view()};

      constexpr_test(at(vs, 0u) == '1');
      constexpr_test(at(vs, 2u) == '3');
      constexpr_test(at(vs, 3u) == '4');

      constexpr_test(at(vl, 0u) == 'L');
      constexpr_test(at(vl, 2u) == 'r');
      constexpr_test(at(vl, 3u) == 'e');
      return {};
    };
    result |= run_consteval_test<string_type_list>(fn_tmpl);
    result |= run_constexpr_test<string_type_list>(fn_tmpl);
  };

  "basic_string_reserve"_test = [&]
  {
    auto fn_tmpl = []<typename string_type>(string_type const *) -> metatests::test_result
    {
      using st = string_type;
      using char_type = typename string_type::char_type;
      auto constexpr text_long{
        cast_fixed_string<char_type>("Lorem ipsum dolor sit amet, consectetur adipiscing elit."
                                     " Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.")
      };
      auto constexpr text_short{cast_fixed_string<char_type>("1234")};
        {
        st vs{text_short.view()};
        vs.reserve(5u);
        constexpr_test(size(vs) == text_short.size());
        constexpr_test(vs == text_short.view());
        constexpr_test(capacity(vs) >= 5u);
        constexpr_test(is_null_termianted(vs));
        vs.reserve(525u);
        constexpr_test(size(vs) == text_short.size());
        constexpr_test(vs == text_short.view());
        constexpr_test(capacity(vs) >= 525u);
        constexpr_test(is_null_termianted(vs));
        }
        {
        st vl{text_long.view()};
        vl.reserve(5u);
        constexpr_test(size(vl) == text_long.size());
        constexpr_test(vl == text_long.view());
        constexpr_test(capacity(vl) >= 5u);
        constexpr_test(is_null_termianted(vl));
        vl.reserve(525u);
        constexpr_test(size(vl) == text_long.size());
        constexpr_test(vl == text_long.view());
        constexpr_test(capacity(vl) >= 525u);
        constexpr_test(is_null_termianted(vl));
        vl.reserve(1525u);
        constexpr_test(size(vl) == text_long.size());
        constexpr_test(vl == text_long.view());
        constexpr_test(capacity(vl) >= 1525u);
        constexpr_test(is_null_termianted(vl));
        }
      return {};
    };

    result |= run_consteval_test<buffered_string_type_list>(fn_tmpl);
    result |= run_constexpr_test<buffered_string_type_list>(fn_tmpl);
  };
  "basic_string_clear"_test = [&]
  {
    auto fn_tmpl = []<typename string_type>(string_type const *) -> metatests::test_result
    {
      using st = string_type;
      using char_type = typename string_type::char_type;
      auto constexpr text_short{cast_fixed_string<char_type>("1234")};
      st vs{text_short.view()};
      vs.clear();
      constexpr_test(size(vs) == 0u);
      constexpr_test(empty(vs));
      constexpr_test(is_null_termianted(vs));
      auto constexpr text_long{
        cast_fixed_string<char_type>("Lorem ipsum dolor sit amet, consectetur adipiscing elit."
                                     " Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.")
      };
      vs = text_long.view();
      vs.clear();
      constexpr_test(size(vs) == 0u);
      constexpr_test(empty(vs));
      constexpr_test(is_null_termianted(vs));

      return {};
    };
    result |= run_consteval_test<string_type_list>(fn_tmpl);
    result |= run_constexpr_test<string_type_list>(fn_tmpl);
  };

  "basic_string_shrink_to_fit"_test = [&]
  {
    auto fn_tmpl = []<typename string_type>(string_type const *) -> metatests::test_result
    {
      using st = string_type;
      using char_type = typename string_type::char_type;
      using size_type = typename string_type::size_type;
        {
        st s;
        size_type my_capacity{s.capacity()};
        s.shrink_to_fit();
        constexpr_test(size(s) == 0u);
        constexpr_test(is_null_termianted(s));
        constexpr_test(my_capacity == s.capacity());
        }
      auto constexpr text_short{cast_fixed_string<char_type>("1234")};
        {
        st s{text_short};
        size_type my_capacity{s.capacity()};
        s.shrink_to_fit();
        constexpr_test(my_capacity == s.capacity());
        constexpr_test(size(s) == text_short.size());
        constexpr_test(is_null_termianted(s));

        s.reserve(200);
        constexpr_test(200u <= s.capacity());
        constexpr_test(size(s) == text_short.size());

        s.shrink_to_fit();
        constexpr_test(size(s) == text_short.size());
        constexpr_test(is_null_termianted(s));
        constexpr_test(my_capacity == s.capacity());
        }
      auto constexpr text_long{
        cast_fixed_string<char_type>("Lorem ipsum dolor sit amet, consectetur adipiscing elit."
                                     " Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.")
      };
        {
        st s{text_long};
        size_type my_capacity{s.capacity()};
        s.shrink_to_fit();
        constexpr_test(my_capacity == s.capacity());
        constexpr_test(200u > s.capacity());
        constexpr_test(size(s) == text_long.size());
        constexpr_test(is_null_termianted(s));

        s.reserve(200);
        constexpr_test(200u <= s.capacity());
        constexpr_test(size(s) == text_long.size());
        constexpr_test(s == text_long.view());
        constexpr_test(is_null_termianted(s));

        s.shrink_to_fit();
        constexpr_test(size(s) == text_long.size());
        constexpr_test(s == text_long.view());
        constexpr_test(is_null_termianted(s));
        constexpr_test(my_capacity == s.capacity());
        }
      return {};
    };
    result |= run_consteval_test<buffered_string_type_list>(fn_tmpl);
    result |= run_constexpr_test<buffered_string_type_list>(fn_tmpl);
  };

  "basic_string_resize"_test = [&]
  {
    auto fn_tmpl = []<typename string_type>(string_type const *) -> metatests::test_result
    {
      using st = string_type;
      using char_type = typename string_type::char_type;

      auto constexpr text_short{cast_fixed_string<char_type>("1234")};
      auto constexpr expected{cast_fixed_string<char_type>("123")};
        {
        st vs{text_short.view()};
        vs.resize(4u);
        constexpr_test(size(vs) == text_short.size());
        constexpr_test(vs == text_short.view());
        constexpr_test(is_null_termianted(vs));

        vs.resize(3u);
        constexpr_test(size(vs) == 3u);

        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));

        vs.resize(210u);
        constexpr_test(size(vs) == 210u);
        constexpr_test(vs.view().substr(0u, 3u) == expected.view());
        constexpr_test(all_elememnts_equal_to(vs.begin() + 3, vs.end(), '\0'));
        constexpr_test(is_null_termianted(vs));
        vs.resize(220u);
        constexpr_test(size(vs) == 220u);
        constexpr_test(vs.view().substr(0u, 3u) == expected.view());
        constexpr_test(all_elememnts_equal_to(vs.begin() + 3, vs.end(), '\0'));
        constexpr_test(is_null_termianted(vs));
        }

        {
        st vs{text_short.view()};
        vs.resize(4u, 'a');
        constexpr_test(size(vs) == text_short.size());
        constexpr_test(vs == text_short.view());
        constexpr_test(is_null_termianted(vs));

        vs.resize(3u, 'a');
        constexpr_test(size(vs) == 3u);
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        vs.resize(210u, 'a');
        constexpr_test(size(vs) == 210u);
        constexpr_test(vs.view().substr(0u, 3u) == expected.view());
        constexpr_test(all_elememnts_equal_to(vs.begin() + 3, vs.end(), 'a'));
        constexpr_test(is_null_termianted(vs));
        vs.resize(220u, 'b');
        constexpr_test(size(vs) == 220u);
        constexpr_test(vs.view().substr(0u, 3u) == expected.view());
        constexpr_test(all_elememnts_equal_to(vs.begin() + 3, vs.begin() + 210, 'a'));
        constexpr_test(all_elememnts_equal_to(vs.begin() + 210, vs.begin() + 220, 'b'));
        constexpr_test(is_null_termianted(vs));
        }
      return {};
    };

    result |= run_consteval_test<string_type_list>(fn_tmpl);
    result |= run_constexpr_test<string_type_list>(fn_tmpl);
  };

  "basic_string_resize_and_overwrite"_test = [&]
  {
    auto fn_tmpl = []<typename string_type>(string_type const *) -> metatests::test_result
    {
      using st = string_type;
      using char_type = typename string_type::char_type;
      using size_type = typename string_type::size_type;
      auto constexpr text_short{cast_fixed_string<char_type>("a")};
      st vs{text_short.view()};
      vs.resize_and_overwrite(
        8u,
        [](char_type * data, size_type buff_cap) noexcept -> size_type
        {
          std::iota(data, data + 8, char_type('1'));
          constexpr_test(buff_cap >= 8u);
          return 8u;
        }
      );
        {
        auto constexpr expected{cast_fixed_string<char_type>("12345678")};
        constexpr_test(vs == expected.view());
        }
      constexpr_test(is_null_termianted(vs));
      vs.resize_and_overwrite(
        3u,
        [](char_type * data, size_type buff_cap) noexcept -> size_type
        {
          std::iota(data, data + 3, char_type('a'));
          constexpr_test(buff_cap >= 3u);
          return 3u;
        }
      );
        {
        auto constexpr expected{cast_fixed_string<char_type>("abc")};
        constexpr_test(vs == expected.view());
        }
      return {};
    };

    result |= run_consteval_test<string_type_list>(fn_tmpl);
    result |= run_constexpr_test<string_type_list>(fn_tmpl);
  };

  "basic_string_insert"_test = [&]
  {
    auto fn_tmpl = []<typename string_type>(string_type const *) -> metatests::test_result
    {
      using st = string_type;
      using char_type = typename string_type::char_type;

      auto constexpr text_short{cast_fixed_string<char_type>("1234")};
        // insert( size_type index, size_type count, char_type ch )
        {
        st vs{text_short.view()};
        vs.insert(4u, 4u, 'a');
        constexpr_test(size(vs) == 8u);
        auto constexpr expected{cast_fixed_string<char_type>("1234aaaa")};
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        }
        {
        st vs{text_short.view()};
        vs.insert(3u, 4u, 'a');
        constexpr_test(size(vs) == 8u);
        auto constexpr expected{cast_fixed_string<char_type>("123aaaa4")};
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        }
        {
        st vs{text_short.view()};
        vs.insert(0u, 4u, 'a');
        constexpr_test(size(vs) == 8u);
        auto constexpr expected{cast_fixed_string<char_type>("aaaa1234")};
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        }
        {
        st vs{text_short.view()};
        vs.insert(1u, 2u, 'a');
        constexpr_test(size(vs) == 6u);
        auto constexpr expected{cast_fixed_string<char_type>("1aa234")};
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        vs.insert(2u, 4u, 'b');
        constexpr_test(size(vs) == 10u);
        auto constexpr expected2{cast_fixed_string<char_type>("1abbbba234")};
        constexpr_test(vs == expected2.view());
        }
      // insert( size_type index, std::convertible_to<view_type> auto const & t )
      auto constexpr text_short2{cast_fixed_string<char_type>("abcd")};
        {
        st vs{text_short.view()};
        vs.insert(4u, text_short2);
        constexpr_test(size(vs) == 8u);
        auto constexpr expected{cast_fixed_string<char_type>("1234abcd")};
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        }
        {
        st vs{text_short.view()};
        vs.insert(3u, text_short2);
        constexpr_test(size(vs) == 8u);
        auto constexpr expected{cast_fixed_string<char_type>("123abcd4")};
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        }
        {
        st vs{text_short.view()};
        vs.insert(0u, text_short2);
        constexpr_test(size(vs) == 8u);
        auto constexpr expected{cast_fixed_string<char_type>("abcd1234")};
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        }
        {
        st vs{text_short.view()};
        vs.insert(1u, text_short2.view().substr(0u, 2u));
        constexpr_test(size(vs) == 6u);
        auto constexpr expected{cast_fixed_string<char_type>("1ab234")};
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        vs.insert(2u, text_short2);
        constexpr_test(size(vs) == 10u);
        auto constexpr expected2{cast_fixed_string<char_type>("1aabcdb234")};
        constexpr_test(vs == expected2.view());
        }
      auto constexpr text_long{
        cast_fixed_string<char_type>("Lorem ipsum dolor sit amet, consectetur adipiscing elit."
                                     " Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.")
      };
        {
        st vs{text_short.view()};
        vs.insert(4u, text_long);
        constexpr_test(size(vs) == text_long.size() + text_short.size());
        auto constexpr expected{
          cast_fixed_string<char_type>("1234Lorem ipsum dolor sit amet, consectetur adipiscing elit."
                                       " Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.")
        };
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        }
        {
        st vs{text_short.view()};
        vs.insert(0u, text_long);
        constexpr_test(size(vs) == text_long.size() + text_short.size());
        auto constexpr expected{
          cast_fixed_string<char_type>("Lorem ipsum dolor sit amet, consectetur adipiscing elit."
                                       " Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.1234")
        };
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        }
        {
        st vs{text_short.view()};
        vs.insert(2u, text_long);
        constexpr_test(size(vs) == text_long.size() + text_short.size());
        auto constexpr expected{
          cast_fixed_string<char_type>("12Lorem ipsum dolor sit amet, consectetur adipiscing elit."
                                       " Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.34")
        };
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        }
      auto constexpr text_long2{
        cast_fixed_string<char_type>("Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.")
      };
        {
        st vs{text_long.view()};
        vs.insert(4u, text_long2);
        constexpr_test(size(vs) == text_long.size() + text_long2.size());
        auto constexpr expected{
          cast_fixed_string<char_type>("Lore"
                                       "Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante."
                                       "m ipsum dolor sit amet, consectetur adipiscing elit."
                                       " Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.")
        };
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        }
        {
        st vs{text_long.view()};
        vs.insert(text_long.size(), text_long2);
        constexpr_test(size(vs) == text_long.size() + text_long2.size());
        auto constexpr expected{
          cast_fixed_string<char_type>("Lorem ipsum dolor sit amet, consectetur adipiscing elit."
                                       " Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante."
                                       "Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.")
        };
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        }

        // insert( const_iterator pos, char_type ch )
        {
        st vs{text_short.view()};
        vs.insert(std::next(vs.begin(), 3), 4u, 'a');
        constexpr_test(size(vs) == 8u);
        auto constexpr expected{cast_fixed_string<char_type>("123aaaa4")};
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        }
        // insert( const_iterator pos, size_type count, char_type ch )
        {
        st vs{text_short.view()};
        vs.insert(std::next(vs.begin(), 4), 'a');
        constexpr_test(size(vs) == 5u);
        auto constexpr expected{cast_fixed_string<char_type>("1234a")};
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        }
        // insert( const_iterator pos, InputIt first, InputIt last )
        {
        st vs{text_short.view()};
        vs.insert(std::next(vs.begin(), 4), text_short2.begin(), text_short2.end());
        constexpr_test(size(vs) == 8u);
        auto constexpr expected{cast_fixed_string<char_type>("1234abcd")};
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        }
      return {};
    };
    result |= run_consteval_test<string_type_list>(fn_tmpl);
    result |= run_constexpr_test<string_type_list>(fn_tmpl);
  };

  "basic_string_replace"_test = [&]
  {
    auto fn_tmpl = []<typename string_type>(string_type const *) -> metatests::test_result
    {
      using st = string_type;
      using char_type = typename string_type::char_type;

      auto constexpr text_short{cast_fixed_string<char_type>("1234")};
        // replace( size_type pos, size_type count, std::convertible_to<view_type> auto const & v )
        {
        st vs{text_short};
          {
          constexpr auto what{cast_fixed_string<char_type>("14")};
          constexpr auto expected{cast_fixed_string<char_type>("12314")};
          vs.replace(3u, 1u, what);
          constexpr_test(vs == expected.view());
          }
          {
          constexpr auto what{cast_fixed_string<char_type>("1")};
          constexpr auto expected{cast_fixed_string<char_type>("114")};
          vs.replace(1u, 3u, what);
          constexpr_test(vs == expected.view());
          }
          {
          constexpr auto what{cast_fixed_string<char_type>("abcdef123456789")};
          constexpr auto expected{cast_fixed_string<char_type>("1abcdef12345678914")};
          vs.replace(1u, 0u, what);
          constexpr_test(vs == expected.view());
          }
          {
          constexpr auto what{cast_fixed_string<char_type>("abcdef123456789")};
          constexpr auto expected{cast_fixed_string<char_type>("1abcdef12345678914abcdef123456789")};
          vs.replace(18u, 0u, what);
          constexpr_test(vs == expected.view());
          }
          {
          constexpr auto what{cast_fixed_string<char_type>("oprs")};
          constexpr auto expected{cast_fixed_string<char_type>("1aboprs12345678914abcdef123456789")};
          vs.replace(3u, 4u, what);
          constexpr_test(vs == expected.view());
          }
          {
          constexpr auto what{cast_fixed_string<char_type>("tuw")};
          constexpr auto expected{cast_fixed_string<char_type>("1abtuw4abcdef123456789")};
          vs.replace(3u, 14u, what);
          constexpr_test(vs == expected.view());
          }
          {
          constexpr auto what{cast_fixed_string<char_type>("xyz")};
          constexpr auto expected{cast_fixed_string<char_type>("xyzuw4abcdef123456789")};
          vs.replace(0u, 4u, what);
          constexpr_test(vs == expected.view());
          }
          {
          constexpr auto what{cast_fixed_string<char_type>("xyz")};
          constexpr auto expected{cast_fixed_string<char_type>("xyzuw4abcdef12345xyz")};
          vs.replace(17u, 4u, what);
          constexpr_test(vs == expected.view());
          }
        }
        // replace( const_iterator first, const_iterator last, std::convertible_to<view_type> auto const & v )
        {
        st vs{text_short};
        constexpr auto what{cast_fixed_string<char_type>("ab")};
        constexpr auto expected{cast_fixed_string<char_type>("123ab")};
        vs.replace(vs.begin() + 3, vs.begin() + 4, what);
        constexpr_test(vs == expected.view());
        }

        // replace( size_type pos, size_type count, size_type count2, char_type ch )
        {
        st vs{text_short};
          {
          vs.replace(3u, 1u, 2u, 'X');
          constexpr auto expected{cast_fixed_string<char_type>("123XX")};
          constexpr_test(vs == expected.view());
          }
          {
          vs.replace(1u, 3u, 1u, 'Y');
          constexpr auto expected{cast_fixed_string<char_type>("1YX")};
          constexpr_test(vs == expected.view());
          }
          {
          vs.replace(1u, 0u, 20u, 'Z');
          constexpr auto expected{cast_fixed_string<char_type>("1ZZZZZZZZZZZZZZZZZZZZYX")};
          constexpr_test(vs == expected.view());
          }
        }
        // replace( const_iterator first, const_iterator last, size_type count2, char_type ch )
        {
        st vs{text_short};
          {
          vs.replace(vs.begin() + 3u, vs.begin() + 4u, 2u, 'X');
          constexpr auto expected{cast_fixed_string<char_type>("123XX")};
          constexpr_test(vs == expected.view());
          }
          {
          vs.replace(vs.begin() + 1u, vs.begin() + 4u, 1u, 'Y');
          constexpr auto expected{cast_fixed_string<char_type>("1YX")};
          constexpr_test(vs == expected.view());
          }
          {
          vs.replace(vs.begin() + 1u, vs.begin() + 1u, 20u, 'Z');
          constexpr auto expected{cast_fixed_string<char_type>("1ZZZZZZZZZZZZZZZZZZZZYX")};
          constexpr_test(vs == expected.view());
          }
          // replace( const_iterator first, const_iterator last, iterator first2, iterator last2 )
          {
          constexpr auto what{cast_fixed_string<char_type>("xyzuw4abcdef12345xyz")};
          vs.replace(vs.begin() + 1u, vs.begin() + 4u, what.begin(), what.end());
          constexpr auto expected{cast_fixed_string<char_type>("1xyzuw4abcdef12345xyzZZZZZZZZZZZZZZZZZYX")};
          constexpr_test(vs == expected.view());
          }
        }
      return {};
    };
    result |= run_consteval_test<string_type_list>(fn_tmpl);
    result |= run_constexpr_test<string_type_list>(fn_tmpl);
  };

  "basic_string_append"_test = [&]
  {
    auto fn_tmpl = []<typename string_type>(string_type const *) -> metatests::test_result
    {
      using st = string_type;
      using char_type = typename string_type::char_type;

      // append( std::convertible_to<view_type> auto const & s )
      auto constexpr text_short{cast_fixed_string<char_type>("1234")};
      auto constexpr text_short2{cast_fixed_string<char_type>("abc")};
        {
        st vs{text_short};
        vs.append(text_short2);
        constexpr_test(size(vs) == 7u);
        constexpr auto expected{cast_fixed_string<char_type>("1234abc")};
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        }
      auto constexpr text_long{
        cast_fixed_string<char_type>("Lorem ipsum dolor sit amet, consectetur adipiscing elit."
                                     " Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.")
      };
        {
        st vs{text_short};
        vs.append(text_long);
        constexpr_test(size(vs) == text_short.size() + text_long.size());
        auto constexpr expected{
          cast_fixed_string<char_type>("1234"
                                       "Lorem ipsum dolor sit amet, consectetur adipiscing elit."
                                       " Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.")
        };
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        }
        {
        st vs{text_long};
        vs.append(text_short);
        constexpr_test(size(vs) == text_short.size() + text_long.size());
        auto constexpr expected{
          cast_fixed_string<char_type>("Lorem ipsum dolor sit amet, consectetur adipiscing elit."
                                       " Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante."
                                       "1234")
        };
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        }
        // append( std::convertible_to<view_type> auto const & s, size_type index_str, size_type count = npos )
        {
        st vs{text_short};
        vs.append(text_long, 6u, 5u);
        constexpr_test(size(vs) == text_short.size() + 5u);
        auto constexpr expected{cast_fixed_string<char_type>("1234ipsum")};
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        }
        // append( forward_it first, forward_it last )
        {
        st vs{text_short};
        vs.append(text_long.begin(), text_long.begin() + 5u);
        constexpr_test(size(vs) == text_short.size() + 5u);
        auto constexpr expected{cast_fixed_string<char_type>("1234Lorem")};
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        }
        // append( size_type count, char_type ch )
        {
        st vs{text_short};
        vs.append(5u, char_type('x'));
        constexpr_test(size(vs) == text_short.size() + 5u);
        auto constexpr expected{cast_fixed_string<char_type>("1234xxxxx")};
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        }
      return {};
    };
    result |= run_consteval_test<string_type_list>(fn_tmpl);
    result |= run_constexpr_test<string_type_list>(fn_tmpl);
  };

  "basic_string_erase"_test = [&]
  {
    auto fn_tmpl = []<typename string_type>(string_type const *) -> metatests::test_result
    {
      using st = string_type;
      using char_type = typename string_type::char_type;
      auto constexpr text_long{
        cast_fixed_string<char_type>("Lorem ipsum dolor sit amet, consectetur adipiscing elit."
                                     " Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.")
      };
        {
        st vs{text_long};
          {
          vs.erase(0u, 5u);
          auto constexpr expected{
            cast_fixed_string<char_type>(" ipsum dolor sit amet, consectetur adipiscing elit."
                                         " Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.")
          };
          constexpr_test(vs == expected.view());
          constexpr_test(is_null_termianted(vs));
          }
          {
          vs.erase(5u, 5u);
          auto constexpr expected{
            cast_fixed_string<char_type>(" ipsuor sit amet, consectetur adipiscing elit."
                                         " Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.")
          };
          constexpr_test(vs == expected.view());
          constexpr_test(is_null_termianted(vs));
          }
          {
          vs.erase(15u);
          auto constexpr expected{cast_fixed_string<char_type>(" ipsuor sit ame")};
          constexpr_test(vs == expected.view());
          constexpr_test(is_null_termianted(vs));
          }
        }
        // erase( const_iterator pos )
        // erase( const_iterator first, const_iterator last )
        {
        st vs{text_long};
          {
          vs.erase(vs.begin());
          auto constexpr expected{
            cast_fixed_string<char_type>("orem ipsum dolor sit amet, consectetur adipiscing elit."
                                         " Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.")
          };
          constexpr_test(vs == expected.view());
          constexpr_test(is_null_termianted(vs));
          }
          {
          vs.erase(vs.cbegin(), std::next(vs.cbegin(), 5));
          auto constexpr expected{
            cast_fixed_string<char_type>("ipsum dolor sit amet, consectetur adipiscing elit."
                                         " Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.")
          };
          constexpr_test(vs == expected.view());
          constexpr_test(is_null_termianted(vs));
          }
          {
          vs.erase(std::next(vs.cbegin(), 5), std::next(vs.cbegin(), 5));
          auto constexpr expected{
            cast_fixed_string<char_type>("ipsum dolor sit amet, consectetur adipiscing elit."
                                         " Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.")
          };
          constexpr_test(vs == expected.view());
          constexpr_test(is_null_termianted(vs));
          }
          {
          vs.erase(std::next(vs.cbegin(), 5), std::next(vs.cbegin(), 15));
          auto constexpr expected{
            cast_fixed_string<char_type>("ipsum amet, consectetur adipiscing elit."
                                         " Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.")
          };
          constexpr_test(vs == expected.view());
          constexpr_test(is_null_termianted(vs));
          }
          {
          vs.erase(std::next(vs.cbegin(), 25), vs.cend());
          auto constexpr expected{cast_fixed_string<char_type>("ipsum amet, consectetur a")};
          constexpr_test(vs == expected.view());
          constexpr_test(is_null_termianted(vs));
          }
          {
          vs.erase(std::next(vs.cbegin(), 2));
          auto constexpr expected{cast_fixed_string<char_type>("ipum amet, consectetur a")};
          constexpr_test(vs == expected.view());
          constexpr_test(is_null_termianted(vs));
          }
          {
          vs.erase(vs.cend());
          auto constexpr expected{cast_fixed_string<char_type>("ipum amet, consectetur a")};
          constexpr_test(vs == expected.view());
          constexpr_test(is_null_termianted(vs));
          }
        }
      return {};
    };
    result |= run_consteval_test<string_type_list>(fn_tmpl);
    result |= run_constexpr_test<string_type_list>(fn_tmpl);
  };

  "basic_string_pop_back"_test = [&]
  {
    auto fn_tmpl = []<typename string_type>(string_type const *) -> metatests::test_result
    {
      using st = string_type;
      using char_type = typename string_type::char_type;

      auto constexpr text_short{cast_fixed_string<char_type>("1234")};
      st vs{text_short};
        {
        vs.pop_back();
        auto constexpr expected{cast_fixed_string<char_type>("123")};
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        }
        {
        vs.pop_back();
        auto constexpr expected{cast_fixed_string<char_type>("12")};
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        }
        {
        vs.pop_back();
        auto constexpr expected{cast_fixed_string<char_type>("1")};
        constexpr_test(vs == expected.view());
        constexpr_test(is_null_termianted(vs));
        }
        {
        vs.pop_back();
        constexpr_test(vs.empty());
        constexpr_test(is_null_termianted(vs));
        }
      return {};
    };
    result |= run_consteval_test<string_type_list>(fn_tmpl);
    result |= run_constexpr_test<string_type_list>(fn_tmpl);
  };

  "basic_string_find"_test = [&]
  {
    auto fn_tmpl = []<typename string_type>(string_type const *) -> metatests::test_result
    {
      using st = string_type;
      using char_type = typename string_type::char_type;
      auto constexpr text_long{
        cast_fixed_string<char_type>("Lorem ipsum dolor sit amet, consectetur adipiscing elit."
                                     " Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.")
      };
      st vs{text_long};
        {
        auto constexpr what{cast_fixed_string<char_type>("Lorem")};
        constexpr_test(st::npos != vs.find(what));
        }
        {
        auto constexpr what{cast_fixed_string<char_type>("LoremX")};
        constexpr_test(st::npos == vs.find(what));
        }
        {
        auto constexpr what{cast_fixed_string<char_type>("PLorem")};
        constexpr_test(st::npos == vs.find(what));
        }
        {
        auto constexpr what{cast_fixed_string<char_type>("Pellentesque")};
        constexpr_test(st::npos != vs.find(what));
        }
        {
        auto constexpr what{cast_fixed_string<char_type>("Pellentesque")};
        constexpr_test(st::npos != vs.find(what));
        }
        {
        auto constexpr what{cast_fixed_string<char_type>("PeLlentesque")};
        constexpr_test(st::npos == vs.find(what));
        }
        {
        auto constexpr what{cast_fixed_string<char_type>("ante.")};
        constexpr_test(st::npos != vs.find(what));
        }
      auto constexpr cstr{cast_fixed_string<char_type>("Lorem ipsum dolor sit amet, Lorem ipsum dolor sit amet,")};
      st str{cstr};
        // find( char_type ch, size_type pos = 0u ) const noexcept
        {
        constexpr_test(0u == str.find('L'));
        constexpr_test(28u == str.find('L', 1));
        constexpr_test(8u == str.find('s'));
        constexpr_test(18u == str.find('s', 9));
        constexpr_test(string_type::npos == str.find('z', 10));
        }
        // find( std::convertible_to<view_type> auto const & s, size_type pos = 0u ) const noexcept
        {
        auto constexpr cst{cast_fixed_string<char_type>("Lorem")};
        st to_find(cst);
        constexpr_test(0u == str.find(to_find));
        constexpr_test(28u == str.find(to_find, 5));
        constexpr_test(string_type::npos == str.find(to_find, 29));
        }

        // find( std::convertible_to<view_type> auto const & s, size_type pos, size_type count )
        {
        auto constexpr cst{cast_fixed_string<char_type>("Lorem")};
        st to_find(cst);
        constexpr_test(0u == str.find(to_find, 0, 3));
        constexpr_test(28u == str.find(to_find, 5, 3));
        constexpr_test(string_type::npos == str.find(to_find, 29, 3));
        }
      return {};
    };
    result |= run_consteval_test<string_type_list>(fn_tmpl);
    result |= run_constexpr_test<string_type_list>(fn_tmpl);
  };

  "basic_string_rfind"_test = [&]
  {
    auto fn_tmpl = []<typename string_type>(string_type const *) -> metatests::test_result
    {
      using st = string_type;
      using char_type = typename string_type::char_type;
      auto constexpr cstr{cast_fixed_string<char_type>("Lorem ipsum dolor sit amet, Lorem ipsum dolor sit amet,")};

      st str{cstr};
        // rfind( char_type ch, size_type pos = npos ) const noexcept
        {
        constexpr_test(28u == str.rfind('L'));
        constexpr_test(1u == str.rfind('o', 5));
        constexpr_test(0u == str.rfind('L', 27));
        constexpr_test(46u == str.rfind('s'));
        constexpr_test(36u == str.rfind('s', 45));
        constexpr_test(54u == str.rfind(','));
        constexpr_test(54u == str.rfind(',', 54));
        constexpr_test(26u == str.rfind(',', 53));
        constexpr_test(st::npos == str.rfind('z', 10));
        }
      // rfind( std::convertible_to<view_type> auto const & s, size_type pos = npos ) const noexcept
      auto constexpr Lorem{cast_fixed_string<char_type>("Lorem")};
      auto constexpr dolor{cast_fixed_string<char_type>("dolor")};
        {
        st to_find(Lorem);
        constexpr_test(28u == str.rfind(to_find));
        constexpr_test(0u == str.rfind(to_find, 10));
        st to_find2{dolor};
        constexpr_test(string_type::npos == str.rfind(to_find2, 11));
        }
        // rfind( std::convertible_to<view_type> auto const & s, size_type pos, size_type count ) const noexcept
        {
        st to_find(Lorem);
        constexpr_test(52u == str.rfind(to_find.substr(3), str.size(), 1));
        constexpr_test(0u == str.rfind(to_find, 10, 2));
        st to_find2{dolor};
        constexpr_test(string_type::npos == str.rfind(to_find2, 11, 5));
        }
      return {};
    };
    result |= run_consteval_test<string_type_list>(fn_tmpl);
    result |= run_constexpr_test<string_type_list>(fn_tmpl);
  };

  "basic_string_find_and_replace"_test = [&]
  {
    auto fn_tmpl = []<typename string_type>(string_type const *) -> metatests::test_result
    {
      using st = string_type;
      using char_type = typename string_type::char_type;
      auto constexpr text_long{cast_fixed_string<char_type>("Lorem ipsum dolor sit amet, consectetur adipiscing elit."
                                                            " Lorem ipsum dolor sit amet, consectetur adipiscing elit.")
      };
      st vs{text_long};
        {
        constexpr auto what{cast_fixed_string<char_type>("Lorem")};
        constexpr auto with{cast_fixed_string<char_type>("lOREM")};
        vs.find_and_replace(what, with);
        constexpr auto expected{cast_fixed_string<char_type>("lOREM ipsum dolor sit amet, consectetur adipiscing elit."
                                                             " lOREM ipsum dolor sit amet, consectetur adipiscing elit."
        )};
        constexpr_test(vs == expected.view());
        }
        {
        constexpr auto what{cast_fixed_string<char_type>(" ")};
        constexpr auto with{cast_fixed_string<char_type>("__")};
        vs.find_and_replace(what, with);
        constexpr auto expected{
          cast_fixed_string<char_type>("lOREM__ipsum__dolor__sit__amet,__consectetur__adipiscing__elit."
                                       "__lOREM__ipsum__dolor__sit__amet,__consectetur__adipiscing__elit.")
        };
        constexpr_test(vs == expected.view());
        }
        {
        constexpr auto what{cast_fixed_string<char_type>("__a")};
        constexpr auto with{cast_fixed_string<char_type>("=")};
        vs.find_and_replace(what, with);
        constexpr auto expected{
          cast_fixed_string<char_type>("lOREM__ipsum__dolor__sit=met,__consectetur=dipiscing__elit."
                                       "__lOREM__ipsum__dolor__sit=met,__consectetur=dipiscing__elit.")
        };
        constexpr_test(vs == expected.view());
        }
        {
        constexpr auto what{cast_fixed_string<char_type>("__")};
        constexpr auto with{cast_fixed_string<char_type>("")};
        vs.find_and_replace(what, with);
        constexpr auto expected{cast_fixed_string<char_type>("lOREMipsumdolorsit=met,consectetur=dipiscingelit."
                                                             "lOREMipsumdolorsit=met,consectetur=dipiscingelit.")};
        constexpr_test(vs == expected.view());
        }
      return {};
    };
    result |= run_consteval_test<string_type_list>(fn_tmpl);
    result |= run_constexpr_test<string_type_list>(fn_tmpl);
  };

  "basic_string_pop_contains"_test = [&]
  {
    auto fn_tmpl = []<typename string_type>(string_type const *) -> metatests::test_result
    {
      using st = string_type;
      using char_type = typename string_type::char_type;

      auto constexpr text_long{
        cast_fixed_string<char_type>("Lorem ipsum dolor sit amet, consectetur adipiscing elit."
                                     " Praesent ac enim tellus. Pellentesque nec lectus ligula, eu iaculis ante.")
      };
      st vs{text_long};
      auto constexpr Lorem{cast_fixed_string<char_type>("Lorem")};
      constexpr_test(true == vs.contains(Lorem));
      auto constexpr LoremX{cast_fixed_string<char_type>("LoremX")};
      constexpr_test(false == vs.contains(LoremX));
      auto constexpr PLorem{cast_fixed_string<char_type>("PLorem")};
      constexpr_test(false == vs.contains(PLorem));

      auto constexpr Pellentesque{cast_fixed_string<char_type>("Pellentesque")};
      constexpr_test(true == vs.contains(Pellentesque));
      auto constexpr PeLlentesque{cast_fixed_string<char_type>("PeLlentesque")};
      constexpr_test(false == vs.contains(PeLlentesque));
      auto constexpr ante{cast_fixed_string<char_type>("ante.")};
      constexpr_test(true == vs.contains(ante));
      return {};
    };
    result |= run_consteval_test<string_type_list>(fn_tmpl);
    result |= run_constexpr_test<string_type_list>(fn_tmpl);
  };

  "basic_string_starts_with"_test = [&]
  {
    auto fn_tmpl = []<typename string_type>(string_type const *) -> metatests::test_result
    {
      using st = string_type;
      using char_type = typename string_type::char_type;
      auto constexpr text_long{cast_fixed_string<char_type>("Lorem ipsum")};
      st vs{text_long};
      constexpr_test(true == vs.starts_with('L'));
      constexpr_test(true == vs.starts_with(text_long));
        {
        auto constexpr what{cast_fixed_string<char_type>("Lorem ipsum ")};
        constexpr_test(false == vs.starts_with(what));
        }
        {
        auto constexpr what{cast_fixed_string<char_type>("Lorem")};
        constexpr_test(true == vs.starts_with(what));
        }
        {
        auto constexpr what{cast_fixed_string<char_type>("L")};
        constexpr_test(true == vs.starts_with(what));
        }
      return {};
    };
    result |= run_consteval_test<string_type_list>(fn_tmpl);
    result |= run_constexpr_test<string_type_list>(fn_tmpl);
  };

  "basic_string_ends_with"_test = [&]
  {
    auto fn_tmpl = []<typename string_type>(string_type const *) -> metatests::test_result
    {
      using st = string_type;
      using char_type = typename string_type::char_type;
      auto constexpr text_long{cast_fixed_string<char_type>("Lorem ipsum")};
      st vs{text_long};
      constexpr_test(true == vs.ends_with(text_long));
        {
        auto constexpr what{cast_fixed_string<char_type>(" Lorem ipsum")};
        constexpr_test(false == vs.ends_with(what));
        }
        {
        auto constexpr what{cast_fixed_string<char_type>("ipsum")};
        constexpr_test(true == vs.ends_with(what));
        }
        {
        auto constexpr what{cast_fixed_string<char_type>("m")};
        constexpr_test(true == vs.ends_with(what));
        }
      constexpr_test(true == vs.ends_with('m'));
      return {};
    };
    result |= run_consteval_test<string_type_list>(fn_tmpl);
    result |= run_constexpr_test<string_type_list>(fn_tmpl);
  };

  "basic_string_compare"_test = [&]
  {
    auto fn_tmpl = []<typename string_type>(string_type const *) -> metatests::test_result
    {
      using st = string_type;
      using char_type = typename string_type::char_type;

      constexpr auto text{cast_fixed_string<char_type>("Lorem ipsum dolor sit amet, Lorem ipsum dolor sit amet,")};
      st str{text};
      st str1(str.substr(0, 5));
      st str1b(str.substr(0, 15));
      st str2(str.substr(6, 5));

        // compare( std::convertible_to<view_type> auto const & s ) const noexcept
        {
        constexpr_test(str1.compare(str2) < 0);
        constexpr_test(str1 < str2);
        constexpr_test(str1 <=> str2 == std::strong_ordering::less);
        constexpr_test(str2 <=> str1 == std::strong_ordering::greater);

        constexpr_test(str1.compare(str1b) < 0);
        constexpr_test(str1 < str1b);
        constexpr_test(str1 <=> str1b == std::strong_ordering::less);
        constexpr_test(str1b <=> str1 == std::strong_ordering::greater);

        constexpr_test(0 == str1.compare(str1));
        constexpr_test(str1 == str1);
        constexpr_test(str1 <=> str1 == std::strong_ordering::equal);

        constexpr_test(!(str1 != str1));
        constexpr_test(!(str1 < str1));
        constexpr_test(!(str1 > str1));

        constexpr_test(str1b.compare(str1) > 0);
        constexpr_test(str1b > str1);

        constexpr_test(str2.compare(str1) > 0);
        constexpr_test(str2 > str1);

        constexpr_test(!(str2 == str1));
        constexpr_test(str2 != str1);
        constexpr_test(!(str2 < str1));
        }
        // compare( size_type pos1, size_type count1,
        //     std::convertible_to<view_type> auto const & s ) const noexcept
        {
        constexpr_test(str1.compare(1, 2, str2) > 0);  // or.ipsum
        constexpr_test(str1.compare(0, 2, str1) < 0);  // Lo.Lorem
        constexpr_test(str2.compare(1, 3, str1) > 0);  // psum.Lorem
        }

      return {};
    };
    result |= run_consteval_test<string_type_list>(fn_tmpl);
    result |= run_constexpr_test<string_type_list>(fn_tmpl);
  };

  "basic_string_find_first_of"_test = [&]
  {
    auto fn_tmpl = []<typename string_type>(string_type const *) -> metatests::test_result
    {
      using st = string_type;
      using char_type = typename string_type::char_type;
      constexpr auto text{cast_fixed_string<char_type>("Lorem ipsum dolor sit amet, Lorem ipsum dolor sit amet,")};
      st str{text};
        {
        constexpr auto what{cast_fixed_string<char_type>("oL")};
        constexpr_test(str.find_first_of(what) == 0u);
        }
        {
        constexpr auto what{cast_fixed_string<char_type>("ol")};
        constexpr_test(str.find_first_of(what) == 1u);
        }
        {
        constexpr auto what{cast_fixed_string<char_type>("oL")};
        constexpr_test(str.find_first_of(what, 5u) == 13u);
        }
      return {};
    };
    result |= run_consteval_test<string_type_list>(fn_tmpl);
    result |= run_constexpr_test<string_type_list>(fn_tmpl);
  };

  "basic_string_rbegin_rend"_test = [&]
  {
    auto fn_tmpl = []<typename string_type>(string_type const *) -> metatests::test_result
    {
      using st = string_type;
      using char_type = typename string_type::char_type;
      constexpr auto text{cast_fixed_string<char_type>("Lorem ipsum dolor sit amet")};
      st str{text};
      st rstr{str.rbegin(), str.rend()};
      constexpr auto what{cast_fixed_string<char_type>("tema tis rolod muspi meroL")};
      constexpr_test(rstr == what.view());
      st str2{rstr.rbegin(), rstr.rend()};
      constexpr_test(str2 == text.view());
      return {};
    };
    result |= run_consteval_test<string_type_list>(fn_tmpl);
    result |= run_constexpr_test<string_type_list>(fn_tmpl);
  };

  "basic_string_hash"_test = [&]
  {
    auto fn_tmpl = []<typename string_type>(string_type const *) -> metatests::test_result
    {
      using st = string_type;
      using char_type = typename string_type::char_type;
      constexpr auto text{cast_fixed_string<char_type>("Lorem ipsum dolor sit amet")};
      constexpr std::hash<std::basic_string_view<char_type>> stl_hash{};

      st str{text};
      if(std::is_constant_evaluated())
        constexpr_test(coll::hash(str) != 0u);
      else
        constexpr_test(coll::hash(str) == stl_hash(str.view()));
      return {};
    };
    result |= run_consteval_test<string_type_list>(fn_tmpl);
    result |= run_constexpr_test<string_type_list>(fn_tmpl);
  };
  }

