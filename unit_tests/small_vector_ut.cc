#include <coll/small_vector.h>
#include <unit_test_core.h>

#include <iostream>

namespace coll
{
using detail::small_vector_storage_type;
using enum detail::small_vector_storage_type;
using enum detail::vector_outcome_e;

using constexpr_size_type_traits_list = boost::mpl::list<uint8_t,uint16_t,uint32_t,uint64_t>;
//---------------------------------------------------------------------------------------------------------------------
template<typename size_type,typename value_type>
inline void dump_storage_info()
  {
  using st = detail::small_vector_storage<value_type, size_type>;
  std::cout << "size_type " << sizeof(size_type) << " value_type " << sizeof(value_type) << " "
            << "buffered_capacity " << unsigned(st::buffered_capacity) << "\n"
            << "struct size " << sizeof(st) << " storage size " << sizeof(typename st::storage_type) << "\n\n" ;
  }
template<typename size_type,typename value_type>
inline void dump_estorage_info()
  {
  using st = detail::small_vector_storage<value_type, size_type, 0u>;
  std::cout << "size_type " << sizeof(size_type) << " value_type " << sizeof(value_type) << " "
            << "buffered_capacity " << unsigned(st::buffered_capacity) << "\n"
            << "struct size " << sizeof(st) << " storage size " << sizeof(typename st::storage_type) << "\n\n" ;
  }
//---------------------------------------------------------------------------------------------------------------------
constexpr bool equal( auto const & result, auto const & expected ) noexcept
  {
  return std::equal(begin(result),end(result),
                    begin(expected), end(expected));
  }
  
//---------------------------------------------------------------------------------------------------------------------
template<typename size_type,typename value_type>
consteval bool test_storage_info(std::size_t e_struct_size, std::size_t e_buff_capacity)
  {
  using st = detail::small_vector_storage<value_type, size_type>;
  CONSTEXPR_TEST( sizeof(st) == e_struct_size);
  CONSTEXPR_TEST( st::buffered_capacity == e_buff_capacity);
  return true;
  }

static_assert(sizeof(void *) ==8, "thise tests are written for 64bit platform");

consteval bool consteval_test_small_vector_storage_size()
  {
  test_storage_info<uint8_t, uint8_t>(16,14);
  test_storage_info<uint16_t, uint8_t>(16,13);
  test_storage_info<uint32_t, uint8_t>(24,19);
  test_storage_info<uint64_t, uint8_t>(32,23);
  
  test_storage_info<uint8_t,  uint16_t>(16,7);
  test_storage_info<uint16_t, uint16_t>(16,6);
  test_storage_info<uint32_t, uint16_t>(24,9);
  test_storage_info<uint64_t, uint16_t>(32,11);
  
  test_storage_info<uint8_t, uint32_t>(16,3);
  test_storage_info<uint16_t, uint32_t>(16,3);
  test_storage_info<uint32_t, uint32_t>(24,4);
  test_storage_info<uint64_t, uint32_t>(32,5);
  
  test_storage_info<uint8_t, uint64_t>(16,1);
  test_storage_info<uint16_t, uint64_t>(16,1);
  test_storage_info<uint32_t, uint64_t>(24,2);
  test_storage_info<uint64_t, uint64_t>(32,2);
  
  static_assert( sizeof(non_trivial) == 4 );
  test_storage_info<uint8_t, non_trivial>(16,3);
  test_storage_info<uint16_t, non_trivial>(16,3);
  test_storage_info<uint32_t, non_trivial>(24,4);
  test_storage_info<uint64_t, non_trivial>(32,5);
  return true;
  }
  
struct struct_3_byte{ uint8_t tst[3]; };
struct struct_5_byte{ uint8_t tst[5]; };

BOOST_AUTO_TEST_CASE( small_vector_storage_size )
{
  BOOST_TEST(consteval_test_small_vector_storage_size());
#if 0
  std::cout << "V uint8_t" << std::endl;
  dump_storage_info<uint8_t, uint8_t>();
  dump_storage_info<uint16_t, uint8_t>();
  dump_storage_info<uint32_t, uint8_t>();
  dump_storage_info<uint64_t, uint8_t>();
  
  std::cout << "V uint16_t" << std::endl;
  dump_storage_info<uint8_t,  uint16_t>();
  dump_storage_info<uint16_t, uint16_t>();
  dump_storage_info<uint32_t, uint16_t>();
  dump_storage_info<uint64_t, uint16_t>();
  
  std::cout << "V uint32_t" << std::endl;
  dump_storage_info<uint8_t, uint32_t>();
  dump_storage_info<uint16_t, uint32_t>();
  dump_storage_info<uint32_t, uint32_t>();
  dump_storage_info<uint64_t, uint32_t>();
  
  std::cout << "V uint64_t" << std::endl;
  dump_storage_info<uint8_t, uint64_t>();
  dump_storage_info<uint16_t, uint64_t>();
  dump_storage_info<uint32_t, uint64_t>();
  dump_storage_info<uint64_t, uint64_t>();
  
  std::cout << "V struct_3_byte" << std::endl;
  dump_storage_info<uint8_t, struct_3_byte>();
  dump_storage_info<uint16_t, struct_3_byte>();
  dump_storage_info<uint32_t, struct_3_byte>();
  dump_storage_info<uint64_t, struct_3_byte>();
  
  std::cout << "V struct_5_byte" << std::endl;
  dump_storage_info<uint8_t, struct_5_byte>();
  dump_storage_info<uint16_t, struct_5_byte>();
  dump_storage_info<uint32_t, struct_5_byte>();
  dump_storage_info<uint64_t, struct_5_byte>();
  
  std::cout << "V non_trivial" << std::endl;
  dump_storage_info<uint8_t, non_trivial>();
  dump_storage_info<uint16_t, non_trivial>();
  dump_storage_info<uint32_t, non_trivial>();
  dump_storage_info<uint64_t, non_trivial>();

  std::cout << "V uint8_t" << std::endl;
  dump_estorage_info<uint8_t, uint8_t>();
  dump_estorage_info<uint16_t, uint8_t>();
  dump_estorage_info<uint32_t, uint8_t>();
  dump_estorage_info<uint64_t, uint8_t>();
#endif
}
//---------------------------------------------------------------------------------------------------------------------
template<typename value_type, typename size_type>
constexpr bool constexpr_test_small_vector_constr()
  {
    {
    constexpr auto capacity_req =at_least<value_type>(size_type(8));
    using st = small_vector<value_type, size_type, capacity_req>;
    st v;
    CONSTEXPR_TEST(size(v) == 0u);
    CONSTEXPR_TEST(empty(v));
    CONSTEXPR_TEST(begin(v) == end(v));
    CONSTEXPR_TEST(capacity(v) >= 8u );
    CONSTEXPR_TEST(data(v) != nullptr );
    }
    {
    using st = small_vector<value_type, size_type, 0>;
    st v;
    CONSTEXPR_TEST(size(v) == 0u);
    CONSTEXPR_TEST(empty(v));
    CONSTEXPR_TEST(begin(v) == end(v));
    CONSTEXPR_TEST(capacity(v) == 0u );
    CONSTEXPR_TEST(data(v) == nullptr );
    }
  return true;
  }
template<typename value_type, typename size_type>
consteval bool consteval_test_small_vector_constr()
  {
  return constexpr_test_small_vector_constr<value_type,size_type>();
  }

static volatile bool vfalse{};
static bool test_failure() { return vfalse; }

BOOST_AUTO_TEST_CASE_TEMPLATE( small_vector_storage_constr, size_type, constexpr_size_type_traits_list )
{
  BOOST_TEST( (consteval_test_small_vector_constr<uint8_t,size_type>()) );
  BOOST_TEST( (consteval_test_small_vector_constr<uint16_t,size_type>()) );
  BOOST_TEST( (consteval_test_small_vector_constr<uint32_t,size_type>()) );
  BOOST_TEST( (consteval_test_small_vector_constr<uint64_t,size_type>()) );
  BOOST_TEST( (consteval_test_small_vector_constr<struct_3_byte,size_type>()) );
  BOOST_TEST( (consteval_test_small_vector_constr<struct_5_byte,size_type>()) );
  
  BOOST_TEST( (constexpr_test_small_vector_constr<uint8_t,size_type>()) );
  BOOST_TEST( (constexpr_test_small_vector_constr<uint16_t,size_type>()) );
  BOOST_TEST( (constexpr_test_small_vector_constr<uint32_t,size_type>()) );
  BOOST_TEST( (constexpr_test_small_vector_constr<uint64_t,size_type>()) );
  BOOST_TEST( (constexpr_test_small_vector_constr<struct_3_byte,size_type>()) );
  BOOST_TEST( (constexpr_test_small_vector_constr<struct_5_byte,size_type>()) );
  BOOST_TEST( (constexpr_test_small_vector_constr<non_trivial,size_type>()) );
  BOOST_TEST( (constexpr_test_small_vector_constr<non_trivial_ptr,size_type>()) );
  BOOST_TEST( (constexpr_test_small_vector_constr<non_trivial_ptr_except,size_type>()) );
  BOOST_TEST( (constexpr_test_small_vector_constr<non_trivial_ptr_except_copy,size_type>()) );
  
  CONSTEXPR_TEST(!test_failure());
}
//---------------------------------------------------------------------------------------------------------------------
constexpr bool constexpr_test_small_vector_emplace_back()
  {
  using value_type = uint32_t;
  using st = small_vector<value_type, uint16_t, 3>;
  st v;
  CONSTEXPR_TEST(detail::growth(uint16_t(3), uint16_t(1)) == 7 );
  
  auto res = emplace_back( v, 0xf5u );
  CONSTEXPR_TEST( res == no_error );
  CONSTEXPR_TEST(size(v) == 1u);
  CONSTEXPR_TEST(!empty(v));
  CONSTEXPR_TEST(begin(v) != end(v));
  CONSTEXPR_TEST( *begin(v) == 0xf5u );
  CONSTEXPR_TEST(back(v) == 0xf5u );
  CONSTEXPR_TEST(v.active_storage() == buffered );
  
  res = emplace_back( v, 0x15u );
  {
  std::array<value_type,2> expected{0xf5u,0x15u };
  CONSTEXPR_TEST( res == no_error );
  CONSTEXPR_TEST(size(v) == 2u);
  CONSTEXPR_TEST(!empty(v));
  CONSTEXPR_TEST(begin(v) != end(v));
  CONSTEXPR_TEST(v.active_storage() == buffered );
  CONSTEXPR_TEST(equal(v,expected));
  }
  
  res = emplace_back( v, 0x25u );
  {
  std::array<value_type,3> expected{0xf5u,0x15u,0x25u};
  CONSTEXPR_TEST( res == no_error );
  CONSTEXPR_TEST(size(v) == 3u);
  CONSTEXPR_TEST(!empty(v));
  CONSTEXPR_TEST(begin(v) != end(v));
  CONSTEXPR_TEST(v.active_storage() == buffered );
  CONSTEXPR_TEST(equal(v,expected));
  }
  
  {
  res = emplace_back( v, 0x35u );
  std::array<value_type,4> expected{0xf5u,0x15u,0x25u,0x35u};
  CONSTEXPR_TEST( res == no_error );
  CONSTEXPR_TEST(v.active_storage() == dynamic );
  CONSTEXPR_TEST(size(v) == 4u);
  CONSTEXPR_TEST(!empty(v));
  CONSTEXPR_TEST(begin(v) != end(v));
  CONSTEXPR_TEST(equal(v,expected));
  }
  {
  res = emplace_back( v, 0x45u );
  std::array<value_type,5> expected{0xf5u,0x15u,0x25u,0x35u,0x45u};
  CONSTEXPR_TEST( res == no_error );
  CONSTEXPR_TEST(v.active_storage() == dynamic );
  CONSTEXPR_TEST(size(v) == 5u);
  CONSTEXPR_TEST(!empty(v));
  CONSTEXPR_TEST(begin(v) != end(v));
  CONSTEXPR_TEST(equal(v,expected));
  }
  {
  res = emplace_back( v, 0x55u );
  std::array<value_type,6> expected{0xf5u,0x15u,0x25u,0x35u,0x45u,0x55u};
  CONSTEXPR_TEST( res == no_error );
  CONSTEXPR_TEST(v.active_storage() == dynamic );
  CONSTEXPR_TEST(size(v) == 6u);
  CONSTEXPR_TEST(!empty(v));
  CONSTEXPR_TEST(begin(v) != end(v));
  CONSTEXPR_TEST(equal(v,expected));
  }
  {
  res = emplace_back( v, 1u );
  std::array<value_type,7> expected{0xf5u,0x15u,0x25u,0x35u,0x45u,0x55u,1u};
  CONSTEXPR_TEST( res == no_error );
  CONSTEXPR_TEST(v.active_storage() == dynamic );
  CONSTEXPR_TEST(size(v) == expected.size());
  CONSTEXPR_TEST(!empty(v));
  CONSTEXPR_TEST(begin(v) != end(v));
  CONSTEXPR_TEST(equal(v,expected));
  }
  {
  res = emplace_back( v, 2u );
  std::array<value_type,8> expected{0xf5u,0x15u,0x25u,0x35u,0x45u,0x55u,1u,2u};
  CONSTEXPR_TEST( res == no_error );
  CONSTEXPR_TEST(v.active_storage() == dynamic );
  CONSTEXPR_TEST(size(v) == expected.size());
  CONSTEXPR_TEST(!empty(v));
  CONSTEXPR_TEST(begin(v) != end(v));
  CONSTEXPR_TEST(equal(v,expected));
  }
  {
  res = emplace_back( v, 3u );
  std::array<value_type,9> expected{0xf5u,0x15u,0x25u,0x35u,0x45u,0x55u,1u,2u,3u};
  CONSTEXPR_TEST( res == no_error );
  CONSTEXPR_TEST(v.active_storage() == dynamic );
  CONSTEXPR_TEST(size(v) == expected.size());
  CONSTEXPR_TEST(!empty(v));
  CONSTEXPR_TEST(begin(v) != end(v));
  CONSTEXPR_TEST(equal(v,expected));
  }
  return true;
  }

consteval bool consteval_test_small_vector_emplace_back()
  { return constexpr_test_small_vector_emplace_back(); }
BOOST_AUTO_TEST_CASE( test_small_vector_emplace_back )
{
  BOOST_TEST(consteval_test_small_vector_emplace_back());
}

//---------------------------------------------------------------------------------------------------------------------

consteval bool consteval_test_small_vector_emplace_back_0()
  {
  using value_type = uint32_t;
  using st = small_vector<value_type, uint16_t, 0u>;
  st v;
  
  auto res = emplace_back( v, 0xf5u );
  CONSTEXPR_TEST( res == no_error );
  CONSTEXPR_TEST(size(v) == 1u);
  CONSTEXPR_TEST(!empty(v));
  CONSTEXPR_TEST(begin(v) != end(v));
  CONSTEXPR_TEST( *begin(v) == 0xf5u );
  CONSTEXPR_TEST(back(v) == 0xf5u );
  CONSTEXPR_TEST(v.active_storage() == dynamic );
  
  res = emplace_back( v, 0x15u );
  {
  std::array<value_type,2> expected{0xf5u,0x15u };
  CONSTEXPR_TEST( res == no_error );
  CONSTEXPR_TEST(size(v) == 2u);
  CONSTEXPR_TEST(!empty(v));
  CONSTEXPR_TEST(begin(v) != end(v));
  CONSTEXPR_TEST(v.active_storage() == dynamic );
  CONSTEXPR_TEST(equal(v,expected));
  }
  
  res = emplace_back( v, 0x25u );
  {
  std::array<value_type,3> expected{0xf5u,0x15u,0x25u};
  CONSTEXPR_TEST( res == no_error );
  CONSTEXPR_TEST(size(v) == 3u);
  CONSTEXPR_TEST(!empty(v));
  CONSTEXPR_TEST(begin(v) != end(v));
  CONSTEXPR_TEST(v.active_storage() == dynamic );
  CONSTEXPR_TEST(equal(v,expected));
  }
  return true;
  }
BOOST_AUTO_TEST_CASE( test_small_vector_emplace_back_0 )
{
  BOOST_TEST(consteval_test_small_vector_emplace_back_0());
}
//---------------------------------------------------------------------------------------------------------------------
consteval small_vector_storage_type
expected_istorage(auto capacity_req) noexcept
  { return capacity_req == 0u ? dynamic : buffered; }
//---------------------------------------------------------------------------------------------------------------------
template<typename value_type, typename size_type, 
          size_type capacity_req =at_least<value_type>(size_type(4))>
constexpr bool constexpr_test_small_vector_emplace_back_tmpl()
  {
  std::array<value_type,127> test_values{};
  std::iota( begin(test_values), end(test_values), value_type(1) );
  
//   constexpr size_type capacity_req =at_least<value_type>(size_type(4));
  using st = small_vector<value_type, size_type, capacity_req>;
  st v;
  constexpr size_type first_growth = detail::growth( capacity_req, size_type(1));
  constexpr size_type second_growth = detail::growth( first_growth, size_type(1));
  static_assert(second_growth+1u < test_values.size() );
  for( size_type s{}; s != capacity_req; ++s)
    {
    auto res = emplace_back( v, test_values[s] );
    CONSTEXPR_TEST( res == no_error );
    CONSTEXPR_TEST(v.active_storage() == expected_istorage(capacity_req) );
    CONSTEXPR_TEST(size(v) == s+1);
    CONSTEXPR_TEST(!empty(v));
    std::span expected{ test_values.data(), s+1u };
    CONSTEXPR_TEST(equal(v,expected));
    CONSTEXPR_TEST(back(v) == test_values[s] );
    }
  for( size_type s{capacity_req}; s != first_growth; ++s)
    {
    auto res = emplace_back( v, test_values[s] );
    CONSTEXPR_TEST( res == no_error );
    CONSTEXPR_TEST(v.active_storage() == dynamic );
    CONSTEXPR_TEST(size(v) == s+1);
    CONSTEXPR_TEST(!empty(v));
    std::span expected{ test_values.data(), s+1u };
    CONSTEXPR_TEST(equal(v,expected));
    CONSTEXPR_TEST(back(v) == test_values[s] );
    }
//   dump(v);
  for( size_type s{first_growth}; s != (second_growth+1u); ++s)
    {
    auto res = emplace_back( v, test_values[s] );
    CONSTEXPR_TEST( res == no_error );
    CONSTEXPR_TEST(v.active_storage() == dynamic );
    CONSTEXPR_TEST(size(v) == s+1);
    CONSTEXPR_TEST(!empty(v));
    std::span expected{ test_values.data(), s+1u };
//     dump(expected);
//     dump(v);
    CONSTEXPR_TEST(equal(v,expected));
    CONSTEXPR_TEST(back(v) == test_values[s] );
    }
  return true;
  }
template<typename value_type, typename size_type, 
          size_type capacity_req =at_least<value_type>(size_type(4))>
consteval bool consteval_test_small_vector_emplace_back_tmpl()
  {
  return constexpr_test_small_vector_emplace_back_tmpl<value_type,size_type,capacity_req>();
  }
BOOST_AUTO_TEST_CASE_TEMPLATE( test_small_vector_emplace_back_tmpl, size_type, constexpr_size_type_traits_list )
  {
  BOOST_TEST(( consteval_test_small_vector_emplace_back_tmpl<uint8_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_emplace_back_tmpl<uint16_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_emplace_back_tmpl<uint32_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_emplace_back_tmpl<uint64_t,size_type>()));
  
  BOOST_TEST(( constexpr_test_small_vector_emplace_back_tmpl<uint8_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_emplace_back_tmpl<uint16_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_emplace_back_tmpl<uint32_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_emplace_back_tmpl<uint64_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_emplace_back_tmpl<non_trivial,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_emplace_back_tmpl<non_trivial_ptr,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_emplace_back_tmpl<non_trivial_ptr_except,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_emplace_back_tmpl<non_trivial_ptr_except_copy,size_type>()));
  
  BOOST_TEST(( consteval_test_small_vector_emplace_back_tmpl<uint8_t,size_type, 0>()));
  BOOST_TEST(( consteval_test_small_vector_emplace_back_tmpl<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_emplace_back_tmpl<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_emplace_back_tmpl<non_trivial_ptr_except_copy,size_type,0>()));
  
  BOOST_TEST(( constexpr_test_small_vector_emplace_back_tmpl<uint8_t,size_type, 0>()));
  BOOST_TEST(( constexpr_test_small_vector_emplace_back_tmpl<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_emplace_back_tmpl<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_emplace_back_tmpl<non_trivial_ptr_except_copy,size_type,0>()));
  }
//---------------------------------------------------------------------------------------------------------------------
//test for use with runtime asan to check proper unwinding and memory reclamation
template<typename value_type, typename size_type, 
          size_type capacity_req = at_least<value_type>(size_type(4))>
constexpr bool constexpr_test_small_vector_emplace_back_except()
  {
  std::array<value_type,127> test_values{};
  std::iota( begin(test_values), end(test_values), value_type(1) );

  using st = small_vector<value_type, size_type, capacity_req>;
  {
  st v;
  reserve(v,4u);
  insert(v, begin(v), begin(test_values), std::next(begin(test_values), int(capacity(v))) );
  BOOST_REQUIRE_THROW((emplace_back(v, value_type(128) )), std::runtime_error);
  
  value_type const cobj(128);
  BOOST_REQUIRE_THROW((emplace_back(v, cobj )), std::runtime_error);
  }
  {
  st v;
  reserve(v,4u);
  insert(v, begin(v), begin(test_values), std::next(begin(test_values), int(capacity(v))) );
  back(v).set_value(128);
  BOOST_REQUIRE_THROW((emplace_back(v, value_type(1) )), std::runtime_error);
  back(v).set_value(128);
  value_type const cobj(1);
  BOOST_REQUIRE_THROW((emplace_back(v, cobj )), std::runtime_error);
  }
  {
  st v;
  reserve(v,40u);
  insert(v, begin(v), begin(test_values), std::next(begin(test_values), int(capacity(v))) );
  BOOST_REQUIRE_THROW((emplace_back(v, value_type(128) )), std::runtime_error);
  
  value_type const cobj(128);
  BOOST_REQUIRE_THROW((emplace_back(v, cobj )), std::runtime_error);
  }
  {
  st v;
  reserve(v,40u);
  insert(v, begin(v), begin(test_values), std::next(begin(test_values), int(capacity(v))) );
  back(v).set_value(128);
  BOOST_REQUIRE_THROW((emplace_back(v, value_type(1) )), std::runtime_error);
  back(v).set_value(128);
  value_type const cobj(1);
  BOOST_REQUIRE_THROW((emplace_back(v, cobj )), std::runtime_error);
  }
  return true;
  }

BOOST_AUTO_TEST_CASE_TEMPLATE( test_small_vector_emplace_back_except, size_type, constexpr_size_type_traits_list )
  {
  BOOST_TEST(( constexpr_test_small_vector_emplace_back_except<non_trivial_throwing<128>,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_emplace_back_except<non_trivial_throwing_copy<128>,size_type>()));

  BOOST_TEST(( constexpr_test_small_vector_emplace_back_except<non_trivial_throwing<128>,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_emplace_back_except<non_trivial_throwing_copy<128>,size_type,0>()));
  }
//---------------------------------------------------------------------------------------------------------------------
template<typename value_type, typename size_type, 
          size_type capacity_req =at_least<value_type>(size_type(4))>
constexpr bool constexpr_test_small_vector_erase_at_end()
  {
  std::array<value_type,127> test_values;
  std::iota( begin(test_values), end(test_values), value_type(1) );
  
  using st = small_vector<value_type, size_type, capacity_req>;
  st v;

  constexpr size_type second_growth = detail::growth( detail::growth( capacity_req, size_type(1)), size_type(1));
  static_assert(second_growth+1u < test_values.size() );
  for( size_type s{}; s != second_growth; ++s)
    emplace_back( v, test_values[s] );
  { 
  CONSTEXPR_TEST(v.active_storage() == dynamic );
  CONSTEXPR_TEST(size(v) == second_growth);
  CONSTEXPR_TEST(!empty(v));
  std::span expected{ test_values.data(), second_growth };
  CONSTEXPR_TEST(equal(v,expected));
  }
  for( size_type s{second_growth}; s > 1u; s-=2)
    {
    auto it = erase_at_end(v, std::prev(end(v),2));
    CONSTEXPR_TEST(it == end(v));
    CONSTEXPR_TEST(size(v) == s-2);
    std::span expected{ test_values.data(), s-2u };
    CONSTEXPR_TEST(equal(v,expected));
    if(s>2)
      CONSTEXPR_TEST(back(v) == test_values[s-3u] );
    }
  return true;
  }
template<typename value_type, typename size_type, 
          size_type capacity_req =at_least<value_type>(size_type(4))>
consteval bool consteval_test_small_vector_erase_at_end()
  {
  return constexpr_test_small_vector_erase_at_end<value_type,size_type,capacity_req>();
  }
BOOST_AUTO_TEST_CASE_TEMPLATE( test_small_vector_erase_at_end, size_type, constexpr_size_type_traits_list )
  {
  BOOST_TEST(( consteval_test_small_vector_erase_at_end<uint8_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_erase_at_end<uint16_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_erase_at_end<uint32_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_erase_at_end<uint64_t,size_type>()));
  
  BOOST_TEST(( constexpr_test_small_vector_erase_at_end<uint8_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_erase_at_end<uint16_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_erase_at_end<uint32_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_erase_at_end<uint64_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_erase_at_end<non_trivial,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_erase_at_end<non_trivial_ptr,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_erase_at_end<non_trivial_ptr_except,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_erase_at_end<non_trivial_ptr_except_copy,size_type>()));
  
  BOOST_TEST(( consteval_test_small_vector_erase_at_end<uint8_t,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_erase_at_end<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_erase_at_end<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_erase_at_end<non_trivial_ptr_except_copy,size_type,0>()));
  
  BOOST_TEST(( constexpr_test_small_vector_erase_at_end<uint8_t,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_erase_at_end<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_erase_at_end<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_erase_at_end<non_trivial_ptr_except_copy,size_type,0>()));
  }
//---------------------------------------------------------------------------------------------------------------------
template<typename value_type, typename size_type, 
          size_type capacity_req =at_least<value_type>(size_type(4))>
constexpr bool constexpr_test_small_vector_pop_back()
  {
  std::array<value_type,127> test_values;
  std::iota( begin(test_values), end(test_values), value_type(1) );
  
  using st = small_vector<value_type, size_type, capacity_req>;
  st v;

  constexpr size_type second_growth = detail::growth( detail::growth( capacity_req, size_type(1)), size_type(1));
  static_assert(second_growth+1u < test_values.size() );
  for( size_type s{}; s != second_growth; ++s)
    emplace_back( v, test_values[s] );
  { 
  CONSTEXPR_TEST(v.active_storage() == dynamic );
  CONSTEXPR_TEST(size(v) == second_growth);
  CONSTEXPR_TEST(!empty(v));
  std::span expected{ test_values.data(), second_growth };
  CONSTEXPR_TEST(equal(v,expected));
  }
  size_type curr_capacity { capacity(v) };
  for( size_type s{second_growth}; s != 0; --s)
    {
    pop_back(v);
    CONSTEXPR_TEST(size(v) == s-1);
    std::span expected{ test_values.data(), s-1u };
    CONSTEXPR_TEST(equal(v,expected));
    CONSTEXPR_TEST(capacity(v) == curr_capacity);
    if(s>1)
      CONSTEXPR_TEST(back(v) == test_values[s-2u] );
    }
  return true;
  }
template<typename value_type, typename size_type, 
          size_type capacity_req =at_least<value_type>(size_type(4))>
consteval bool consteval_test_small_vector_pop_back()
  {
  return constexpr_test_small_vector_pop_back<value_type,size_type,capacity_req>();
  }
BOOST_AUTO_TEST_CASE_TEMPLATE( test_small_vector_pop_back, size_type, constexpr_size_type_traits_list )
  {
  BOOST_TEST(( consteval_test_small_vector_pop_back<uint8_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_pop_back<uint16_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_pop_back<uint32_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_pop_back<uint64_t,size_type>()));
  
  BOOST_TEST(( constexpr_test_small_vector_pop_back<uint8_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_pop_back<uint16_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_pop_back<uint32_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_pop_back<uint64_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_pop_back<non_trivial,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_pop_back<non_trivial_ptr,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_pop_back<non_trivial_ptr_except,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_pop_back<non_trivial_ptr_except_copy,size_type>()));
  
  BOOST_TEST(( consteval_test_small_vector_pop_back<uint8_t,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_pop_back<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_pop_back<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_pop_back<non_trivial_ptr_except_copy,size_type,0>()));
  
  BOOST_TEST(( constexpr_test_small_vector_pop_back<uint8_t,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_pop_back<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_pop_back<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_pop_back<non_trivial_ptr_except_copy,size_type,0>()));
  }
  
//---------------------------------------------------------------------------------------------------------------------
template<typename value_type, typename size_type, 
          size_type capacity_req =at_least<value_type>(size_type(4))>
constexpr bool constexpr_test_small_vector_clear()
  {
  std::array<value_type,127> test_values;
  std::iota( begin(test_values), end(test_values), value_type(1) );
  
  using st = small_vector<value_type, size_type, capacity_req>;
  st v;

  constexpr size_type second_growth = detail::growth( detail::growth( capacity_req, size_type(1)), size_type(1));
  static_assert(second_growth+1u < test_values.size() );
  for( size_type s{}; s != second_growth; ++s)
    emplace_back( v, test_values[s] );
  { 
  CONSTEXPR_TEST(v.active_storage() == dynamic );
  CONSTEXPR_TEST(size(v) == second_growth);
  CONSTEXPR_TEST(!empty(v));
  std::span expected{ test_values.data(), second_growth };
  CONSTEXPR_TEST(equal(v,expected));
  }
  size_type curr_capacity { capacity(v) };
  clear(v);
  CONSTEXPR_TEST(capacity(v) == curr_capacity);
  CONSTEXPR_TEST(size(v) == 0u);
  CONSTEXPR_TEST(empty(v));
  CONSTEXPR_TEST(v.active_storage() == dynamic );
  
  return true;
  }
template<typename value_type, typename size_type,
          size_type capacity_req =at_least<value_type>(size_type(4))>
consteval bool consteval_test_small_vector_clear()
  {
  return constexpr_test_small_vector_clear<value_type,size_type,capacity_req>();
  }
BOOST_AUTO_TEST_CASE_TEMPLATE( test_small_vector_clear, size_type, constexpr_size_type_traits_list )
  {
  BOOST_TEST(( consteval_test_small_vector_clear<uint8_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_clear<uint16_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_clear<uint32_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_clear<uint64_t,size_type>()));
  
  BOOST_TEST(( constexpr_test_small_vector_clear<uint8_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_clear<uint16_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_clear<uint32_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_clear<uint64_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_clear<non_trivial,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_clear<non_trivial_ptr,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_clear<non_trivial_ptr_except,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_clear<non_trivial_ptr_except_copy,size_type>()));
  
  BOOST_TEST(( consteval_test_small_vector_clear<uint8_t,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_clear<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_clear<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_clear<non_trivial_ptr_except_copy,size_type,0>()));
  
  BOOST_TEST(( constexpr_test_small_vector_clear<uint8_t,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_clear<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_clear<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_clear<non_trivial_ptr_except_copy,size_type,0>()));
  }
//---------------------------------------------------------------------------------------------------------------------
//storage buffered is covered by static_vector tests
template<typename value_type, typename size_type,
          size_type capacity_req =at_least<value_type>(size_type(4))>
constexpr bool constexpr_test_small_vector_insert()
  {
  std::array<value_type,127> test_values;
  std::iota( begin(test_values), end(test_values), value_type(1) );
  
  using st = small_vector<value_type, size_type, capacity_req>;
  constexpr size_type first_growth = detail::growth( capacity_req, size_type(1));
  constexpr size_type second_growth = detail::growth( first_growth, size_type(1));
  static_assert(second_growth+1u < test_values.size() );
  
    {
    st v;
    emplace_back(v,value_type(0xfe) );
    emplace_back(v,value_type(0xff) );
    std::span span1{ test_values.data(), second_growth };
    auto res = insert(v, begin(v), begin(span1), end(span1) );
    
    CONSTEXPR_TEST(res == no_error );
    std::array<value_type,second_growth+2> expected;
    std::iota( begin(expected), std::prev(end(expected),2), value_type(1) );
    expected[ expected.size()-2] = value_type(0xfe);
    expected[ expected.size()-1] = value_type(0xff);
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    CONSTEXPR_TEST(v.active_storage() == dynamic );
    }

    {
    st v;
    emplace_back(v,value_type(0xfe) );
    emplace_back(v,value_type(0xff) );
    std::span span1{ test_values.data(), second_growth };
    auto res = insert(v, std::next(begin(v),1), begin(span1), end(span1) );
    
    CONSTEXPR_TEST(res == no_error );
    std::array<value_type,second_growth+2> expected;
    std::iota( std::next(begin(expected),1), std::prev(end(expected),1), value_type(1) );
    expected[0] = value_type(0xfe);
    expected[ expected.size()-1] = value_type(0xff);
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    CONSTEXPR_TEST(v.active_storage() == dynamic );
    }
    
    {
    st v;
    emplace_back(v,value_type(0xfe) );
    emplace_back(v,value_type(0xff) );
    std::span span1{ test_values.data(), second_growth };
    auto res = insert(v, std::next(begin(v),2), begin(span1), end(span1) );
    
    CONSTEXPR_TEST(res == no_error );
    std::array<value_type,second_growth+2> expected;
    std::iota( std::next(begin(expected),2), end(expected), value_type(1) );
    expected[0] = value_type(0xfe);
    expected[1] = value_type(0xff);
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    CONSTEXPR_TEST(v.active_storage() == dynamic );
    }
  return true;
  }
template<typename value_type, typename size_type,
          size_type capacity_req =at_least<value_type>(size_type(4))>
consteval bool consteval_test_small_vector_insert()
  {
  return constexpr_test_small_vector_insert<value_type,size_type,capacity_req>();
  }
BOOST_AUTO_TEST_CASE_TEMPLATE( test_small_vector_insert, size_type, constexpr_size_type_traits_list )
  {
  BOOST_TEST(( consteval_test_small_vector_insert<uint8_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_insert<uint16_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_insert<uint32_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_insert<uint64_t,size_type>()));
  
  BOOST_TEST(( constexpr_test_small_vector_insert<uint8_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_insert<uint16_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_insert<uint32_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_insert<uint64_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_insert<non_trivial,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_insert<non_trivial_ptr,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_insert<non_trivial_ptr_except,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_insert<non_trivial_ptr_except_copy,size_type>()));
  
  BOOST_TEST(( consteval_test_small_vector_insert<uint8_t,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_insert<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_insert<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_insert<non_trivial_ptr_except_copy,size_type,0>()));
  
  BOOST_TEST(( constexpr_test_small_vector_insert<uint8_t,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_insert<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_insert<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_insert<non_trivial_ptr_except_copy,size_type,0>()));
  }
//---------------------------------------------------------------------------------------------------------------------
//test for use with runtime asan to check proper unwinding and memory reclamation
template<typename value_type, typename size_type, 
          size_type capacity_req = at_least<value_type>(size_type(4))>
constexpr bool constexpr_test_small_vector_insert_except()
  {
  std::array<value_type,127> test_values{};
  std::iota( begin(test_values), end(test_values), value_type(1) );
  std::array<value_type,2> throwing;
  std::iota( begin(throwing), end(throwing), value_type(126) );
  ++throwing.back();
  
  using st = small_vector<value_type, size_type, capacity_req>;
  st v;
  reserve(v,4);
  insert(v, begin(v), begin(test_values), std::next(begin(test_values), int(capacity(v))) );
  BOOST_REQUIRE_THROW((insert(v, v.begin(), begin(throwing), end(throwing) )), std::runtime_error);
  
  BOOST_REQUIRE_THROW((insert(v, v.begin(), std::make_move_iterator(begin(throwing)),
                              std::make_move_iterator(end(throwing)) )), std::runtime_error);
  v.front().set_value(128);
  BOOST_REQUIRE_THROW((insert(v, v.begin(), begin(test_values), std::next(begin(test_values),10) )), std::runtime_error);
  
  v.front().set_value(0);
  v.back().set_value(128);
  BOOST_REQUIRE_THROW((insert(v, v.begin(), begin(test_values), std::next(begin(test_values),10) )), std::runtime_error);
  return true;
  }

BOOST_AUTO_TEST_CASE_TEMPLATE( test_small_vector_insert_except, size_type, constexpr_size_type_traits_list )
  {
  BOOST_TEST(( constexpr_test_small_vector_insert_except<non_trivial_throwing<128>,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_insert_except<non_trivial_throwing_copy<128>,size_type>()));

  BOOST_TEST(( constexpr_test_small_vector_insert_except<non_trivial_throwing<128>,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_insert_except<non_trivial_throwing_copy<128>,size_type,0>()));
  }
//---------------------------------------------------------------------------------------------------------------------
//storage buffered is covered by static_vector tests
template<typename value_type, typename size_type,
          size_type capacity_req =at_least<value_type>(size_type(4))>
constexpr bool constexpr_test_small_vector_emplace()
  {
  std::array<value_type,127> test_values;
  std::iota( begin(test_values), end(test_values), value_type(1) );
  
  using st = small_vector<value_type, size_type, capacity_req>;
  constexpr size_type first_growth = detail::growth( capacity_req, size_type(1));
  constexpr size_type second_growth = detail::growth( first_growth, size_type(1));
  static_assert(second_growth+1u < test_values.size() );
  
    {
    st v;
    std::span span1{ test_values.data(), second_growth };
    insert(v, begin(v), begin(span1), end(span1) );
    
    auto res = emplace(v, begin(v), value_type(0x1f) );
    
    CONSTEXPR_TEST(res == no_error );
    std::array<value_type,second_growth+1> expected;
    std::iota( std::next(begin(expected),1), end(expected), value_type(1) );
    expected[0] = value_type(0x1f);
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    }
    {
    st v;
    std::span span1{ test_values.data(), second_growth };
    insert(v, begin(v), begin(span1), end(span1) );
    
    auto res = emplace(v, std::next(begin(v),1), value_type(0x1f) );
    
    CONSTEXPR_TEST(res == no_error );
    std::array<value_type,second_growth+1> expected;
    std::iota( begin(expected), end(expected), value_type(0) );
    expected[0] = value_type(1);
    expected[1] = value_type(0x1f);
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    }
    {
    st v;
    std::span span1{ test_values.data(), second_growth };
    insert(v, begin(v), begin(span1), end(span1) );
    
    auto res = emplace(v, end(v), value_type(0x1f) );
    
    CONSTEXPR_TEST(res == no_error );
    std::array<value_type,second_growth+1> expected;
    std::iota( begin(expected), std::prev(end(expected),1), value_type(1) );
    expected[expected.size()-1] = value_type(0x1f);
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    }
  return true;
  }
  
template<typename value_type, typename size_type,
          size_type capacity_req =at_least<value_type>(size_type(4))>
consteval bool consteval_test_small_vector_emplace()
  { return constexpr_test_small_vector_emplace<value_type,size_type,capacity_req>(); }
  
BOOST_AUTO_TEST_CASE_TEMPLATE( test_small_vector_emplace, size_type, constexpr_size_type_traits_list )
  {
  BOOST_TEST(( consteval_test_small_vector_emplace<uint8_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_emplace<uint16_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_emplace<uint32_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_emplace<uint64_t,size_type>()));
  
  BOOST_TEST(( constexpr_test_small_vector_emplace<uint8_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_emplace<uint16_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_emplace<uint32_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_emplace<uint64_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_emplace<non_trivial,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_emplace<non_trivial_ptr,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_emplace<non_trivial_ptr_except,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_emplace<non_trivial_ptr_except_copy,size_type>()));
  
  BOOST_TEST(( consteval_test_small_vector_emplace<uint8_t,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_emplace<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_emplace<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_emplace<non_trivial_ptr_except_copy,size_type,0>()));
  
  BOOST_TEST(( constexpr_test_small_vector_emplace<uint8_t,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_emplace<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_emplace<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_emplace<non_trivial_ptr_except_copy,size_type,0>()));
  }
//---------------------------------------------------------------------------------------------------------------------
//test for use with runtime asan to check proper unwinding and memory reclamation
template<typename value_type, typename size_type, 
          size_type capacity_req = at_least<value_type>(size_type(4))>
constexpr bool constexpr_test_small_vector_emplace_except()
  {
  std::array<value_type,127> test_values{};
  std::iota( begin(test_values), end(test_values), value_type(1) );
  
  using st = small_vector<value_type, size_type, capacity_req>;
  st v;
  reserve(v,4);
  insert(v, begin(v), begin(test_values), std::next(begin(test_values), int(capacity(v))) );
  BOOST_REQUIRE_THROW((emplace(v, v.begin(), value_type{128} )), std::runtime_error);
  
  value_type const tst{128};
  BOOST_REQUIRE_THROW((emplace(v, v.begin(), tst )), std::runtime_error);
  v.front().set_value(128);
  BOOST_REQUIRE_THROW((emplace(v, v.begin(), 1 )), std::runtime_error);
  
  v.front().set_value(0);
  v.back().set_value(128);
  BOOST_REQUIRE_THROW((emplace(v, v.begin(), 1 )), std::runtime_error);
  return true;
  }
BOOST_AUTO_TEST_CASE_TEMPLATE( test_small_vector_emplace_except, size_type, constexpr_size_type_traits_list )
  {
  BOOST_TEST(( constexpr_test_small_vector_emplace_except<non_trivial_throwing<128>,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_emplace_except<non_trivial_throwing_copy<128>,size_type>()));

  BOOST_TEST(( constexpr_test_small_vector_emplace_except<non_trivial_throwing<128>,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_emplace_except<non_trivial_throwing_copy<128>,size_type,0>()));
  }
//---------------------------------------------------------------------------------------------------------------------
template<typename value_type, typename size_type,
          size_type capacity_req =at_least<value_type>(size_type(4)),
          size_type capacity_req2 = capacity_req>
constexpr bool constexpr_test_small_vector_copy_constr()
  {
  std::array<value_type,127> test_values;
  std::iota( begin(test_values), end(test_values), value_type(1) );
  
  using sa = small_vector<value_type, size_type, capacity_req>;
  using st = small_vector<value_type, size_type, capacity_req2>;
  constexpr size_type first_growth = detail::growth( capacity_req, size_type(1));
  constexpr size_type second_growth = detail::growth( first_growth, size_type(1));
  static_assert(second_growth+1u < test_values.size() );
  
  if constexpr(capacity_req !=0u)
    {
    st v;
    std::span expected{ test_values.data(), capacity_req };
    insert(v, begin(v), begin(expected), end(expected) );
    
    if constexpr(capacity_req == capacity_req2)
      CONSTEXPR_TEST(v.active_storage() == expected_istorage(capacity_req) );
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    st const & cv { v };
    sa cp{ cv };
    if constexpr(capacity_req == capacity_req2)
      CONSTEXPR_TEST(cp.active_storage() == expected_istorage(capacity_req) );
    CONSTEXPR_TEST(size(cp) == expected.size());
    CONSTEXPR_TEST(equal(cp,expected));
    
    }
    {
    st v;
    std::span expected{ test_values.data(), second_growth };
    insert(v, begin(v), begin(expected), end(expected) );
    
    if constexpr(capacity_req == capacity_req2)
      CONSTEXPR_TEST(v.active_storage() == dynamic );
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    st const & cv { v };
    sa cp{ cv };
    if constexpr(capacity_req == capacity_req2)
      CONSTEXPR_TEST(cp.active_storage() == dynamic );
    CONSTEXPR_TEST(size(cp) == expected.size());
    CONSTEXPR_TEST(equal(cp,expected));
    
    }
  return true;
  }
template<typename value_type, typename size_type,
          size_type capacity_req =at_least<value_type>(size_type(4))>
consteval bool consteval_test_small_vector_copy_constr()
  {
  return constexpr_test_small_vector_copy_constr<value_type,size_type,capacity_req>();
  }
BOOST_AUTO_TEST_CASE_TEMPLATE( test_small_vector_copy_constr, size_type, constexpr_size_type_traits_list )
  {
  BOOST_TEST(( consteval_test_small_vector_copy_constr<uint8_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_copy_constr<uint16_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_copy_constr<uint32_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_copy_constr<uint64_t,size_type>()));
  
  BOOST_TEST(( constexpr_test_small_vector_copy_constr<uint8_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_constr<uint16_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_constr<uint32_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_constr<uint64_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_constr<non_trivial,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_constr<non_trivial_ptr,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_constr<non_trivial_ptr_except,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_constr<non_trivial_ptr_except_copy,size_type>()));
  
  BOOST_TEST(( consteval_test_small_vector_copy_constr<uint8_t,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_copy_constr<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_copy_constr<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_copy_constr<non_trivial_ptr_except_copy,size_type,0>()));
  
  BOOST_TEST(( constexpr_test_small_vector_copy_constr<uint8_t,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_constr<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_constr<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_constr<non_trivial_ptr_except_copy,size_type,0>()));
  }
//---------------------------------------------------------------------------------------------------------------------
template<typename value_type, typename size_type,
          size_type capacity_req = at_least<value_type>(size_type(4)),
          size_type capacity_req2 = 25>
constexpr bool constexpr_test_small_vector_copy_constr_compat()
  {
  return constexpr_test_small_vector_copy_constr<value_type,size_type,capacity_req,capacity_req2>();
  }
template<typename value_type, typename size_type,
          size_type capacity_req = at_least<value_type>(size_type(4)),
          size_type capacity_req2 = 25>
consteval bool consteval_test_small_vector_copy_constr_compat()
  {
  return constexpr_test_small_vector_copy_constr<value_type,size_type,capacity_req,capacity_req2>();
  }
BOOST_AUTO_TEST_CASE_TEMPLATE( test_small_vector_copy_constr_compat, size_type, constexpr_size_type_traits_list )
  {
  BOOST_TEST(( consteval_test_small_vector_copy_constr_compat<uint8_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_copy_constr_compat<uint16_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_copy_constr_compat<uint32_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_copy_constr_compat<uint64_t,size_type>()));
  
  BOOST_TEST(( constexpr_test_small_vector_copy_constr_compat<uint8_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_constr_compat<uint16_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_constr_compat<uint32_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_constr_compat<uint64_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_constr_compat<non_trivial,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_constr_compat<non_trivial_ptr,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_constr_compat<non_trivial_ptr_except,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_constr_compat<non_trivial_ptr_except_copy,size_type>()));
  
  BOOST_TEST(( constexpr_test_small_vector_copy_constr_compat<uint8_t,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_constr_compat<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_constr_compat<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_constr_compat<non_trivial_ptr_except_copy,size_type,0>()));
  
  BOOST_TEST(( constexpr_test_small_vector_copy_constr_compat<uint8_t,size_type,25,0>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_constr_compat<non_trivial_ptr,size_type,25,0>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_constr_compat<non_trivial_ptr_except,size_type,25,0>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_constr_compat<non_trivial_ptr_except_copy,size_type,25,0>()));
  }
//---------------------------------------------------------------------------------------------------------------------
//test for use with runtime asan to check proper unwinding and memory reclamation
template<typename value_type, typename size_type,
          size_type capacity_req =at_least<value_type>(size_type(4))>
constexpr bool constexpr_test_small_vector_copy_constr_except()
  {
  std::array<value_type,127> test_values;
  std::iota( begin(test_values), end(test_values), value_type(1) );
  
  using st = small_vector<value_type, size_type, capacity_req>;
  constexpr size_type first_growth = detail::growth( capacity_req, size_type(1));
  constexpr size_type second_growth = detail::growth( first_growth, size_type(1));
  static_assert(second_growth+1u < test_values.size() );
  
  if constexpr(capacity_req !=0u)
    {
    st v;
    std::span expected{ test_values.data(), capacity_req };
    insert(v, begin(v), begin(expected), end(expected) );

    CONSTEXPR_TEST(v.active_storage() == expected_istorage(capacity_req) );
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    v.front().set_value(128);
    st const & cv { v };
    bool except = false;
    try {
      st cp{ cv };
      }
    catch(...)
      {except = true; }
    CONSTEXPR_TEST(except);
    }
  if constexpr(capacity_req !=0u)
    {
    st v;
    std::span expected{ test_values.data(), capacity_req };
    insert(v, begin(v), begin(expected), end(expected) );

    CONSTEXPR_TEST(v.active_storage() == expected_istorage(capacity_req) );
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    v.front().set_value(0);
    v.back().set_value(128);
    st const & cv { v };
    bool except = false;
    try {
      st cp{ cv };
      }
    catch(...)
      {except = true; }
    CONSTEXPR_TEST(except);
    }
    {
    st v;
    std::span expected{ test_values.data(), second_growth };
    insert(v, begin(v), begin(expected), end(expected) );
    
    CONSTEXPR_TEST(v.active_storage() == dynamic );
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    v.front().set_value(128);
    st const & cv { v };
    bool except = false;
    try {
      st cp{ cv };
      }
    catch(...)
      {except = true; }
    CONSTEXPR_TEST(except);
    }
    {
    st v;
    std::span expected{ test_values.data(), second_growth };
    insert(v, begin(v), begin(expected), end(expected) );
    
    CONSTEXPR_TEST(v.active_storage() == dynamic );
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    v.front().set_value(0);
    v.back().set_value(128);
    st const & cv { v };
    bool except = false;
    try {
      st cp{ cv };
      }
    catch(...)
      {except = true; }
    CONSTEXPR_TEST(except);
    }
  return true;
  }
BOOST_AUTO_TEST_CASE_TEMPLATE( test_small_vector_copy_constr_except, size_type, constexpr_size_type_traits_list )
  {
  BOOST_TEST(( constexpr_test_small_vector_copy_constr_except<non_trivial_throwing<128>,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_constr_except<non_trivial_throwing_copy<128>,size_type>()));

  BOOST_TEST(( constexpr_test_small_vector_copy_constr_except<non_trivial_throwing<128>,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_constr_except<non_trivial_throwing_copy<128>,size_type,0>()));
  }
//---------------------------------------------------------------------------------------------------------------------
template<typename value_type, typename size_type,
          size_type capacity_req =at_least<value_type>(size_type(4))>
constexpr bool constexpr_test_small_vector_move_constr()
  {
  std::array<value_type,127> test_values;
  std::iota( begin(test_values), end(test_values), value_type(1) );
  
  using st = small_vector<value_type, size_type, capacity_req>;
  constexpr size_type first_growth = detail::growth( capacity_req, size_type(1));
  constexpr size_type second_growth = detail::growth( first_growth, size_type(1));
  static_assert(second_growth+1u < test_values.size() );
  
  if constexpr(capacity_req != 0u)
    {
    st v;
    
    std::span expected{ test_values.data(), capacity_req };
    insert(v, begin(v), begin(expected), end(expected) );
    
    CONSTEXPR_TEST(v.active_storage() == expected_istorage(capacity_req) );
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));

