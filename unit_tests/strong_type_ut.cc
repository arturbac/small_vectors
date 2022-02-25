#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include <strong_type.h>

struct test_tag : public strong_type_default_traits{};

using test_types = boost::mpl::list<
  uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t
  >;

#define CONSTEXPR_TEST( a ) \
  if( !(a) ) return false
//----------------------------------------------------------------------------------------------------------------------
template<typename value_type>
consteval auto test_strong_type_basic()
  {
  using test_type = strong_type<value_type, test_tag>;
  test_type tst{ value_type{0x55}};
  CONSTEXPR_TEST(*tst == 0x55);
  CONSTEXPR_TEST(tst.value() == 0x55);
  
  test_type tst2 { tst };
  CONSTEXPR_TEST(*tst2 == 0x55);
  
  test_type tst3{ value_type{0x15}};
  CONSTEXPR_TEST(*tst3 == 0x15);
  tst3 = tst2;
  CONSTEXPR_TEST(*tst3 == 0x55);
  
  test_type tst4 { std::numeric_limits<value_type>::max() };
  CONSTEXPR_TEST( *tst4 == std::numeric_limits<value_type>::max() );
  tst3 = std::move(tst4);
  CONSTEXPR_TEST( *tst3 == std::numeric_limits<value_type>::max() );
  return true;
  }
  
BOOST_AUTO_TEST_CASE_TEMPLATE(strong_type_basic, value_type, test_types) 
{
  constexpr auto test_result(test_strong_type_basic<value_type>());
  static_assert(test_result);
  BOOST_TEST(test_result);
}
//----------------------------------------------------------------------------------------------------------------------
template<typename value_type>
consteval auto test_strong_type_arithemtic_base()
  {
  using test_type = strong_type<value_type, test_tag>;

  test_type tst{ value_type{0x55}};

  CONSTEXPR_TEST( *++tst == 0x56 );
  CONSTEXPR_TEST( *tst++ == 0x56 );
  CONSTEXPR_TEST( *tst == 0x57 );
  
  CONSTEXPR_TEST( *--tst == 0x56 );
  CONSTEXPR_TEST( *tst-- == 0x56 );
  CONSTEXPR_TEST( *tst == 0x55 );
  
  CONSTEXPR_TEST( *(tst + test_type(1)) == 0x56 );
  CONSTEXPR_TEST( *(tst - test_type(1)) == 0x54 );
  
  tst = test_type{ value_type{13}};
  CONSTEXPR_TEST( *(tst * test_type(2)) == 26 );
  
  tst = test_type{ value_type{44}};
  CONSTEXPR_TEST( *(tst / test_type(2)) == 22 );
  
  tst = test_type{ value_type{22}};
  CONSTEXPR_TEST( *(tst % test_type(10)) == 2 );
  
  tst = test_type{ value_type{0x55}};
  CONSTEXPR_TEST( *(tst += test_type(1)) == 0x56 );
  
  tst = test_type{ value_type{0x55}};
  CONSTEXPR_TEST( *(tst -= test_type(1)) == 0x54 );
  
  tst = test_type{ value_type{13}};
  CONSTEXPR_TEST( *(tst *= test_type(2)) == 26 );
  
  tst = test_type{ value_type{44}};
  CONSTEXPR_TEST( *(tst /= test_type(2)) == 22 );
  
  tst = test_type{ value_type{22}};
  CONSTEXPR_TEST( *(tst %= test_type(10)) == 2 );
  
  return true;
  }
  
