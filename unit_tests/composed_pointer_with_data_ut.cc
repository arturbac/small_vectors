#include <coll/composed_pointer_with_data.h>

#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>

#define CONSTEXPR_TEST( a ) \
  if( !(a) ) throw
    

namespace coll
{

enum struct some_enum : uint8_t { no, yes, maybe };
using type = composed_pointer_with_data<double,some_enum>;
consteval auto test_composed_pointer_with_data()
  {
  CONSTEXPR_TEST( type::data_mask == 0b111 );
  type x{};
  CONSTEXPR_TEST( x.data() == some_enum{} );
  CONSTEXPR_TEST( (x.data_ & type::pointer_mask) == 0 );
 
  return true;
  }
  
BOOST_AUTO_TEST_CASE( composed_pointer_with_data_tests )
  {
  constexpr auto test_result(test_composed_pointer_with_data());
  static_assert(test_result);
  BOOST_TEST(test_result);
  
  {
  type x{};
  BOOST_TEST( (x.data() == some_enum{}) );
  BOOST_TEST( (x.data_ & type::pointer_mask) == 0 );
  BOOST_TEST( x.ptr() == nullptr );
  }
  
  {
  double test[2];
  type x{ &test[1] };
  BOOST_TEST( (x.data() == some_enum{}) );
  BOOST_TEST( (x.data_ & type::pointer_mask) != 0 );
  BOOST_TEST( x.ptr() == &test[1] );
  
  x.set_data( some_enum::yes);
  BOOST_TEST( (x.data() == some_enum::yes ) );
  BOOST_TEST( x.ptr() == &test[1] );
  
  x.set_ptr( &test[0] );
  BOOST_TEST( (x.data() == some_enum::yes ) );
  BOOST_TEST( x.ptr() == &test[0] );
  x.set_data( some_enum::maybe);
  BOOST_TEST( (x.data() == some_enum::maybe ) );
  BOOST_TEST( x.ptr() == &test[0] );
  }
  }

}