    st cp{ std::move(v) };
    CONSTEXPR_TEST(cp.active_storage() == expected_istorage(capacity_req) );
    CONSTEXPR_TEST(size(cp) == expected.size());
    CONSTEXPR_TEST(equal(cp,expected));
    
    CONSTEXPR_TEST(v.active_storage() == expected_istorage(capacity_req) );
    CONSTEXPR_TEST(size(v) == 0u );
    }
    
    {
    st v;
    std::span expected{ test_values.data(), second_growth };
    insert(v, begin(v), begin(expected), end(expected) );
    
    CONSTEXPR_TEST(v.active_storage() == dynamic );
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));

    st cp{ std::move(v) };
    CONSTEXPR_TEST(cp.active_storage() == dynamic );
    CONSTEXPR_TEST(size(cp) == expected.size());
    CONSTEXPR_TEST(equal(cp,expected));
    
    CONSTEXPR_TEST(v.active_storage() == expected_istorage(capacity_req) );
    CONSTEXPR_TEST(size(v) == 0u );
    }
  return true;
  }
template<typename value_type, typename size_type,
          size_type capacity_req =at_least<value_type>(size_type(4))>
consteval bool consteval_test_small_vector_move_constr()
  {
  return constexpr_test_small_vector_move_constr<value_type,size_type,capacity_req>();
  }
