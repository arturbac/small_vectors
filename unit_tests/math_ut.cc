#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include <utils_typedef.h>
#include <utils/math.h>
#include <cpuext/sse2.h>

constexpr bool consteval_throw_if_fails(auto x)
  {
  throw;
  return false;
  }

#define CONSTEXPR_TEST(a) \
  if(!(a))                \
  throw
// return consteval_throw_if_fails(a)

using traits_list
  = boost::mpl::list<float, double, int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t>;
using float_list = boost::mpl::list<float, double>;

//-----------------------------------------------------------------------------------------------------
template<typename value_type>
consteval auto test_utils_math_abs()
  {
  CONSTEXPR_TEST(math::abs<value_type>(0) == 0);
  CONSTEXPR_TEST(math::abs<value_type>(1) == 1);
  CONSTEXPR_TEST(math::abs(std::numeric_limits<value_type>::max()) == std::numeric_limits<value_type>::max());

  if constexpr(std::is_signed_v<value_type>)
    {
    CONSTEXPR_TEST(
      math::abs(-(std::numeric_limits<value_type>::max() - 1)) == std::numeric_limits<value_type>::max() - 1
    );
    CONSTEXPR_TEST(math::abs<value_type>(-0) == 0);
    CONSTEXPR_TEST(math::abs<value_type>(-1) == 1);
    }

  return true;
  }

BOOST_AUTO_TEST_CASE_TEMPLATE(utils_math_abs, value_type, traits_list)
  {
  constexpr auto test_result(test_utils_math_abs<value_type>());
  static_assert(test_result);
  BOOST_TEST(test_result);
  }

//-----------------------------------------------------------------------------------------------------
consteval auto test_utils_math_bits_reverse()
  {
    {
    uint8_t val = 0b10100010;
    uint8_t exp = 0b01000101;
    uint8_t res = math::reverse_bits(val);
    CONSTEXPR_TEST(res == exp);
    }
    {
    uint8_t val = 0b10000000;
    uint8_t exp = 0b00000001;
    uint8_t res = math::reverse_bits(val);
    CONSTEXPR_TEST(res == exp);
    }
    {
    uint8_t val = 0b10100001;
    uint8_t exp = 0b10000101;
    uint8_t res = math::reverse_bits(val);
    CONSTEXPR_TEST(res == exp);
    }
    {
    uint8_t val = 0b00000000;
    uint8_t exp = 0b00000000;
    uint8_t res = math::reverse_bits(val);
    CONSTEXPR_TEST(res == exp);
    }
    {
    uint8_t val = 0b11111111;
    uint8_t exp = 0b11111111;
    uint8_t res = math::reverse_bits(val);
    CONSTEXPR_TEST(res == exp);
    }
  return true;
  }

BOOST_AUTO_TEST_CASE(utils_math_bits_reverse)
  {
  constexpr auto test_result(test_utils_math_bits_reverse());
  static_assert(test_result);
  BOOST_TEST(test_result);
  }

using sse2::m128d_t;
//-----------------------------------------------------------------------------------------------------
#if !defined(__clang__)
#define outcome_constexpr constexpr
#else
#define outcome_constexpr const
#endif
//-----------------------------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(cpuext_sse2_haddpd)
  {
    {
    outcome_constexpr m128d_t cnst_res = sse2::hadd_pd(m128d_t{}, m128d_t{});
    m128d_t const rtres = _mm_hadd_pd(m128d_t{}, m128d_t{});
    BOOST_TEST(cnst_res[0] == rtres[0]);
    BOOST_TEST(cnst_res[1] == rtres[1]);
    }
    {
    double constexpr x0 = 10.1234;
    double constexpr y0 = -10.791234;
    double constexpr x1 = 20.41234;
    double constexpr y1 = 40.61234;

    outcome_constexpr m128d_t cnst_res = sse2::hadd_pd(m128d_t{x0, x1}, m128d_t{y0, y1});
    m128d_t const rtres = _mm_hadd_pd(m128d_t{x0, x1}, m128d_t{y0, y1});
    BOOST_TEST(cnst_res[0] == rtres[0]);
    BOOST_TEST(cnst_res[1] == rtres[1]);
    }
  }

