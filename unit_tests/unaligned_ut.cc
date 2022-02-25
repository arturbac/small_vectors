#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <utils/unaligned.h>
#include <cmath>

namespace memutil
{
BOOST_AUTO_TEST_CASE( utils_math_unaligned )
{
  uint8_t store[128];
  {
  using type = int32_t;
  type test = 0x5555aaaa;
  
  unaligned_store<type>(&store[1], test );
  type result = unaligned_load<type>(&store[1]);
  BOOST_TEST( result == test );
  }
  {
  using type = double;
  type test = 0.45671974353;
  unaligned_store<type>(&store[1], test );
  type result = unaligned_load<type>(&store[1]);
  BOOST_TEST( result == test );
  }
{
  using type = int64_t;
  type test = 0x111177775555aaaa;
  
  unaligned_store<type>(&store[1], test );
  type result = unaligned_load<type>(&store[1]);
  BOOST_TEST( result == test );
  }
  {
  enum struct test_enum : uint16_t { one, two, three };
  using type = test_enum;
  type test = test_enum::two;
  unaligned_store<type,2>(&store[3], test );
  type result = unaligned_load<type,2>(&store[3]);
  BOOST_TEST( (result == test) );
  }
}
}