BOOST_AUTO_TEST_CASE_TEMPLATE( test_small_vector_move_constr, size_type, constexpr_size_type_traits_list )
  {
  BOOST_TEST(( consteval_test_small_vector_move_constr<uint8_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_move_constr<uint16_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_move_constr<uint32_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_move_constr<uint64_t,size_type>()));
  
  BOOST_TEST(( constexpr_test_small_vector_move_constr<uint8_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_move_constr<uint16_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_move_constr<uint32_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_move_constr<uint64_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_move_constr<non_trivial,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_move_constr<non_trivial_ptr,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_move_constr<non_trivial_ptr_except,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_move_constr<non_trivial_ptr_except_copy,size_type>()));
  
  BOOST_TEST(( consteval_test_small_vector_move_constr<uint8_t,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_move_constr<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_move_constr<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_move_constr<non_trivial_ptr_except_copy,size_type,0>()));
  
  BOOST_TEST(( constexpr_test_small_vector_move_constr<uint8_t,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_move_constr<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_move_constr<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_move_constr<non_trivial_ptr_except_copy,size_type,0>()));
  }
//---------------------------------------------------------------------------------------------------------------------
template<typename value_type, typename size_type,
          size_type capacity_req =at_least<value_type>(size_type(4)),
          size_type capacity_req2 = capacity_req>
