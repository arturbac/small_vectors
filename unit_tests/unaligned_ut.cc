#include <unit_test_core.h>

#include <small_vectors/utils/unaligned.h>
#include <cmath>

using traits_list = metatests::type_list<uint8_t, std::byte>;
using namespace metatests;

int main()
  {
  using namespace boost::ut;
  using namespace small_vectors::memutil;

  metatests::test_result result;

  "test_int"_test = [&]
  {
    auto fn_tmpl = []<typename value_type>(value_type const *) -> metatests::test_result
    {
      value_type store[128];
      using type = int32_t;
      type testv = 0x5555aaaa;

      unaligned_store<type>(&store[1], testv);
      type r = unaligned_load<type>(&store[1]);
      constexpr_test(r == testv);
      return {};
    };
    result |= run_constexpr_test<traits_list>(fn_tmpl);
    result |= run_consteval_test<traits_list>(fn_tmpl);
  };

  "test double"_test = [&]
  {
    auto fn_tmpl = []<typename value_type>(value_type const *) -> metatests::test_result
    {
      value_type store[128];
      using type = double;
      type testv = 0.45671974353;
      unaligned_store<type>(&store[1], testv);
      type r = unaligned_load<type>(&store[1]);
      constexpr_test(r == testv);
      return {};
    };
    result |= run_constexpr_test<traits_list>(fn_tmpl);
    result |= run_consteval_test<traits_list>(fn_tmpl);
  };

  "test int64_t"_test = [&]
  {
    auto fn_tmpl = []<typename value_type>(value_type const *) -> metatests::test_result
    {
      value_type store[128];
      using type = int64_t;
      type testv = 0x111177775555aaaa;

      unaligned_store<type>(&store[1], testv);
      type r = unaligned_load<type>(&store[1]);
      constexpr_test(r == testv);
      return {};
    };
    result |= run_constexpr_test<traits_list>(fn_tmpl);
    result |= run_consteval_test<traits_list>(fn_tmpl);
  };

  "test enum"_test = [&]
  {
    enum struct test_enum : uint16_t
      {
      one,
      two,
      three
      };
    auto fn_tmpl = []<typename value_type>(value_type const *) -> metatests::test_result
    {
      value_type store[128];

      using type = test_enum;
        {
        type testv = test_enum::three;
        unaligned_store<type>(&store[3], testv);
        type r = unaligned_load<type>(&store[3]);
        constexpr_test(r == testv);
        }
        {
        type testv = test_enum::two;
        unaligned_store<type, 2>(&store[3], testv);
        type r = unaligned_load<type, 2>(&store[3]);
        constexpr_test(r == testv);
        }
      return {};
    };
    result |= run_constexpr_test<traits_list>(fn_tmpl);
    result |= run_consteval_test<traits_list>(fn_tmpl);
  };
  }
