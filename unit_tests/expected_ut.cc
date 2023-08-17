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

using value_type_list = metatests::type_list<int32_t,uint32_t,void,non_trivial,non_trivial_ptr>;
enum struct test_error { no_error, error1, error2 };
using error_type_list = metatests::type_list<int32_t,test_error,non_trivial,non_trivial_ptr>;
using namespace cxx23;

static_assert(concepts::not_expected<int>);
static_assert(concepts::not_expected<void>);
static_assert(concepts::not_expected<non_trivial>);
static_assert(!concepts::not_expected<expected<void,int>>);
static_assert(!concepts::not_expected<expected<int,int>>);
static_assert(!concepts::not_expected<expected<non_trivial,int>>);

namespace expected_return_type_check
{
  consteval bool test_ref()
    {
    expected<int,int> v{};
    expected<void,int> v2{};
    return std::is_same_v<decltype(v.value()), int &> && std::is_same_v<decltype(v2.value()), void>;
    }
  static_assert(test_ref());
  
  consteval bool test_const_ref()
    {
    const expected<int,int> v{};
    return std::is_same_v<decltype(v.value()), int const &>;
    }
  static_assert(test_const_ref());
  
  consteval bool test_rref()
    {
    return std::is_same_v<decltype(expected<int,int>{}.value()), int &&>
      && std::is_same_v<decltype(expected<void,int>{}.value()), void>;
    }
  static_assert(test_rref());
  
  consteval bool test_const_rref()
    {
    return std::is_same_v<decltype(std::add_const_t<expected<int,int>>{}.value()), int const &&>;
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
        unexpected_type un{ std::in_place, static_cast<error_type>(2)};
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
  "expected"_test = [&]
    {
    auto fn_tmpl =
      []<typename value_type, typename error_type>
        ( value_type const *, error_type const *) -> metatests::test_result
      {
      
      return {};
      };
    result |= run_consteval_test_dual<value_type_list,error_type_list>(fn_tmpl);
    result |= run_constexpr_test_dual<value_type_list,error_type_list>(fn_tmpl);
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