constexpr bool constexpr_test_small_vector_copy_assignment()
  {
  std::array<value_type,127> test_values;
  std::iota( begin(test_values), end(test_values), value_type(1) );
  
  using st = small_vector<value_type, size_type, capacity_req>;
  using sa = small_vector<value_type, size_type, capacity_req2>;
  constexpr size_type first_growth = detail::growth( capacity_req, size_type(1));
  constexpr size_type second_growth = detail::growth( first_growth, size_type(1));
  static_assert(second_growth+1u < test_values.size() );
  
  //buffered = buffered
  if constexpr(expected_istorage(capacity_req)==buffered)
    {
    sa v;
    std::span expected{ test_values.data(), capacity_req };
    insert(v, begin(v), begin(expected), end(expected) );
    if constexpr(capacity_req == capacity_req2)
      CONSTEXPR_TEST(v.active_storage() == buffered );
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    sa const & cv { v };
    
    std::span redzone{ test_values.data()+5, capacity_req-1 };
    st cp;
    insert(cp, begin(cp), begin(redzone), end(redzone) );
    if constexpr(capacity_req == capacity_req2)
      CONSTEXPR_TEST(cp.active_storage() == buffered );
    CONSTEXPR_TEST(size(cp) == redzone.size());
    CONSTEXPR_TEST(equal(cp,redzone));
    cp.assign(cv);
    if constexpr(capacity_req == capacity_req2)
      CONSTEXPR_TEST(cp.active_storage() == buffered );
    CONSTEXPR_TEST(size(cp) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    }
  //dyanmic = buffered
  if constexpr(expected_istorage(capacity_req)==buffered)
    {
    sa v;
    std::span expected{ test_values.data(), capacity_req };
    insert(v, begin(v), begin(expected), end(expected) );
    if constexpr(capacity_req == capacity_req2)
      CONSTEXPR_TEST(v.active_storage() == buffered );
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    sa const & cv { v };
    
    std::span redzone{ test_values.data()+5, second_growth };
    st cp;
    insert(cp, begin(cp), begin(redzone), end(redzone) );
    if constexpr(capacity_req == capacity_req2)
      CONSTEXPR_TEST(cp.active_storage() == dynamic );
    CONSTEXPR_TEST(size(cp) == redzone.size());
    CONSTEXPR_TEST(equal(cp,redzone));
    cp.assign(cv);
    CONSTEXPR_TEST(cp.active_storage() == buffered );
    CONSTEXPR_TEST(size(cp) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    }
  //buffered = dynamic
  if constexpr(expected_istorage(capacity_req)==buffered)
    {
    sa v;
    std::span expected{ test_values.data(), second_growth };
    insert(v, begin(v), begin(expected), end(expected) );
    if constexpr(capacity_req == capacity_req2)
      CONSTEXPR_TEST(v.active_storage() == dynamic );
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    sa const & cv { v };
    
    std::span redzone{ test_values.data()+5, capacity_req-1 };
    st cp;
    insert(cp, begin(cp), begin(redzone), end(redzone) );
    if constexpr(capacity_req == capacity_req2)
    CONSTEXPR_TEST(cp.active_storage() == buffered );
    CONSTEXPR_TEST(size(cp) == redzone.size());
    CONSTEXPR_TEST(equal(cp,redzone));
    cp.assign(cv);
    CONSTEXPR_TEST(cp.active_storage() == dynamic );
    CONSTEXPR_TEST(size(cp) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    }
  //dyanmic = dynamic
    {
    sa v;
    std::span expected{ test_values.data(), second_growth };
    insert(v, begin(v), begin(expected), end(expected) );
    if constexpr(capacity_req == capacity_req2)
      CONSTEXPR_TEST(v.active_storage() == dynamic );
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    sa const & cv { v };
    
    std::span redzone{ test_values.data()+5, second_growth };
    st cp;
    insert(cp, begin(cp), begin(redzone), end(redzone) );
    CONSTEXPR_TEST(cp.active_storage() == dynamic );
    CONSTEXPR_TEST(size(cp) == redzone.size());
    CONSTEXPR_TEST(equal(cp,redzone));
    cp.assign(cv);
    CONSTEXPR_TEST(cp.active_storage() == dynamic );
    CONSTEXPR_TEST(size(cp) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    }
  return true;
  }
  
template<typename value_type, typename size_type,
          size_type capacity_req =at_least<value_type>(size_type(4))>
consteval bool consteval_test_small_vector_copy_assignment()
  { return constexpr_test_small_vector_copy_assignment<value_type,size_type,capacity_req>(); }
  
BOOST_AUTO_TEST_CASE_TEMPLATE( test_small_vector_copy_assignment, size_type, constexpr_size_type_traits_list )
  {
  BOOST_TEST(( consteval_test_small_vector_copy_assignment<uint8_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_copy_assignment<uint16_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_copy_assignment<uint32_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_copy_assignment<uint64_t,size_type>()));
  
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment<uint8_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment<uint16_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment<uint32_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment<uint64_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment<non_trivial,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment<non_trivial_ptr,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment<non_trivial_ptr_except,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment<non_trivial_ptr_except_copy,size_type>()));
  
  BOOST_TEST(( consteval_test_small_vector_copy_assignment<uint8_t,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_copy_assignment<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_copy_assignment<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_copy_assignment<non_trivial_ptr_except_copy,size_type,0>()));
  
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment<uint8_t,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment<non_trivial_ptr_except_copy,size_type,0>()));
  }
  
