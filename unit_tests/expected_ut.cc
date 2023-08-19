#define SMALL_VECTORS_ENABLE_CUSTOM_EXCPECTED 1

#include <utils/expected.h>
#include <unit_test_core.h>

using metatests::constexpr_test;
using metatests::run_consteval_test;
using metatests::run_constexpr_test;
using metatests::run_consteval_test_dual;
using metatests::run_constexpr_test_dual;

namespace ut = boost::ut;
using ut::operator""_test;
using namespace ut::operators::terse;
using metatests::test_result;

using value_type_list = metatests::type_list<int32_t,uint32_t,void,non_trivial,non_trivial_ptr,non_trivial_ptr_except>;
using value_type_non_void_list = metatests::type_list<int32_t,uint32_t,non_trivial,non_trivial_ptr,non_trivial_ptr_except>;
using value_type_non_void_noexcept_list = metatests::type_list<int32_t,uint32_t,non_trivial,non_trivial_ptr>;
enum struct test_error { no_error, error1, error2 };
constexpr test_error operator++( test_error v) noexcept
  {
  return static_cast<test_error>( static_cast<int>(v) + 1 );
  }
using error_type_list = metatests::type_list<int32_t,test_error,non_trivial,non_trivial_ptr,non_trivial_ptr_except>;
using namespace cxx23;

static_assert(concepts::not_expected<int>);
static_assert(concepts::not_expected<void>);
static_assert(concepts::not_expected<non_trivial>);
static_assert(!concepts::not_expected<expected<void,int>>);
static_assert(!concepts::not_expected<expected<int,int>>);
static_assert(!concepts::not_expected<expected<non_trivial,int>>);

template<typename expected_return_type>
struct implicit_test_t
  {
  template<typename I>
  constexpr auto operator()(I && i) const noexcept
      -> expected_return_type
    {
    return std::forward<I>(i);
    }
  };
template<typename expected_return_type>
inline constexpr implicit_test_t<expected_return_type> implicit_test;

namespace expected_value_return_type_check
{
  consteval bool test_ref()
    {
    expected<int,int> v{};
    expected<void,int> v2{};
    return std::is_same_v<decltype(v.value()), int &>
        && std::is_same_v<decltype(*v), int &>
        && std::is_same_v<decltype(v2.value()), void>
        && std::is_same_v<decltype(*v2), void>;
    }
  static_assert(test_ref());
  
  consteval bool test_const_ref()
    {
    const expected<int,int> v{};
    return std::is_same_v<decltype(v.value()), int const &>
        && std::is_same_v<decltype(*v), int const &>;
    }
  static_assert(test_const_ref());
  
  consteval bool test_rref()
    {
    return std::is_same_v<decltype(expected<int,int>{}.value()), int &&>
        && std::is_same_v<decltype(*expected<int,int>{}), int &&>
        && std::is_same_v<decltype(expected<void,int>{}.value()), void>
        && std::is_same_v<decltype(*expected<void,int>{}), void>;
    }
  static_assert(test_rref());
  
  consteval bool test_const_rref()
    {
    return std::is_same_v<decltype(std::add_const_t<expected<int,int>>{}.value()), int const &&>
        && std::is_same_v<decltype(*std::add_const_t<expected<int,int>>{}), int const &&>;
    }
  static_assert(test_const_rref());
}
namespace expected_error_return_type_check
{
  consteval bool test_ref()
    {
    expected<int,int> v{};
    expected<void,int> v2{};
    return std::is_same_v<decltype(v.error()), int &>
        && std::is_same_v<decltype(v2.error()), int &>;
    }
  static_assert(test_ref());
  
  consteval bool test_const_ref()
    {
    const expected<int,int> v{};
    const expected<void,int> v2{};
    return std::is_same_v<decltype(v.error()), int const &>
        && std::is_same_v<decltype(v2.error()), int const &>;
    }
  static_assert(test_const_ref());
  
  consteval bool test_rref()
    {
    return std::is_same_v<decltype(expected<int,int>{}.error()), int &&>
        && std::is_same_v<decltype(expected<void,int>{}.error()), int &&>;
    }
  static_assert(test_rref());
  
