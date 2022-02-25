#include <utils/endian.h>
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#define CONSTEXPR_TEST( a ) \
  if( !(a) ) throw 
//-----------------------------------------------------------------------------------------------------

consteval auto test_utils_byteswap()
  {
  CONSTEXPR_TEST( cxx23::byteswap(0) == 0 );
  CONSTEXPR_TEST( cxx23::byteswap(uint16_t(0xa51f)) == 0x1fa5);
  CONSTEXPR_TEST( cxx23::byteswap(0xffa51fe0) == 0xe01fa5ff );
  CONSTEXPR_TEST( cxx23::byteswap(0xffa51fe000003a01ull) == 0x013a0000e01fa5ffull );
  
  double src { cxx20::bit_cast<double>(0xffa51fe000003a01ull)};
  double res { cxx23::byteswap(src) };
  uint64_t res_dbl_byes{ cxx20::bit_cast<uint64_t>(res)};
  CONSTEXPR_TEST( res_dbl_byes == 0x013a0000e01fa5ffull );
  return true;
  }
  
BOOST_AUTO_TEST_CASE( utils_bytswap )
  {
  constexpr auto test_result(test_utils_byteswap());
  static_assert(test_result);
  BOOST_TEST(test_result);
  
  BOOST_TEST( cxx23::byteswap(0) == 0 );
  BOOST_TEST( cxx23::byteswap(uint16_t(0xa51f)) == 0x1fa5);
  BOOST_TEST( cxx23::byteswap(0xffa51fe0) == 0xe01fa5ff );
  BOOST_TEST( cxx23::byteswap(0xffa51fe000003a01ull) == 0x013a0000e01fa5ffull );
  
  double src { cxx20::bit_cast<double>(0xffa51fe000003a01ull)};
  double res { cxx23::byteswap(src) };
  uint64_t res_dbl_byes{ cxx20::bit_cast<uint64_t>(res)};
  BOOST_TEST( res_dbl_byes == 0x013a0000e01fa5ffull );
  }