template<typename value_type, typename size_type,
          size_type capacity_req =at_least<value_type>(size_type(4)),
          size_type capacity_req2 =at_least<value_type>(size_type(25))>
constexpr bool constexpr_test_small_vector_copy_assignment_compat()
  { return constexpr_test_small_vector_copy_assignment<value_type,size_type,capacity_req,capacity_req2>(); }
  
template<typename value_type, typename size_type,
          size_type capacity_req =at_least<value_type>(size_type(4)),
          size_type capacity_req2 =at_least<value_type>(size_type(25))>
consteval bool consteval_test_small_vector_copy_assignment_compat()
  { return constexpr_test_small_vector_copy_assignment<value_type,size_type,capacity_req,capacity_req2>(); }
  
BOOST_AUTO_TEST_CASE_TEMPLATE( test_small_vector_copy_assignment_compat, size_type, constexpr_size_type_traits_list )
  {
  BOOST_TEST(( consteval_test_small_vector_copy_assignment_compat<uint8_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_copy_assignment_compat<uint16_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_copy_assignment_compat<uint32_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_copy_assignment_compat<uint64_t,size_type>()));
  
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment_compat<uint8_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment_compat<uint16_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment_compat<uint32_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment_compat<uint64_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment_compat<non_trivial,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment_compat<non_trivial_ptr,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment_compat<non_trivial_ptr_except,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment_compat<non_trivial_ptr_except_copy,size_type>()));
  
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment_compat<uint8_t,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment_compat<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment_compat<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment_compat<non_trivial_ptr_except_copy,size_type,0>()));
  
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment_compat<uint8_t,size_type,25,0>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment_compat<non_trivial_ptr,size_type,25,0>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment_compat<non_trivial_ptr_except,size_type,25,0>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment_compat<non_trivial_ptr_except_copy,size_type,25,0>()));
  }