  consteval bool test_const_rref()
    {
    return std::is_same_v<decltype(std::add_const_t<expected<int,int>>{}.error()), int const &&>
        && std::is_same_v<decltype(std::add_const_t<expected<void,int>>{}.error()), int const &&>;
    }
  static_assert(test_const_rref());
}
namespace unexpected_test
{
static void do_test(test_result &result)
  {
  "unexpected_constr"_test = [&]
    {
    auto fn_tmpl =
      []<typename error_type>
        ( error_type const *) -> metatests::test_result
      {
      using unexpected_type = unexpected<error_type>;
        {
        unexpected un{error_type{}};
        static_assert( std::same_as<unexpected_type,decltype(un)>);
        }
        {
        unexpected un{ static_cast<error_type>(2)};
        constexpr_test(un.error() == static_cast<error_type>(2) );
        }
        {
        unexpected_type un{ in_place, static_cast<error_type>(2)};
        constexpr_test(un.error() == static_cast<error_type>(2) );
        }
        {
        unexpected const un{ static_cast<error_type>(2)};
        constexpr_test(un.error() == static_cast<error_type>(2) );
        }
        {
        constexpr_test(unexpected{ static_cast<error_type>(2)}.error() == static_cast<error_type>(2) );
        }
        {
        constexpr_test(std::add_const_t<unexpected_type>{ static_cast<error_type>(2)}.error() == static_cast<error_type>(2) );
        }

      return {};
      };
    result |= run_consteval_test<error_type_list>(fn_tmpl);
    result |= run_constexpr_test<error_type_list>(fn_tmpl);
    };
    
  "unexpected_compare"_test = [&]
    {
      auto fn_tmpl = []() -> metatests::test_result
        {
        constexpr_test( unexpected<int>{2} == unexpected<short>{short(2)});
        constexpr_test( unexpected<int>{2} != unexpected<short>{short(3)});
        return {};
        };
      result |= run_consteval_test(fn_tmpl);
      result |= run_constexpr_test(fn_tmpl);
    };
  "unexpected_swap"_test = [&]
    {
      auto fn_tmpl =
      []<typename error_type>
        ( error_type const *) -> metatests::test_result
        {
        unexpected<error_type> un1{error_type(1)};
        unexpected<error_type> un2{error_type(2)};
        un1.swap(un2);
        constexpr_test( un1.error() == error_type(2));
        constexpr_test( un2.error() == error_type(1));
        return {};
        };
      result |= run_consteval_test<error_type_list>(fn_tmpl);
      result |= run_constexpr_test<error_type_list>(fn_tmpl);
    };
  }
}