BOOST_AUTO_TEST_CASE_TEMPLATE(strong_type_arithemtic_base, value_type, test_types) 
{
  constexpr auto test_result(test_strong_type_arithemtic_base<value_type>());
  static_assert(test_result);
  BOOST_TEST(test_result);
}
//----------------------------------------------------------------------------------------------------------------------
template<typename value_type>
consteval auto test_strong_strong_type_comparision()
  {
  using test_type = strong_type<value_type, test_tag>;
  
  test_type tst0{ value_type{0x55}};
  test_type tst1{ value_type{0x55}};
  test_type tst2{ value_type{0x56}};
  
  CONSTEXPR_TEST( tst0 == tst0 );
  CONSTEXPR_TEST( tst0 == tst1 );
  CONSTEXPR_TEST( !(tst0 == tst2) );
  
  CONSTEXPR_TEST( !(tst0 != tst0) );
  CONSTEXPR_TEST( !(tst0 != tst1) );
  CONSTEXPR_TEST( tst0 != tst2 );
  
  CONSTEXPR_TEST( (tst0 <=> tst0) == std::strong_ordering::equal );
  CONSTEXPR_TEST( !(tst0 < tst0) );
  CONSTEXPR_TEST( !(tst0 < tst1) );
  CONSTEXPR_TEST(  (tst0 < tst2) );
  CONSTEXPR_TEST( (tst0 <=> tst2) == std::strong_ordering::less );
  CONSTEXPR_TEST( !(tst2 < tst0) );
  
  CONSTEXPR_TEST( (tst0 <= tst0) );
  CONSTEXPR_TEST( (tst0 <= tst1) );
  CONSTEXPR_TEST( (tst0 <= tst2) );
  CONSTEXPR_TEST( !(tst2 <= tst0) );
  
  CONSTEXPR_TEST( !(tst0 > tst0) );
  CONSTEXPR_TEST( !(tst0 > tst1) );
  CONSTEXPR_TEST( !(tst0 > tst2) );
  CONSTEXPR_TEST(  (tst2 > tst0) );
  CONSTEXPR_TEST( (tst2 <=> tst0) == std::strong_ordering::greater );
  
  CONSTEXPR_TEST(  (tst0 >= tst0) );
  CONSTEXPR_TEST(  (tst0 >= tst1) );
  CONSTEXPR_TEST( !(tst0 >= tst2) );
  CONSTEXPR_TEST(  (tst2 >= tst0) );
  
  CONSTEXPR_TEST( (tst1 <=> tst2) == std::strong_ordering::less );
  CONSTEXPR_TEST( (tst1 <=> tst0) == std::strong_ordering::equal );
  
  return true;
  }
  
BOOST_AUTO_TEST_CASE_TEMPLATE(strong_type_comparision, value_type, test_types) 
{
  constexpr auto test_result(test_strong_strong_type_comparision<value_type>());
  static_assert(test_result);
  BOOST_TEST(test_result);
}
//----------------------------------------------------------------------------------------------------------------------
template<typename value_type>
consteval auto test_strong_strong_type_binary()
  {
  using test_type = strong_type<value_type, test_tag>;
                          test_type tst0{ value_type{0b01010101}};
                          test_type tst1{ value_type{0b00111100}};
  CONSTEXPR_TEST( (tst0 ^ tst1) == test_type{ value_type{0b01101001}} );
  CONSTEXPR_TEST( (tst0 | tst1) == test_type{ value_type{0b01111101}} );
  CONSTEXPR_TEST( (tst0 & tst1) == test_type{ value_type{0b00010100}} );
  
  CONSTEXPR_TEST( (tst0 >> 1) == test_type{ value_type{0b00101010}} );
  CONSTEXPR_TEST( (tst0 >> test_type{ value_type{1}}) == test_type{ value_type{0b00101010}} );
  
  tst0 = test_type{ value_type{0b00101010}};
  CONSTEXPR_TEST( (tst0 << 1) == test_type{ value_type{0b01010100}} );
  CONSTEXPR_TEST( (tst0 << test_type{ value_type{1}}) == test_type{ value_type{0b01010100}} );
  
  tst0 = test_type{ value_type{0b01010101}};
  CONSTEXPR_TEST( (tst0 ^= tst1) == test_type{ value_type{0b01101001}} );
  
  tst0 = test_type{ value_type{0b01010101}};
  CONSTEXPR_TEST( (tst0 |= tst1) == test_type{ value_type{0b01111101}} );
  
  tst0 = test_type{ value_type{0b01010101}};
  CONSTEXPR_TEST( (tst0 &= tst1) == test_type{ value_type{0b00010100}} );
  
  tst0 = test_type{ value_type{0b01010101}};
  CONSTEXPR_TEST( (tst0 >>= test_type{ value_type{1}}) == test_type{ value_type{0b00101010}} );
  
  tst0 = test_type{ value_type{0b00101010}};
  CONSTEXPR_TEST( (tst0 <<= test_type{ value_type{1}}) == test_type{ value_type{0b01010100}} );
  
  return true;
  }
  
BOOST_AUTO_TEST_CASE_TEMPLATE(strong_type_binary, value_type, test_types) 
{
  constexpr auto test_result(test_strong_strong_type_binary<value_type>());
  static_assert(test_result);
  BOOST_TEST(test_result);
}

//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