//---------------------------------------------------------------------------------------------------------------------
template<typename value_type, typename size_type,
          size_type capacity_req =at_least<value_type>(size_type(4))>
constexpr bool constexpr_test_small_vector_copy_assignment_except()
  {
  std::array<value_type,127> test_values;
  std::iota( begin(test_values), end(test_values), value_type(1) );
  
  using st = small_vector<value_type, size_type, capacity_req>;
  constexpr size_type first_growth = detail::growth( capacity_req, size_type(1));
  constexpr size_type second_growth = detail::growth( first_growth, size_type(1));
  static_assert(second_growth+1u < test_values.size() );
  
  //buffered = buffered left throws
  if constexpr(expected_istorage(capacity_req)==buffered)
    {
    st v;
    std::span expected{ test_values.data(), capacity_req };
    insert(v, begin(v), begin(expected), end(expected) );
    
    CONSTEXPR_TEST(v.active_storage() == buffered );
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    st const & cv { v };
    
    std::span redzone{ test_values.data()+5, capacity_req-1 };
    st cp;
    insert(cp, begin(cp), begin(redzone), end(redzone) );
    CONSTEXPR_TEST(cp.active_storage() == buffered );
    CONSTEXPR_TEST(size(cp) == redzone.size());
    CONSTEXPR_TEST(equal(cp,redzone));
    cp.back().set_value(128);
    bool except = false;
    
    try {
      cp = cv;
      }
    catch(...)
      {except = true; }
    CONSTEXPR_TEST(not except);
    }
  //buffered = buffered right throws
  if constexpr(expected_istorage(capacity_req)==buffered)
    {
    st v;
    std::span expected{ test_values.data(), capacity_req };
    insert(v, begin(v), begin(expected), end(expected) );
    
    CONSTEXPR_TEST(v.active_storage() == buffered );
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    st const & cv { v };
    
    std::span redzone{ test_values.data()+5, capacity_req-1 };
    st cp;
    insert(cp, begin(cp), begin(redzone), end(redzone) );
    CONSTEXPR_TEST(cp.active_storage() == buffered );
    CONSTEXPR_TEST(size(cp) == redzone.size());
    CONSTEXPR_TEST(equal(cp,redzone));
    v.back().set_value(128);
    bool except = false;
    try {
      cp = cv;
      }
    catch(...)
      {except = true; }
    CONSTEXPR_TEST(except);
    }
    
  //dyanmic = buffered left throws
  if constexpr(expected_istorage(capacity_req)==buffered)
    {
    st v;
    std::span expected{ test_values.data(), capacity_req };
    insert(v, begin(v), begin(expected), end(expected) );
    
    CONSTEXPR_TEST(v.active_storage() == buffered );
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    st const & cv { v };
    
    std::span redzone{ test_values.data()+5, second_growth };
    st cp;
    insert(cp, begin(cp), begin(redzone), end(redzone) );
    CONSTEXPR_TEST(cp.active_storage() == dynamic );
    CONSTEXPR_TEST(size(cp) == redzone.size());
    CONSTEXPR_TEST(equal(cp,redzone));
    cp.back().set_value(128);
    bool except = false;
    try {
      cp = cv;
      }
    catch(...)
      {except = true; }
    CONSTEXPR_TEST(not except);
    }
  //dyanmic = buffered right throws
  if constexpr(expected_istorage(capacity_req)==buffered)
    {
    st v;
    std::span expected{ test_values.data(), capacity_req };
    insert(v, begin(v), begin(expected), end(expected) );
    
    CONSTEXPR_TEST(v.active_storage() == buffered );
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    st const & cv { v };
    
    std::span redzone{ test_values.data()+5, second_growth };
    st cp;
    insert(cp, begin(cp), begin(redzone), end(redzone) );
    CONSTEXPR_TEST(cp.active_storage() == dynamic );
    CONSTEXPR_TEST(size(cp) == redzone.size());
    CONSTEXPR_TEST(equal(cp,redzone));
    v.back().set_value(128);
    bool except = false;
    try {
      cp = cv;
      }
    catch(...)
      {except = true; }
    CONSTEXPR_TEST(except);
    }
    
  //buffered = dynamic buffered left throws
  if constexpr(expected_istorage(capacity_req)==buffered)
    {
    st v;
    std::span expected{ test_values.data(), second_growth };
    insert(v, begin(v), begin(expected), end(expected) );
    
    CONSTEXPR_TEST(v.active_storage() == dynamic );
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    st const & cv { v };
    
    std::span redzone{ test_values.data()+5, capacity_req-1 };
    st cp;
    insert(cp, begin(cp), begin(redzone), end(redzone) );
    CONSTEXPR_TEST(cp.active_storage() == buffered );
    CONSTEXPR_TEST(size(cp) == redzone.size());
    CONSTEXPR_TEST(equal(cp,redzone));
    cp.back().set_value(128);
    bool except = false;
    try {
      cp = cv;
      }
    catch(...)
      {except = true; }
    CONSTEXPR_TEST(not except);
    }
  //buffered = dynamic buffered right throws
  if constexpr(expected_istorage(capacity_req)==buffered)
    {
    st v;
    std::span expected{ test_values.data(), second_growth };
    insert(v, begin(v), begin(expected), end(expected) );
    
    CONSTEXPR_TEST(v.active_storage() == dynamic );
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    st const & cv { v };
    
    std::span redzone{ test_values.data()+5, capacity_req-1 };
    st cp;
    insert(cp, begin(cp), begin(redzone), end(redzone) );
    CONSTEXPR_TEST(cp.active_storage() == buffered );
    CONSTEXPR_TEST(size(cp) == redzone.size());
    CONSTEXPR_TEST(equal(cp,redzone));
    v.back().set_value(128);
    bool except = false;
    try {
      cp = cv;
      }
    catch(...)
      {except = true; }
    CONSTEXPR_TEST(except);
    }
    
  //dyanmic = dynamic left throws
    {
    st v;
    std::span expected{ test_values.data(), second_growth };
    insert(v, begin(v), begin(expected), end(expected) );
    
    CONSTEXPR_TEST(v.active_storage() == dynamic );
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    st const & cv { v };
    
    std::span redzone{ test_values.data()+1, second_growth };
    st cp;
    insert(cp, begin(cp), begin(redzone), end(redzone) );
    CONSTEXPR_TEST(cp.active_storage() == dynamic );
    CONSTEXPR_TEST(size(cp) == redzone.size());
    CONSTEXPR_TEST(equal(cp,redzone));
  
    cp.back().set_value(128);
    bool except = false;
    try {
      cp = cv;
      }
    catch(...)
      {except = true; }
    CONSTEXPR_TEST(not except);
    }
    
  //dyanmic = dynamic right throws
    {
    st v;
    std::span expected{ test_values.data(), second_growth };
    insert(v, begin(v), begin(expected), end(expected) );
    
    CONSTEXPR_TEST(v.active_storage() == dynamic );
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    st const & cv { v };
    
    std::span redzone{ test_values.data()+5, second_growth };
    st cp;
    insert(cp, begin(cp), begin(redzone), end(redzone) );
    CONSTEXPR_TEST(cp.active_storage() == dynamic );
    CONSTEXPR_TEST(size(cp) == redzone.size());
    CONSTEXPR_TEST(equal(cp,redzone));
    v.back().set_value(128);
    bool except = false;
    try {
      cp = cv;
      }
    catch(...)
      {except = true; }
    CONSTEXPR_TEST(except);
    }
  return true;
  }
BOOST_AUTO_TEST_CASE_TEMPLATE( test_small_vector_copy_assignment_except, size_type, constexpr_size_type_traits_list )
  {
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment_except<non_trivial_throwing<128>,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment_except<non_trivial_throwing_copy<128>,size_type>()));

  BOOST_TEST(( constexpr_test_small_vector_copy_assignment_except<non_trivial_throwing<128>,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_copy_assignment_except<non_trivial_throwing_copy<128>,size_type,0>()));
  }
//---------------------------------------------------------------------------------------------------------------------
template<typename value_type, typename size_type,
          size_type capacity_req =at_least<value_type>(size_type(4))>
