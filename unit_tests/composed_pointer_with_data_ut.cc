#include <coll/composed_pointer_with_data.h>
#include <unit_test_core.h>

using namespace coll;
using metatests::constexpr_test;
using boost::ut::expect;
using boost::ut::operator""_test;

enum struct some_enum : uint8_t { no, yes, maybe };
using type = composed_pointer_with_data<double,some_enum>;
consteval auto test_composed_pointer_with_data()
  {
  constexpr_test( type::data_mask == 0b111 );
  type x{};
  constexpr_test( x.data() == some_enum{} );
  constexpr_test( (x.data_ & type::pointer_mask) == 0 );

  return true;
  }

int main()
{

"composed_pointer_with_data_tests"_test = []
  {
  constexpr auto test_result(test_composed_pointer_with_data());
  static_assert(test_result);
  expect(test_result);
  
    {
    type x{};
    expect( (x.data() == some_enum{}) );
    expect( (x.data_ & type::pointer_mask) == 0 );
    expect( x.ptr() == nullptr );
    }

    {
    double test[2];
    type x{ &test[1] };
    expect( (x.data() == some_enum{}) );
    expect( (x.data_ & type::pointer_mask) != 0 );
    expect( x.ptr() == &test[1] );

    x.set_data( some_enum::yes);
    expect( (x.data() == some_enum::yes ) );
    expect( x.ptr() == &test[1] );

    x.set_ptr( &test[0] );
    expect( (x.data() == some_enum::yes ) );
    expect( x.ptr() == &test[0] );
    x.set_data( some_enum::maybe);
    expect( (x.data() == some_enum::maybe ) );
    expect( x.ptr() == &test[0] );
    }
  };

}
