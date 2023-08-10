#include <coll/ranges/accumulate.h>
#include <utils/strong_type.h>
#include <unit_test_core.h>

struct test_tag : public strong_type_default_traits{};
using test_user_type = strong_type<int64_t, test_tag>;
using test_type_list = metatests::type_list<int8_t,uint8_t,uint32_t, int64_t, size_t,test_user_type>;
using namespace metatests;
using boost::ut::operator""_test;
using boost::ut::eq;
using boost::ut::neq;
//----------------------------------------------------------------------------------------------------------------------
int main()
{
  test_result result;
  "test_accumulate"_test = [&result]
    {
    auto fn_tmpl =
      []<typename value_type>
        ( value_type const * ) -> metatests::test_result
      {
      using limits = std::numeric_limits<value_type>;
        {
        constexpr std::array<value_type,1> range{};
        constexpr_test(value_type(1) == coll::ranges::accumulate(range, value_type(1)));
        }
        {
        constexpr std::array<value_type,10> range
          {
          limits::lowest(), value_type(0), limits::max()/value_type(4), value_type(1)
          };
        constexpr auto sum(limits::lowest()+  value_type(0)+ limits::max()/value_type(4)+ value_type(1));
        constexpr_test(sum == coll::ranges::accumulate(range, value_type(0)));
        }
        {
        constexpr std::array<value_type,10> range
          {
          value_type(0), limits::max()/value_type(4), value_type(1)
          };
        constexpr auto sum(value_type(0)+ limits::max()/value_type(4)+ value_type(1));
        constexpr_test(sum*value_type(2) == coll::ranges::accumulate(range, value_type(0),
                                                       [](value_type init, value_type obj) noexcept
                                                       { return value_type(init + obj*value_type(2)); } ));
        }
      return {};
      };
    result |= run_consteval_test<test_type_list>(fn_tmpl);
    result |= run_constexpr_test<test_type_list>(fn_tmpl);
    };
}