constexpr bool constexpr_test_small_vector_move_assignment()
  {
  std::array<value_type,127> test_values;
  std::iota( begin(test_values), end(test_values), value_type(1) );
  
  using st = small_vector<value_type, size_type, capacity_req>;
  constexpr size_type first_growth = detail::growth( capacity_req, size_type(1));
  constexpr size_type second_growth = detail::growth( first_growth, size_type(1));
  static_assert(second_growth+1u < test_values.size() );
  
  //buffered = buffered
  if constexpr(expected_istorage(capacity_req)==buffered)
    {
    st v;
    std::span expected{ test_values.data(), capacity_req };
    insert(v, begin(v), begin(expected), end(expected) );
    
    CONSTEXPR_TEST(v.active_storage() == buffered );
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    
    std::span redzone{ test_values.data()+5, capacity_req-1 };
    st cp;
    insert(cp, begin(cp), begin(redzone), end(redzone) );
    CONSTEXPR_TEST(cp.active_storage() == buffered );
    CONSTEXPR_TEST(size(cp) == redzone.size());
    CONSTEXPR_TEST(equal(cp,redzone));
    cp = std::move(v);
    CONSTEXPR_TEST(cp.active_storage() == buffered );
    CONSTEXPR_TEST(size(cp) == expected.size());
    CONSTEXPR_TEST(equal(cp,expected));
    
    CONSTEXPR_TEST(v.active_storage() == buffered );
    CONSTEXPR_TEST(size(v) == 0u);
    }
  //buffered = dynamic
  if constexpr(expected_istorage(capacity_req)==buffered)
    {
    st v;
    std::span expected{ test_values.data(), second_growth };
    insert(v, begin(v), begin(expected), end(expected) );
    
    CONSTEXPR_TEST(v.active_storage() == dynamic );
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    
    std::span redzone{ test_values.data()+5, capacity_req-1 };
    st cp;
    insert(cp, begin(cp), begin(redzone), end(redzone) );
    CONSTEXPR_TEST(cp.active_storage() == buffered );
    CONSTEXPR_TEST(size(cp) == redzone.size());
    CONSTEXPR_TEST(equal(cp,redzone));
    cp = std::move(v);
    CONSTEXPR_TEST(cp.active_storage() == dynamic );
    CONSTEXPR_TEST(size(cp) == expected.size());
    CONSTEXPR_TEST(equal(cp,expected));
    
    CONSTEXPR_TEST(v.active_storage() == buffered );
    CONSTEXPR_TEST(size(v) == 0u);
    }
  //dynamic = buffered
  if constexpr(expected_istorage(capacity_req)==buffered)
    {
    st v;
    std::span expected{ test_values.data(), capacity_req };
    insert(v, begin(v), begin(expected), end(expected) );
    
    CONSTEXPR_TEST(v.active_storage() == buffered );
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    
    std::span redzone{ test_values.data()+5, second_growth };
    st cp;
    insert(cp, begin(cp), begin(redzone), end(redzone) );
    CONSTEXPR_TEST(cp.active_storage() == dynamic );
    CONSTEXPR_TEST(size(cp) == redzone.size());
    CONSTEXPR_TEST(equal(cp,redzone));
    cp = std::move(v);
    CONSTEXPR_TEST(cp.active_storage() == buffered );
    CONSTEXPR_TEST(size(cp) == expected.size());
    CONSTEXPR_TEST(equal(cp,expected));
    
    CONSTEXPR_TEST(v.active_storage() == buffered );
    CONSTEXPR_TEST(size(v) == 0u);
    }
  //dynamic = dynamic
    {
    st v;
    std::span expected{ test_values.data(), second_growth };
    insert(v, begin(v), begin(expected), end(expected) );
    
    CONSTEXPR_TEST(v.active_storage() == dynamic );
    CONSTEXPR_TEST(size(v) == expected.size());
    CONSTEXPR_TEST(equal(v,expected));
    
    std::span redzone{ test_values.data()+5, second_growth+1 };
    st cp;
    insert(cp, begin(cp), begin(redzone), end(redzone) );
    CONSTEXPR_TEST(cp.active_storage() == dynamic );
    CONSTEXPR_TEST(size(cp) == redzone.size());
    CONSTEXPR_TEST(equal(cp,redzone));
    cp = std::move(v);
    CONSTEXPR_TEST(cp.active_storage() == dynamic );
    CONSTEXPR_TEST(size(cp) == expected.size());
    CONSTEXPR_TEST(equal(cp,expected));
    
    CONSTEXPR_TEST(v.active_storage() == expected_istorage(capacity_req) );
    CONSTEXPR_TEST(size(v) == 0u);
    }
  return true;
  }
  
template<typename value_type, typename size_type,
          size_type capacity_req =at_least<value_type>(size_type(4))>
consteval bool consteval_test_small_vector_move_assignment()
  { return constexpr_test_small_vector_move_assignment<value_type,size_type,capacity_req>(); }
  
