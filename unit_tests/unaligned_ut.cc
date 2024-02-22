#include <unit_test_core.h>

#include <utils/unaligned.h>
#include <cmath>

int main()
  {
  using namespace boost::ut;
  using namespace memutil;

  "[utils_math_unaligned]"_test = []
  {
    uint8_t store[128];

    should("test int") = [&store]
    {
      using type = int32_t;
      type testv = 0x5555aaaa;

      unaligned_store<type>(&store[1], testv);
      type result = unaligned_load<type>(&store[1]);
      expect(result == testv);
    };
    should("test double") = [&store]
    {
      using type = double;
      type testv = 0.45671974353;
      unaligned_store<type>(&store[1], testv);
      type result = unaligned_load<type>(&store[1]);
      expect(result == testv);
    };

    should("test int64_t") = [&store]
    {
      using type = int64_t;
      type testv = 0x111177775555aaaa;

      unaligned_store<type>(&store[1], testv);
      type result = unaligned_load<type>(&store[1]);
      expect(result == testv);
    };

    should("test enum") = [&store]
    {
      enum struct test_enum : uint16_t
        {
        one,
        two,
        three
        };
      using type = test_enum;
      type testv = test_enum::two;
      unaligned_store<type, 2>(&store[3], testv);
      type result = unaligned_load<type, 2>(&store[3]);
      expect(result == testv);
    };
  };
  }