//-----------------------------------------------------------------------------------------------------
template<typename value_type>
consteval auto test_math_consteval_sqrt()
  {
    {
    constexpr value_type sqrt_ce = math::consteval_sqrt<value_type>(value_type(0));  // 38.1635927291968
    CONSTEXPR_TEST(sqrt_ce < value_type(0.00001));
    }
    {
    constexpr value_type expected = value_type(1.4142135623731);
    constexpr value_type sqrt_ce = math::consteval_sqrt<value_type>(2);  // 1.4142135623731
    CONSTEXPR_TEST(math::abs<value_type>(sqrt_ce - expected) < value_type(0.00001));
    }
    {
    constexpr value_type expected = value_type(38.1635927291968);
    constexpr value_type sqrt_ce = math::consteval_sqrt<value_type>(value_type(1456.45981));  // 38.1635927291968
    CONSTEXPR_TEST(math::abs<value_type>(sqrt_ce - expected) < value_type(0.00001));
    }

  return true;
  }

BOOST_AUTO_TEST_CASE_TEMPLATE(math_consteval_sqrt, value_type, float_list)
  {
  constexpr auto test_result(test_math_consteval_sqrt<value_type>());
  static_assert(test_result);
  BOOST_TEST(test_result);
  }

//-----------------------------------------------------------------------------------------------------
template<typename value_type>
consteval auto test_math_consteval_floor()
  {
    {
    constexpr value_type expected = 0;
    constexpr value_type result = math::floor<value_type>(value_type(0.00000043821));
    CONSTEXPR_TEST(result == expected);
    }
    {
    constexpr value_type expected = 0;
    constexpr value_type result = math::floor<value_type>(value_type(0.43821));
    CONSTEXPR_TEST(result == expected);
    }
    {
    constexpr value_type expected = 1;
    constexpr value_type result = math::floor<value_type>(value_type(1.43821));
    CONSTEXPR_TEST(result == expected);
    }
    {
    constexpr value_type expected = 1999999;
    constexpr value_type result = math::floor<value_type>(value_type(1999999.43821));
    CONSTEXPR_TEST(result == expected);
    }
    {
    constexpr value_type expected = -2000000;
    constexpr value_type result = math::floor<value_type>(value_type(-1999999.43821));
    CONSTEXPR_TEST(result == expected);
    }
    {
    constexpr value_type expected = -1;
    constexpr value_type result = math::floor<value_type>(value_type(-0.43821));
    CONSTEXPR_TEST(result == expected);
    }
    {
    constexpr value_type expected = -1;
    constexpr value_type result = math::floor<value_type>(value_type(-0.0000043821));
    CONSTEXPR_TEST(result == expected);
    }
  return true;
  }

BOOST_AUTO_TEST_CASE_TEMPLATE(math_consteval_floor, value_type, float_list)
  {
  constexpr auto test_result(test_math_consteval_floor<value_type>());
  static_assert(test_result);
  BOOST_TEST(test_result);
  }

//-----------------------------------------------------------------------------------------------------
template<typename value_type>
consteval auto test_math_consteval_ceil()
  {
    {
    constexpr value_type expected = 1;
    constexpr value_type result = math::ceil<value_type>(value_type(0.00000043821));
    CONSTEXPR_TEST(result == expected);
    }
    {
    constexpr value_type expected = 1;
    constexpr value_type result = math::ceil<value_type>(value_type(0.43821));
    CONSTEXPR_TEST(result == expected);
    }
    {
    constexpr value_type expected = 2;
    constexpr value_type result = math::ceil<value_type>(value_type(1.43821));
    CONSTEXPR_TEST(result == expected);
    }
    {
    constexpr value_type expected = 2000000;
    constexpr value_type result = math::ceil<value_type>(value_type(1999999.43821));
    CONSTEXPR_TEST(result == expected);
    }
    {
    constexpr value_type expected = -1999999;
    constexpr value_type result = math::ceil<value_type>(value_type(-1999999.43821));
    CONSTEXPR_TEST(result == expected);
    }
    {
    constexpr value_type expected = 0;
    constexpr value_type result = math::ceil<value_type>(value_type(-0.43821));
    CONSTEXPR_TEST(result == expected);
    }
    {
    constexpr value_type expected = 0;
    constexpr value_type result = math::ceil<value_type>(value_type(-0.0000043821));
    CONSTEXPR_TEST(result == expected);
    }
  return true;
  }

BOOST_AUTO_TEST_CASE_TEMPLATE(math_consteval_ceil, value_type, float_list)
  {
  constexpr auto test_result(test_math_consteval_ceil<value_type>());
  static_assert(test_result);
  BOOST_TEST(test_result);
  }