BOOST_AUTO_TEST_CASE_TEMPLATE( test_small_vector_move_assignment, size_type, constexpr_size_type_traits_list )
  {
  BOOST_TEST(( consteval_test_small_vector_move_assignment<uint8_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_move_assignment<uint16_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_move_assignment<uint32_t,size_type>()));
  BOOST_TEST(( consteval_test_small_vector_move_assignment<uint64_t,size_type>()));
  
  BOOST_TEST(( constexpr_test_small_vector_move_assignment<uint8_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_move_assignment<uint16_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_move_assignment<uint32_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_move_assignment<uint64_t,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_move_assignment<non_trivial,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_move_assignment<non_trivial_ptr,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_move_assignment<non_trivial_ptr_except,size_type>()));
  BOOST_TEST(( constexpr_test_small_vector_move_assignment<non_trivial_ptr_except_copy,size_type>()));
  
  BOOST_TEST(( consteval_test_small_vector_move_assignment<uint8_t,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_move_assignment<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_move_assignment<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( consteval_test_small_vector_move_assignment<non_trivial_ptr_except_copy,size_type,0>()));
  
  BOOST_TEST(( constexpr_test_small_vector_move_assignment<uint8_t,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_move_assignment<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_move_assignment<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( constexpr_test_small_vector_move_assignment<non_trivial_ptr_except_copy,size_type,0>()));
  }
  
//---------------------------------------------------------------------------------------------------------------------
template<typename value_type,
          uint8_t capacity_req =at_least<value_type>(uint8_t(4))>
constexpr bool constexpr_test_at_max_size()
  {
  using size_type = uint8_t;
  
  using st = small_vector<value_type, size_type, capacity_req>;
  
  CONSTEXPR_TEST(st::max_size() == std::numeric_limits<size_type>::max());
  
  std::array<value_type,st::max_size()> test_values;
  std::iota( begin(test_values), std::prev(end(test_values),1), value_type(1) );
  test_values[st::max_size()-1] = std::numeric_limits<size_type>::max();
  
  st v;
  {
  std::span expected{ test_values.data(), st::max_size()-1 };
  insert(v, begin(v), begin(expected), end(expected) );
  CONSTEXPR_TEST(v.active_storage() == dynamic );
  CONSTEXPR_TEST(size(v) == expected.size());
  CONSTEXPR_TEST(equal(v,expected));
  }
  {
  std::span expected{ test_values.data(), st::max_size() };
  emplace_back(v, expected.back() );
  CONSTEXPR_TEST(size(v) == expected.size());
  CONSTEXPR_TEST(equal(v,expected));
  }
  return true;
  }
  
template<typename value_type,
          uint8_t capacity_req =at_least<value_type>(uint8_t(4))>
consteval bool consteval_test_at_max_size()
  { return constexpr_test_at_max_size<value_type,capacity_req>(); }
  
BOOST_AUTO_TEST_CASE( test_at_max_size )
  {
  BOOST_TEST(( consteval_test_at_max_size<uint8_t>()));
  BOOST_TEST(( consteval_test_at_max_size<uint16_t>()));
  BOOST_TEST(( consteval_test_at_max_size<uint32_t>()));
  BOOST_TEST(( consteval_test_at_max_size<uint64_t>()));
  
  BOOST_TEST(( constexpr_test_at_max_size<uint8_t>()));
  BOOST_TEST(( constexpr_test_at_max_size<uint16_t>()));
  BOOST_TEST(( constexpr_test_at_max_size<uint32_t>()));
  BOOST_TEST(( constexpr_test_at_max_size<uint64_t>()));
  BOOST_TEST(( constexpr_test_at_max_size<non_trivial>()));
  BOOST_TEST(( constexpr_test_at_max_size<non_trivial_ptr>()));
  BOOST_TEST(( constexpr_test_at_max_size<non_trivial_ptr_except>()));
  BOOST_TEST(( constexpr_test_at_max_size<non_trivial_ptr_except_copy>()));
  
  BOOST_TEST(( consteval_test_at_max_size<uint8_t,0>()));
  BOOST_TEST(( consteval_test_at_max_size<non_trivial_ptr,0>()));
  BOOST_TEST(( consteval_test_at_max_size<non_trivial_ptr_except,0>()));
  BOOST_TEST(( consteval_test_at_max_size<non_trivial_ptr_except_copy,0>()));
  
  BOOST_TEST(( constexpr_test_at_max_size<uint8_t,0>()));
  BOOST_TEST(( constexpr_test_at_max_size<non_trivial_ptr,0>()));
  BOOST_TEST(( constexpr_test_at_max_size<non_trivial_ptr_except,0>()));
  BOOST_TEST(( constexpr_test_at_max_size<non_trivial_ptr_except_copy,0>()));
  }
  
//---------------------------------------------------------------------------------------------------------
template<typename value_type, typename size_type,
          size_type capacity_req =at_least<value_type>(size_type(20))>
constexpr bool constexpr_small_vector_resize()
  {
  
  using vector_type = small_vector<value_type,size_type,capacity_req>;
  vector_type vec;
    {
    std::array<value_type,2> expected;
    std::iota( begin(expected), end(expected), value_type(1) );
    emplace_back(vec,value_type{1});
    reserve(vec,5);
    CONSTEXPR_TEST(size(vec) == 1);
    emplace_back(vec,value_type{2});
    CONSTEXPR_TEST(equal(vec,expected));
    }
  clear(vec);
    {
    std::array<value_type,10> expected{};
    resize(vec,10);
    CONSTEXPR_TEST(size(vec) == 10);
    CONSTEXPR_TEST(equal(vec,expected));
    }
    {
    resize(vec,20);
    std::array<value_type,20> expected{};
    CONSTEXPR_TEST(size(vec) == 20);
    CONSTEXPR_TEST(equal(vec,expected));
    }
    {
    resize(vec,15);
    std::array<value_type,15> expected{};
    CONSTEXPR_TEST(size(vec) == 15);
    CONSTEXPR_TEST(equal(vec,expected));
    }
  clear(vec);
    {
    std::array<value_type,10> expected;
    std::iota( begin(expected), end(expected), value_type(1) );
    insert(vec, begin(vec), begin(expected), end(expected) );
    CONSTEXPR_TEST(size(vec) == 10);
    CONSTEXPR_TEST(equal(vec,expected));
    }
    {
    resize(vec,15);
    std::array<value_type,15> expected{};
    std::iota( begin(expected), begin(expected)+10, value_type(1) );
    CONSTEXPR_TEST(size(vec) == 15);
    CONSTEXPR_TEST(equal(vec,expected));
    }
    {
    resize(vec,5);
    std::array<value_type,5> expected{};
    std::iota( begin(expected), end(expected), value_type(1) );
    CONSTEXPR_TEST(size(vec) == 5);
    CONSTEXPR_TEST(equal(vec,expected));
    }
    {
    resize(vec,125);
    std::array<value_type,125> expected{};
    std::iota( begin(expected), begin(expected)+5, value_type(1) );
    CONSTEXPR_TEST(size(vec) == 125);
    CONSTEXPR_TEST(equal(vec,expected));
    }
    {
    resize(vec,25);
    std::array<value_type,25> expected{};
    std::iota( begin(expected), begin(expected)+5, value_type(1) );
    CONSTEXPR_TEST(size(vec) == 25);
    CONSTEXPR_TEST(equal(vec,expected));
    }
  return true;
  }
  
template<typename value_type, typename size_type,
          size_type capacity_req =at_least<value_type>(size_type(20))>
consteval bool consteval_small_vector_resize()
  { return constexpr_small_vector_resize<value_type,size_type,capacity_req>(); }
  
BOOST_AUTO_TEST_CASE_TEMPLATE( small_vector_resize_consteval, size_type, constexpr_size_type_traits_list )
  {
  BOOST_TEST(( consteval_small_vector_resize<uint8_t,size_type>()));
  BOOST_TEST(( consteval_small_vector_resize<uint16_t,size_type>()));
  BOOST_TEST(( consteval_small_vector_resize<uint32_t,size_type>()));
  BOOST_TEST(( consteval_small_vector_resize<uint64_t,size_type>()));

  BOOST_TEST(( constexpr_small_vector_resize<uint8_t,size_type>()));
  BOOST_TEST(( constexpr_small_vector_resize<uint16_t,size_type>()));
  BOOST_TEST(( constexpr_small_vector_resize<uint32_t,size_type>()));
  BOOST_TEST(( constexpr_small_vector_resize<uint64_t,size_type>()));
  BOOST_TEST(( constexpr_small_vector_resize<non_trivial,size_type>()));
  BOOST_TEST(( constexpr_small_vector_resize<non_trivial_ptr,size_type>()));
  BOOST_TEST(( constexpr_small_vector_resize<non_trivial_ptr_except,size_type>()));
  BOOST_TEST(( constexpr_small_vector_resize<non_trivial_ptr_except_copy,size_type>()));
  
  
  BOOST_TEST(( consteval_small_vector_resize<uint8_t,size_type,0>()));
  BOOST_TEST(( consteval_small_vector_resize<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( consteval_small_vector_resize<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( consteval_small_vector_resize<non_trivial_ptr_except_copy,size_type,0>()));
  
  BOOST_TEST(( constexpr_small_vector_resize<uint8_t,size_type,0>()));
  BOOST_TEST(( constexpr_small_vector_resize<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( constexpr_small_vector_resize<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( constexpr_small_vector_resize<non_trivial_ptr_except_copy,size_type,0>()));
  }
//---------------------------------------------------------------------------------------------------------------------
template<typename value_type, typename size_type,
          size_type capacity_req =at_least<value_type>(size_type(5))>
constexpr bool constexpr_small_vector_shrink_to_fit()
  {
  using vector_type = small_vector<value_type,size_type,capacity_req>;
  constexpr size_type cap { vector_type::buffered_capacity() };
  constexpr size_type first_growth = detail::growth( capacity_req, size_type(1));
    {
    vector_type vec;
    std::array<value_type,first_growth> expected;
    std::iota( begin(expected), end(expected), value_type(1) );
    insert(vec, begin(vec), begin(expected), end(expected) );
    reserve(vec, 2*first_growth);
    CONSTEXPR_TEST(size(vec) == first_growth);
    CONSTEXPR_TEST(capacity(vec) == 2*first_growth);
    CONSTEXPR_TEST(equal(vec,expected));
    auto res = shrink_to_fit(vec);
    CONSTEXPR_TEST(res == no_error);
    CONSTEXPR_TEST(size(vec) == first_growth);
    CONSTEXPR_TEST(capacity(vec) == first_growth);
    CONSTEXPR_TEST(vec.active_storage() == dynamic );
    CONSTEXPR_TEST(equal(vec,expected));
    }
  if constexpr(expected_istorage(capacity_req)==buffered)
    {
      {
      vector_type vec;
      std::array<value_type,cap> expected;
      std::iota( begin(expected), end(expected), value_type(1) );
      insert(vec, begin(vec), begin(expected), end(expected) );
      CONSTEXPR_TEST(size(vec) == cap);
      reserve(vec, first_growth);
      CONSTEXPR_TEST(size(vec) == cap);
      CONSTEXPR_TEST(capacity(vec) == first_growth);
      CONSTEXPR_TEST(equal(vec,expected));
      auto res = shrink_to_fit(vec);
      CONSTEXPR_TEST(res == no_error);
      CONSTEXPR_TEST(capacity(vec) == cap);
      CONSTEXPR_TEST(size(vec) == cap);
      CONSTEXPR_TEST(vec.active_storage() == buffered );
      CONSTEXPR_TEST(equal(vec,expected));
      }
      {
      vector_type vec;
      std::array<value_type,cap> expected;
      std::iota( begin(expected), end(expected), value_type(1) );
      insert(vec, begin(vec), begin(expected), end(expected) );
      CONSTEXPR_TEST(size(vec) == cap);
      CONSTEXPR_TEST(capacity(vec) == cap);
      CONSTEXPR_TEST(equal(vec,expected));
      auto res = shrink_to_fit(vec);
      CONSTEXPR_TEST(res == no_error);
      CONSTEXPR_TEST(capacity(vec) == cap);
      CONSTEXPR_TEST(vec.active_storage() == buffered );
      CONSTEXPR_TEST(size(vec) == cap);
      CONSTEXPR_TEST(equal(vec,expected));
      }
    }
  return true;
  }
  
template<typename value_type, typename size_type,
          size_type capacity_req =at_least<value_type>(size_type(5))>
consteval bool consteval_small_vector_shrink_to_fit()
  { return constexpr_small_vector_shrink_to_fit<value_type,size_type,capacity_req>();  }
  
BOOST_AUTO_TEST_CASE_TEMPLATE( small_vector_shrink_to_fit, size_type, constexpr_size_type_traits_list )
  {
  BOOST_TEST(( consteval_small_vector_shrink_to_fit<uint8_t,size_type>()));
  BOOST_TEST(( consteval_small_vector_shrink_to_fit<uint16_t,size_type>()));
  BOOST_TEST(( consteval_small_vector_shrink_to_fit<uint32_t,size_type>()));
  BOOST_TEST(( consteval_small_vector_shrink_to_fit<uint64_t,size_type>()));

  BOOST_TEST(( constexpr_small_vector_shrink_to_fit<uint8_t,size_type>()));
  BOOST_TEST(( constexpr_small_vector_shrink_to_fit<uint16_t,size_type>()));
  BOOST_TEST(( constexpr_small_vector_shrink_to_fit<uint32_t,size_type>()));
  BOOST_TEST(( constexpr_small_vector_shrink_to_fit<uint64_t,size_type>()));
  BOOST_TEST(( constexpr_small_vector_shrink_to_fit<non_trivial,size_type>()));
  BOOST_TEST(( constexpr_small_vector_shrink_to_fit<non_trivial_ptr,size_type>()));
  BOOST_TEST(( constexpr_small_vector_shrink_to_fit<non_trivial_ptr_except,size_type>()));
  BOOST_TEST(( constexpr_small_vector_shrink_to_fit<non_trivial_ptr_except_copy,size_type>()));
  
  
  BOOST_TEST(( consteval_small_vector_shrink_to_fit<uint8_t,size_type,0>()));
  BOOST_TEST(( consteval_small_vector_shrink_to_fit<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( consteval_small_vector_shrink_to_fit<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( consteval_small_vector_shrink_to_fit<non_trivial_ptr_except_copy,size_type,0>()));
  
  BOOST_TEST(( constexpr_small_vector_shrink_to_fit<uint8_t,size_type,0>()));
  BOOST_TEST(( constexpr_small_vector_shrink_to_fit<non_trivial_ptr,size_type,0>()));
  BOOST_TEST(( constexpr_small_vector_shrink_to_fit<non_trivial_ptr_except,size_type,0>()));
  BOOST_TEST(( constexpr_small_vector_shrink_to_fit<non_trivial_ptr_except_copy,size_type,0>()));
  }
  
//---------------------------------------------------------------------------------------------------------------------
template<typename value_type, typename size_type,
          size_type capacity_req =at_least<value_type>(size_type(5))>
constexpr bool constexpr_small_vector_shrink_to_fit_except()
  {
  using vector_type = small_vector<value_type,size_type,capacity_req>;
  constexpr size_type cap { vector_type::buffered_capacity() };
  constexpr size_type first_growth = detail::growth( capacity_req, size_type(1));
    {
    vector_type vec;
    std::array<value_type,first_growth> expected;
    std::iota( begin(expected), end(expected), value_type(1) );
    insert(vec, begin(vec), begin(expected), end(expected) );
    reserve(vec, 2*first_growth);
    CONSTEXPR_TEST(size(vec) == first_growth);
    CONSTEXPR_TEST(capacity(vec) == 2*first_growth);
    CONSTEXPR_TEST(equal(vec,expected));
    back(vec).set_value(128);
    expected.back().set_value(128);
    bool except = false;
    try {
      shrink_to_fit(vec);
      }
    catch(...)
      {except = true; }
    CONSTEXPR_TEST(except);
    CONSTEXPR_TEST(size(vec) == first_growth);
    CONSTEXPR_TEST(equal(vec,expected));
    }
  if constexpr(expected_istorage(capacity_req)==buffered)
    {
      {
      vector_type vec;
      std::array<value_type,cap> expected;
      std::iota( begin(expected), end(expected), value_type(1) );
      insert(vec, begin(vec), begin(expected), end(expected) );
      CONSTEXPR_TEST(size(vec) == cap);
      reserve(vec, first_growth);
      CONSTEXPR_TEST(size(vec) == cap);
      CONSTEXPR_TEST(capacity(vec) == first_growth);
      CONSTEXPR_TEST(equal(vec,expected));
      back(vec).set_value(128);
      expected.back().set_value(128);
      bool except = false;
      try {
        shrink_to_fit(vec);
        }
      catch(...)
        {except = true; }
      CONSTEXPR_TEST(except);
      CONSTEXPR_TEST(size(vec) == cap);
      CONSTEXPR_TEST(equal(vec,expected));
      }
      {
      vector_type vec;
      std::array<value_type,cap> expected;
      std::iota( begin(expected), end(expected), value_type(1) );
      insert(vec, begin(vec), begin(expected), end(expected) );
      CONSTEXPR_TEST(size(vec) == cap);
      CONSTEXPR_TEST(capacity(vec) == cap);
      CONSTEXPR_TEST(equal(vec,expected));
      back(vec).set_value(128);
      expected.back().set_value(128);
      bool except = false;
      try {
        shrink_to_fit(vec);
        }
      catch(...)
        {except = true; }
      CONSTEXPR_TEST(not except);
      CONSTEXPR_TEST(size(vec) == cap);
      CONSTEXPR_TEST(equal(vec,expected));
      }
    }
  return true;
  }
BOOST_AUTO_TEST_CASE_TEMPLATE( small_vector_shrink_to_fit_except, size_type, constexpr_size_type_traits_list )
  {
  BOOST_TEST(( constexpr_small_vector_shrink_to_fit_except<non_trivial_throwing<128>,size_type>()));
  BOOST_TEST(( constexpr_small_vector_shrink_to_fit_except<non_trivial_throwing_copy<128>,size_type>()));

  BOOST_TEST(( constexpr_small_vector_shrink_to_fit_except<non_trivial_throwing<128>,size_type,0>()));
  BOOST_TEST(( constexpr_small_vector_shrink_to_fit_except<non_trivial_throwing_copy<128>,size_type,0>()));
  }
}