namespace expected_test
{
static void do_test(test_result &result)
  {
  "expected_constr"_test = [&]
    {
    auto fn_tmpl =
      []<typename value_type, typename error_type>
        ( value_type const *, error_type const *) -> metatests::test_result
      {
      using unexpected_type = unexpected<error_type>;
      using expected_type = expected<value_type,error_type>;
        {
        expected_type ex;
        constexpr_test(ex.has_value());
        constexpr_test(ex.value() == value_type{});
        }
        {
        expected_type ex{value_type{2}};
        constexpr_test(ex.has_value());
        constexpr_test(ex.value() == value_type{2});
        }
        {
        expected_type const sr{value_type{2}};
        expected_type ex{ sr };
        constexpr_test(ex.has_value());
        constexpr_test(ex.value() == value_type{2});
        }
        {
        expected_type ex{ in_place, value_type{2}};
        constexpr_test(ex.has_value());
        constexpr_test(ex.value() == value_type{2});
        }
        {
        expected_type const sr{ in_place, value_type{2}};
        expected_type ex{ sr };
        constexpr_test(ex.has_value());
        constexpr_test(ex.value() == value_type{2});
        }
        {
        expected_type sr{ in_place, value_type{2}};
        expected_type ex{ std::move(sr) };
        constexpr_test(ex.has_value());
        constexpr_test(ex.value() == value_type{2});
        }
        {
        expected_type ex{ unexpect, error_type{4}};
        constexpr_test(!ex.has_value());
        constexpr_test(ex.error() == error_type{4});
        }
        {
        unexpected_type const ue{ error_type{4}};
        expected_type ex{ ue };
        constexpr_test(!ex.has_value());
        constexpr_test(ex.error() == error_type{4});
        }
        {
        unexpected_type ue{ error_type{4}};
        expected_type ex{ std::move(ue) };
        constexpr_test(!ex.has_value());
        constexpr_test(ex.error() == error_type{4});
        }
      return {};
      };
    result |= run_consteval_test_dual<value_type_non_void_list,error_type_list>(fn_tmpl);
    result |= run_constexpr_test_dual<value_type_non_void_list,error_type_list>(fn_tmpl);
    };
    
  "expected_constr_convert"_test = [&]
    {
    auto fn_tmpl = []() -> metatests::test_result
      {
        {
        expected<uint8_t,uint16_t> const ex1{uint8_t{2}};
        expected<int,int> ex2(ex1);
        constexpr_test(ex2.has_value());
        constexpr_test(ex2.value() == 2);
        constexpr_test(implicit_test<expected<int,int>>(ex1) == 2);
        }
        {
        expected<int,int> ex2(expected<uint8_t,uint16_t>{uint8_t{2}});
        constexpr_test(ex2.has_value());
        constexpr_test(ex2.value() == 2);
        constexpr_test(implicit_test<expected<int,int>>(expected<uint8_t,uint16_t>{uint8_t{2}}) == 2);
        }
        {
        unexpected const un{uint8_t(2)};
        expected<uint8_t,uint16_t> ex1{ un };
        expected<int,int> ex2(ex1);
        constexpr_test(!ex2.has_value());
        constexpr_test(ex2.error() == 2);
        constexpr_test(implicit_test<expected<int,int>>(un).error() == 2);
        }
        {
        expected<uint8_t,uint16_t> ex1{ unexpected(uint8_t(2))};
        expected<int,int> ex2(ex1);
        constexpr_test(!ex2.has_value());
        constexpr_test(ex2.error() == 2);
        constexpr_test(implicit_test<expected<int,int>>(unexpected(uint8_t(2))).error() == 2);
        }
        {
        uint8_t const val{2};
        expected<int,int> ex2(val);
        constexpr_test(ex2.has_value());
        constexpr_test(ex2.value() == 2);
        constexpr_test(implicit_test<expected<int,int>>(val) == 2);
        }
        {
        expected<int,int> ex2(uint8_t{2});
        constexpr_test(ex2.has_value());
        constexpr_test(ex2.value() == 2);
        constexpr_test(implicit_test<expected<int,int>>(uint8_t{2}) == 2);
        }
      return {};
      };
    result |= run_consteval_test(fn_tmpl);
    result |= run_constexpr_test(fn_tmpl);
    };
  "expected_constr_void"_test = [&]
    {
    auto fn_tmpl =
      []<typename error_type>
        ( error_type const *) -> metatests::test_result
      {
      using unexpected_type = unexpected<error_type>;
      using expected_type = expected<void,error_type>;
        {
        expected_type ex;
        constexpr_test(ex.has_value());
        }
        {
        expected_type ex{ in_place};
        constexpr_test(ex.has_value());
        }
        {
        expected_type const sr{ in_place};
        expected_type ex{ sr };
        constexpr_test(ex.has_value());
        }
        {
        expected_type sr{ in_place};
        expected_type ex{ std::move(sr) };
        constexpr_test(ex.has_value());
        }
        {
        expected_type ex{ unexpect, error_type{4}};
        constexpr_test(!ex.has_value());
        constexpr_test(ex.error() == error_type{4});
        }
        {
        unexpected_type const ue{ error_type{4}};
        expected_type ex{ ue };
        constexpr_test(!ex.has_value());
        constexpr_test(ex.error() == error_type{4});
        }
        {
        unexpected_type ue{ error_type{4}};
        expected_type ex{ std::move(ue) };
        constexpr_test(!ex.has_value());
        constexpr_test(ex.error() == error_type{4});
        }
      return {};
      };
    result |= run_consteval_test<error_type_list>(fn_tmpl);
    result |= run_constexpr_test<error_type_list>(fn_tmpl);
    };
    
  "expected_operator->"_test = [&]
    {
    auto fn_tmpl = []()
          -> metatests::test_result
      {
      using expected_type = expected<non_trivial_ptr_except,int>;
        {
        non_trivial_ptr_except v{ 2 };
        expected_type ex{ in_place, std::move(v)};
        constexpr_test(ex.has_value());
        constexpr_test(ex->value() == 2);
        }
        {
        non_trivial_ptr_except v{ 2 };
        expected_type const ex{ in_place, std::move(v)};
        constexpr_test(ex.has_value());
        constexpr_test( ex->value() == 2);
        }
      return {};
      };
    result |= run_consteval_test(fn_tmpl);
    result |= run_constexpr_test(fn_tmpl);
    };
    
  "expected_operator*"_test = [&]
    {
    auto fn_tmpl =
      []<typename value_type>
        ( value_type const *) -> metatests::test_result
      {
      using expected_type = expected<value_type,int>;
        {
        expected_type ex{ in_place, value_type{2}};
        constexpr_test(ex.has_value());
        constexpr_test(*ex == value_type{2});
        }
        {
        expected_type const ex{ in_place, value_type{2}};
        constexpr_test(ex.has_value());
        constexpr_test(*ex == value_type{2});
        }
        {
        expected_type ex{ in_place, value_type{2}};
        constexpr_test(ex.has_value());
        constexpr_test(*std::move(ex) == value_type{2});
        }
        {
        constexpr_test(*std::add_const_t<expected_type>{ in_place, value_type{2}} == value_type{2});
        }
      return {};
      };
    result |= run_consteval_test<value_type_non_void_list>(fn_tmpl);
    result |= run_constexpr_test<value_type_non_void_list>(fn_tmpl);
    };

  "expected_operator_bool"_test = [&]
    {
    auto fn_tmpl =
      []() -> metatests::test_result
      {
        {
        using expected_type = expected<int,int>;
        expected_type ex{ in_place, 2};
        constexpr_test(static_cast<bool>(ex));
        }
        {
        using expected_type = expected<void,int>;
        expected_type ex{ in_place};
        constexpr_test(static_cast<bool>(ex));
        }
        {
        using expected_type = expected<int,int>;
        expected_type ex{ unexpect, 4};
        constexpr_test(!static_cast<bool>(ex));
        }
        {
        using expected_type = expected<void,int>;
        expected_type ex{ unexpect, 4};
        constexpr_test(!static_cast<bool>(ex));
        }
      return {};
      };
    result |= run_consteval_test(fn_tmpl);
    result |= run_constexpr_test(fn_tmpl);
    };

  "expected_value()"_test = [&]
    {
    auto fn_tmpl =
      []<typename value_type>
        ( value_type const *) -> metatests::test_result
      {
      using expected_type = expected<value_type,int>;
        {
        expected_type ex{ in_place, value_type{2}};
        constexpr_test(ex.has_value());
        constexpr_test(ex.value() == value_type{2});
        }
        {
        expected_type const ex{ in_place, value_type{2}};
        constexpr_test(ex.has_value());
        constexpr_test(ex.value() == value_type{2});
        }
        {
        expected_type ex{ in_place, value_type{2}};
        constexpr_test(ex.has_value());
        constexpr_test(std::move(ex).value() == value_type{2});
        }
        {
        constexpr_test(std::add_const_t<expected_type>{ in_place, value_type{2}}.value() == value_type{2});
        }
      return {};
      };
    result |= run_consteval_test<value_type_non_void_list>(fn_tmpl);
    result |= run_constexpr_test<value_type_non_void_list>(fn_tmpl);
    };
    
  "expected_value() void"_test = [&]
    {
    auto fn_tmpl =
      []() -> metatests::test_result
      {
      using expected_type = expected<void,int>;
        {
        expected_type const ex{ in_place };
        constexpr_test(ex.has_value());
        ex.value(); // if throws not a valid const eval expresion
        }
        {
        expected_type ex{ in_place };
        constexpr_test(ex.has_value());
        std::move(ex).value(); // if throws not a valid const eval expresion
        }
      return {};
      };
    result |= run_consteval_test(fn_tmpl);
    result |= run_constexpr_test(fn_tmpl);
    };

  "expected_error()"_test = [&]
    {
    auto fn_tmpl =
      []<typename error_type>
        ( error_type const *) -> metatests::test_result
      {
        {
        using expected_type = expected<int,error_type>;
        expected_type ex{ unexpect, error_type{4}};
        constexpr_test(!ex.has_value());
        constexpr_test(ex.error() == error_type{4});
        }
        {
        using expected_type = expected<int,error_type>;
        expected_type const ex{ unexpect, error_type{4}};
        constexpr_test(!ex.has_value());
        constexpr_test(ex.error() == error_type{4});
        }
        {
        using expected_type = expected<int,error_type>;
        constexpr_test(expected_type{ unexpect, error_type{4}}.error() == error_type{4});
        }
        {
        using expected_type = expected<int,error_type>;
        constexpr_test(std::add_const_t<expected_type>{ unexpect, error_type{4}}.error() == error_type{4});
        }
        {
        using expected_type = expected<void,error_type>;
        expected_type ex{ unexpect, error_type{4}};
        constexpr_test(!ex.has_value());
        constexpr_test(ex.error() == error_type{4});
        }
        {
        using expected_type = expected<void,error_type>;
        expected_type const ex{ unexpect, error_type{4}};
        constexpr_test(!ex.has_value());
        constexpr_test(ex.error() == error_type{4});
        }
        {
        using expected_type = expected<void,error_type>;
        constexpr_test(expected_type{ unexpect, error_type{4}}.error() == error_type{4});
        }
        {
        using expected_type = expected<void,error_type>;
        constexpr_test(std::add_const_t<expected_type>{ unexpect, error_type{4}}.error() == error_type{4});
        }
      return {};
      };
    result |= run_consteval_test<error_type_list>(fn_tmpl);
    result |= run_constexpr_test<error_type_list>(fn_tmpl);
    };
    
  "expected_emplace()"_test = [&]
    {
    auto fn_tmpl =
      []<typename value_type, typename error_type>
        ( value_type const *, error_type const *) -> metatests::test_result
      {
        {
        using expected_type = expected<value_type,error_type>;
        expected_type ex{ in_place, value_type{2}};
        constexpr_test(ex.has_value());
        constexpr_test(ex.value() == value_type{2});
        ex.emplace( value_type{4} );
        constexpr_test(ex.has_value());
        constexpr_test(ex.value() == value_type{4});
        }
        {
        using expected_type = expected<value_type,error_type>;
        expected_type ex{ unexpect, error_type{4}};
        constexpr_test(!ex.has_value());
        constexpr_test(ex.error() == error_type{4});
        ex.emplace( value_type{4} );
        constexpr_test(ex.has_value());
        constexpr_test(ex.value() == value_type{4});
        }
      return {};
      };
    result |= run_consteval_test_dual<value_type_non_void_noexcept_list,error_type_list>(fn_tmpl);
    result |= run_consteval_test_dual<value_type_non_void_noexcept_list,error_type_list>(fn_tmpl);
    };
    
  "expected_emplace() void"_test = [&]
    {
    auto fn_tmpl =
      []<typename error_type>
        (error_type const *) -> metatests::test_result
      {
        {
        using expected_type = expected<void,error_type>;
        expected_type ex{ in_place };
        constexpr_test(ex.has_value());
        ex.emplace();
        constexpr_test(ex.has_value());
        }
        {
        using expected_type = expected<void,error_type>;
        expected_type ex{ unexpect, error_type{4}};
        constexpr_test(!ex.has_value());
        constexpr_test(ex.error() == error_type{4});
        ex.emplace();
        constexpr_test(ex.has_value());
        }
      return {};
      };
    result |= run_consteval_test<error_type_list>(fn_tmpl);
    result |= run_consteval_test<error_type_list>(fn_tmpl);
    };
    
  "expected_swap()"_test = [&]
    {
    auto fn_tmpl =
      []<typename value_type, typename error_type>
        ( value_type const *, error_type const *) -> metatests::test_result
      {
      // exclude <non_trivial_ptr_except,non_trivial_ptr_except>
      if constexpr( !std::same_as<value_type,non_trivial_ptr_except> || !std::same_as<error_type,non_trivial_ptr_except> )
        {
        using expected_type = expected<value_type,error_type>;
          {
          expected_type ex1{ in_place, value_type{2}};
          expected_type ex2{ in_place, value_type{4}};
          constexpr_test(ex1.has_value());
          constexpr_test(ex2.has_value());
          constexpr_test(ex1.value() == value_type{2});
          constexpr_test(ex2.value() == value_type{4});
          ex1.swap(ex2);
          constexpr_test(ex1.has_value());
          constexpr_test(ex2.has_value());
          constexpr_test(ex1.value() == value_type{4});
          constexpr_test(ex2.value() == value_type{2});
          }
          {
          expected_type ex1{ unexpect, error_type{2}};
          expected_type ex2{ unexpect, error_type{4}};
          constexpr_test(!ex1.has_value());
          constexpr_test(!ex2.has_value());
          constexpr_test(ex1.error() == error_type{2});
          constexpr_test(ex2.error() == error_type{4});
          ex1.swap(ex2);
          constexpr_test(!ex1.has_value());
          constexpr_test(!ex2.has_value());
          constexpr_test(ex1.error() == error_type{4});
          constexpr_test(ex2.error() == error_type{2});
          }
          {
          expected_type ex1{ in_place, value_type{2}};
          expected_type ex2{ unexpect, error_type{4}};
          constexpr_test(ex1.has_value());
          constexpr_test(!ex2.has_value());
          constexpr_test(ex1.value() == value_type{2});
          constexpr_test(ex2.error() == error_type{4});
          ex1.swap(ex2);
          constexpr_test(!ex1.has_value());
          constexpr_test(ex2.has_value());
          constexpr_test(ex1.error() == error_type{4});
          constexpr_test(ex2.value() == value_type{2});
          }
          {
          expected_type ex2{ in_place, value_type{2}};
          expected_type ex1{ unexpect, error_type{4}};
          constexpr_test(ex2.has_value());
          constexpr_test(!ex1.has_value());
          constexpr_test(ex2.value() == value_type{2});
          constexpr_test(ex1.error() == error_type{4});
          ex2.swap(ex1);
          constexpr_test(!ex2.has_value());
          constexpr_test(ex1.has_value());
          constexpr_test(ex2.error() == error_type{4});
          constexpr_test(ex1.value() == value_type{2});
          }
        }
      return {};
      };
    result |= run_consteval_test_dual<value_type_non_void_list,error_type_list>(fn_tmpl);
    result |= run_consteval_test_dual<value_type_non_void_list,error_type_list>(fn_tmpl);
    };
    
  "expected_swap() void"_test = [&]
    {
    auto fn_tmpl =
      []<typename error_type>
        ( error_type const *) -> metatests::test_result
      {
      using expected_type = expected<void,error_type>;
        {
        expected_type ex1{ in_place};
        expected_type ex2{ in_place};
        constexpr_test(ex1.has_value());
        constexpr_test(ex2.has_value());
        ex1.swap(ex2);
        constexpr_test(ex1.has_value());
        constexpr_test(ex2.has_value());
        }
        {
        expected_type ex1{ unexpect, error_type{2}};
        expected_type ex2{ unexpect, error_type{4}};
        constexpr_test(!ex1.has_value());
        constexpr_test(!ex2.has_value());
        constexpr_test(ex1.error() == error_type{2});
        constexpr_test(ex2.error() == error_type{4});
        ex1.swap(ex2);
        constexpr_test(!ex1.has_value());
        constexpr_test(!ex2.has_value());
        constexpr_test(ex1.error() == error_type{4});
        constexpr_test(ex2.error() == error_type{2});
        }
        {
        expected_type ex1{ in_place};
        expected_type ex2{ unexpect, error_type{4}};
        constexpr_test(ex1.has_value());
        constexpr_test(!ex2.has_value());
        constexpr_test(ex2.error() == error_type{4});
        ex1.swap(ex2);
        constexpr_test(!ex1.has_value());
        constexpr_test(ex2.has_value());
        constexpr_test(ex1.error() == error_type{4});
        }
        {
        expected_type ex2{ in_place };
        expected_type ex1{ unexpect, error_type{4}};
        constexpr_test(ex2.has_value());
        constexpr_test(!ex1.has_value());
        constexpr_test(ex1.error() == error_type{4});
        ex2.swap(ex1);
        constexpr_test(!ex2.has_value());
        constexpr_test(ex1.has_value());
        constexpr_test(ex2.error() == error_type{4});
        }
      return {};
      };
    result |= run_consteval_test<error_type_list>(fn_tmpl);
    result |= run_consteval_test<error_type_list>(fn_tmpl);
    };
  "expected_operator=="_test = [&]
    {
    auto fn_tmpl =
      []<typename value_type, typename error_type>
        ( value_type const *, error_type const *) -> metatests::test_result
      {
      using expected_type = expected<value_type,error_type>;

      expected_type ex1{ in_place, value_type{2}};
      expected_type ex2{ in_place, value_type{4}};
      expected_type ex3{ unexpect, error_type{2}};
      expected_type ex4{ unexpect, error_type{4}};
      unexpected ux { error_type{4} };
      constexpr_test(ex1 == value_type{2});
      constexpr_test(ex2 == value_type{4});
      constexpr_test(ex2 != ex1);
      constexpr_test(ex2 != ex3);
      constexpr_test(ex3 != ex4);
      constexpr_test(ex3 == ex3);
      constexpr_test(ex3 != ux);
      constexpr_test(ex4 == ux);

      return {};
      };
    result |= run_consteval_test_dual<value_type_non_void_list,error_type_list>(fn_tmpl);
    result |= run_consteval_test_dual<value_type_non_void_list,error_type_list>(fn_tmpl);
    };
    
  "expected_operator== void"_test = [&]
    {
    auto fn_tmpl =
      []<typename error_type>
        (error_type const *) -> metatests::test_result
      {
        using expected_type = expected<void,error_type>;
          {
          expected_type ex1{ in_place};
          expected_type ex2{ in_place};
          expected_type ex3{ unexpect, error_type{2}};
          expected_type ex4{ unexpect, error_type{4}};
          unexpected ux { error_type{4} };
          constexpr_test(ex1 == ex2);
          constexpr_test(ex2 != ex3);
          constexpr_test(ex3 != ex4);
          constexpr_test(ex3 == ex3);
          constexpr_test(ex3 != ux);
          constexpr_test(ex4 == ux);
          }
      return {};
      };
    result |= run_consteval_test<error_type_list>(fn_tmpl);
    result |= run_consteval_test<error_type_list>(fn_tmpl);
    };
    
  "expected and_then"_test = [&]
    {
    auto fn_tmpl =
      []<typename value_type, typename error_type>
        ( value_type const *, error_type const *) -> metatests::test_result
      {
      using expected_type = expected<value_type,error_type>;
      auto f = [](value_type v) noexcept { return expected_type{ in_place, ++v}; };
        {
        expected_type const ex{ in_place, value_type{2} };
        auto res { ex.and_then(f) };
        constexpr_test( std::same_as<decltype(res), expected_type>);
        constexpr_test( res == value_type{3});
        }
        {
        auto res { expected_type{ in_place, value_type{2} }.and_then(f) };
        constexpr_test( std::same_as<decltype(res), expected_type>);
        constexpr_test( res == value_type{3});
        }
        {
        expected_type const ex{ unexpect, error_type{2} };
        auto res { ex.and_then(f) };
        constexpr_test( std::same_as<decltype(res), expected_type>);
        constexpr_test( res == unexpected{error_type(2)});
        }
        {
        auto res { expected_type{ unexpect, error_type{2} }.and_then(f) };
        constexpr_test( std::same_as<decltype(res), expected_type>);
        constexpr_test( res == unexpected{error_type(2)});
        }
      return {};
      };
      
    result |= run_consteval_test_dual<value_type_non_void_list,error_type_list>(fn_tmpl);
    result |= run_consteval_test_dual<value_type_non_void_list,error_type_list>(fn_tmpl);
    };
    
  "expected and_then void"_test = [&]
    {
    auto fn_tmpl =
      []<typename error_type>
        (error_type const *) -> metatests::test_result
      {
      using expected_type = expected<void,error_type>;
      auto f = []() noexcept { return expected_type{in_place}; };
        {
        expected_type const ex{ in_place };
        auto res { ex.and_then(f) };
        constexpr_test( std::same_as<decltype(res), expected_type>);
        constexpr_test( res.has_value() );
        }
        {
        auto res { expected_type{ in_place }.and_then(f) };
        constexpr_test( std::same_as<decltype(res), expected_type>);
        constexpr_test( res.has_value() );
        }
        {
        auto res { expected_type{ unexpect, error_type{2} }.and_then(f) };
        constexpr_test( std::same_as<decltype(res), expected_type>);
        constexpr_test( res == unexpected{error_type(2)});
        }
      return {};
      };
    result |= run_consteval_test<error_type_list>(fn_tmpl);
    result |= run_consteval_test<error_type_list>(fn_tmpl);
    };
    
  "expected or_else"_test = [&]
    {
    auto fn_tmpl =
      []<typename value_type, typename error_type>
        ( value_type const *, error_type const *) -> metatests::test_result
      {
      using expected_type = expected<value_type,error_type>;
      auto f = [](error_type v) noexcept { return expected_type{ unexpect, ++v}; };
        {
        auto res { expected_type{ unexpect, error_type{2} }.or_else(f) };
        constexpr_test( std::same_as<decltype(res), expected_type>);
        constexpr_test( res == unexpected(error_type{3}));
        }
        {
        auto res { expected_type{ in_place, value_type{2} }.or_else(f) };
        constexpr_test( std::same_as<decltype(res), expected_type>);
        constexpr_test( res == value_type(2) );
        }
      return {};
      };
      
    result |= run_consteval_test_dual<value_type_non_void_list,error_type_list>(fn_tmpl);
    result |= run_consteval_test_dual<value_type_non_void_list,error_type_list>(fn_tmpl);
    };
    
  "expected transform"_test = [&]
    {
    auto fn_tmpl =
      []<typename value_type, typename error_type>
        ( value_type const *, error_type const *) -> metatests::test_result
      {
      using expected_type = expected<value_type,error_type>;
      auto f = [](auto && v) noexcept { value_type vx{v}; return ++vx; };
        {
        expected_type const ex{ in_place, value_type{2} };
        auto res { ex.transform(f) };
        constexpr_test( std::same_as<decltype(res), expected_type>);
        constexpr_test( res == value_type{3});
        }
        {
        auto res { expected_type{ in_place, value_type{2} }.transform(f) };
        constexpr_test( std::same_as<decltype(res), expected_type>);
        constexpr_test( res == value_type{3});
        }
        {
        expected_type const ex{ unexpect, error_type{2} };
        auto res { ex.transform(f) };
        constexpr_test( std::same_as<decltype(res), expected_type>);
        constexpr_test( res == unexpected{error_type(2)});
        }
        {
        auto res { expected_type{ unexpect, error_type{2} }.transform(f) };
        constexpr_test( std::same_as<decltype(res), expected_type>);
        constexpr_test( res == unexpected{error_type(2)});
        }
      return {};
      };
      
    result |= run_consteval_test_dual<value_type_non_void_list,error_type_list>(fn_tmpl);
    result |= run_consteval_test_dual<value_type_non_void_list,error_type_list>(fn_tmpl);
    };
    
  "expected transform void"_test = [&]
    {
    auto fn_tmpl =
      []<typename error_type>
        (error_type const *) -> metatests::test_result
      {
      using expected_type = expected<void,error_type>;
      auto f = []() noexcept {};
        {
        expected_type const ex{ in_place };
        auto res { ex.transform(f) };
        constexpr_test( std::same_as<decltype(res), expected_type>);
        constexpr_test( res.has_value());
        }
        {
        auto res { expected_type{ in_place }.transform(f) };
        constexpr_test( std::same_as<decltype(res), expected_type>);
        constexpr_test( res.has_value() );
        }
        {
        expected_type const ex{ unexpect, error_type{2} };
        auto res { ex.transform(f) };
        constexpr_test( std::same_as<decltype(res), expected_type>);
        constexpr_test( res == unexpected{error_type(2)});
        }
        {
        auto res { expected_type{ unexpect, error_type{2} }.transform(f) };
        constexpr_test( std::same_as<decltype(res), expected_type>);
        constexpr_test( res == unexpected{error_type(2)});
        }
      return {};
      };
      
    result |= run_consteval_test<error_type_list>(fn_tmpl);
    result |= run_consteval_test<error_type_list>(fn_tmpl);
    };
    
  "expected transform_error"_test = [&]
    {
    auto fn_tmpl =
      []<typename value_type, typename error_type>
        ( value_type const *, error_type const *) -> metatests::test_result
      {
      using expected_type = expected<value_type,error_type>;
      auto f = [](auto v) noexcept { return ++v; };
        {
        expected_type const ex{ in_place, value_type{2} };
        auto res { ex.transform_error(f) };
        constexpr_test( std::same_as<decltype(res), expected_type>);
        constexpr_test( res == value_type{2});
        }
        {
        auto res { expected_type{ in_place, value_type{2} }.transform_error(f) };
        constexpr_test( std::same_as<decltype(res), expected_type>);
        constexpr_test( res == value_type{2});
        }
        {
        expected_type const ex{ unexpect, error_type{2} };
        auto res { ex.transform_error(f) };
        constexpr_test( std::same_as<decltype(res), expected_type>);
        constexpr_test( res == unexpected{error_type(3)});
        }
        {
        auto res { expected_type{ unexpect, error_type{2} }.transform_error(f) };
        constexpr_test( std::same_as<decltype(res), expected_type>);
        constexpr_test( res == unexpected{error_type(3)});
        }
      return {};
      };
      
    result |= run_consteval_test_dual<value_type_non_void_list,error_type_list>(fn_tmpl);
    result |= run_consteval_test_dual<value_type_non_void_list,error_type_list>(fn_tmpl);
    };
  }
}

int main()
{
  test_result result;
  unexpected_test::do_test(result);
  expected_test::do_test(result);

  return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
