#include <coll/static_vector.h>
#include <unit_test_core.h>

#include <atomic>
#include <iostream>

//____________________________________________________________________________//

namespace coll
{
// struct non_trivial
//   {
//   int value_;
//   
//   constexpr non_trivial() noexcept : value_{} {}
//   constexpr non_trivial( int value ) noexcept : value_{value} {}
//   
//   constexpr non_trivial( non_trivial && r ) noexcept
//       : value_{r.value_}
//     {
//     r.value_ = {};
//     }
//   constexpr non_trivial( non_trivial const & r) noexcept
//       : value_{r.value_}
//     {
//     }
//   constexpr non_trivial & operator =( non_trivial && r ) noexcept
//     {
//     std::swap(value_,r.value_);
//     return *this;
//     }
//   constexpr non_trivial & operator =( non_trivial const & r ) noexcept
//     {
//     value_ = r.value_;
//     return *this;
//     }
//   
//   constexpr ~non_trivial() {}
//   constexpr bool operator ==( non_trivial const & r ) const noexcept = default;
//   constexpr bool operator ==( int r ) const noexcept { return value_ == r; }
//   };
  
struct non_trivial_counter : public non_trivial
  {
  static std::atomic<int> counter_;
  bool has_resource {};
  
  non_trivial_counter() noexcept : has_resource{} {}
  non_trivial_counter( int value ) noexcept 
    : non_trivial{value}, has_resource{true}
    { ++counter_;}
  
  non_trivial_counter( non_trivial_counter && r ) noexcept
    : non_trivial{ std::move(r) }, has_resource{r.has_resource}
    {
    if(r.has_resource)
      r.has_resource = false;
    }
    
  non_trivial_counter( non_trivial_counter const & r ) noexcept
    : non_trivial{ r }, has_resource{r.has_resource}
    {
    if(has_resource)
      ++counter_;
    }
    
  non_trivial_counter & operator =( non_trivial_counter && r ) noexcept
    {
    non_trivial::operator =(std::move(r));
    if(has_resource)
      {
      --counter_;
      has_resource= false;
      }
    has_resource = std::exchange(r.has_resource, false);
    return *this;
    }
    
  non_trivial_counter & operator =( non_trivial_counter const & r ) noexcept
    {
    non_trivial::operator =(r);
    if(has_resource)
      {
      --counter_;
      has_resource= false;
      }
    if(r.has_resource)
      {
      has_resource=true;
      ++counter_;
      }
    return *this;
    }
  
  ~non_trivial_counter() 
    {
    if(has_resource)
      --counter_;
    }
  bool operator ==( non_trivial_counter const & r ) const noexcept
    {
    return has_resource == r.has_resource  && non_trivial::operator==(r);
    }
  };
std::atomic<int> non_trivial_counter::counter_{};

                                  
using traits_list = boost::mpl::list<uint16_t, int32_t, int64_t, double, 
                                     non_trivial, non_trivial_ptr, non_trivial_ptr_except,
                                     non_trivial_ptr_except_copy>;
using traits_list_move = boost::mpl::list<uint16_t, int32_t, int64_t, double, 
                                     non_trivial, non_trivial_ptr, non_trivial_ptr_except>;
using constexpr_traits_list = boost::mpl::list<uint16_t, int32_t, int64_t, double>;

constexpr bool equal( auto const & result, auto const & expected ) noexcept
  {
  return std::equal(begin(result),end(result),
                    begin(expected), end(expected));
  }
  

//---------------------------------------------------------------------------------------------------------
namespace concepts
{
struct invalid_static_vector
  {
  using value_type = int;
  static constexpr unsigned capacity = 10;
  };

static_assert( unsigned_arithmetic_integral<unsigned> );
static_assert( !unsigned_arithmetic_integral<bool> );
static_assert( !unsigned_arithmetic_integral<int> );
static_assert( !unsigned_arithmetic_integral<float> );
static_assert( same_as_static_vector<static_vector<uint8_t,7>>);
static_assert( same_as_static_vector<const static_vector<uint8_t,7>>);
static_assert( not same_as_static_vector<std::vector<uint8_t>>);
static_assert( not same_as_static_vector<invalid_static_vector>);

namespace method_tests
{
  using vec_type = static_vector<uint8_t,7>;
  static_assert( std::same_as<uint8_t &, decltype( back(std::declval<vec_type &>()))> );
  static_assert( std::same_as<uint8_t const &, decltype( back(std::declval<vec_type const &>()))> );
}
}
static_assert( sizeof(static_vector<uint8_t,7>) == 8 );
static_assert( sizeof(static_vector<uint32_t,3>) == 16 );
struct aligned_3_byte_struct 
  {
  std::byte arr[3];
  };
static_assert( sizeof(static_vector<aligned_3_byte_struct,3>) == 10 );
//---------------------------------------------------------------------------------------------------------
template<typename value_type>
bool consteval consteval_static_vector_deduced_types()
  {
  using vec_type = static_vector<value_type,7>;
  using iterator = typename vec_type::iterator;
  using const_iterator = typename vec_type::const_iterator;
  vec_type v;
  vec_type const & vc{v};
  vec_type & vr{v};

  CONSTEXPR_TEST(( std::same_as<value_type &, decltype( at( v,0u ) )> ));
  CONSTEXPR_TEST(( std::same_as<value_type &, decltype( at( vr,2u ) )> ));
  CONSTEXPR_TEST(( std::same_as<value_type const &, decltype( at(vc, 1u) )> )) ;

  CONSTEXPR_TEST(( std::same_as<iterator, decltype( begin( v ) )> ));
  CONSTEXPR_TEST(( std::same_as<iterator, decltype( begin( vr ) )> ));
  CONSTEXPR_TEST(( std::same_as<const_iterator, decltype( begin( vc ) )> ));

  CONSTEXPR_TEST(( std::same_as<iterator, decltype( end( v ) )> ));
  CONSTEXPR_TEST(( std::same_as<iterator, decltype( end( vr ) )> ));
  CONSTEXPR_TEST(( std::same_as<const_iterator, decltype( end( vc ) )> ));
  return true;
  }

static_assert(consteval_static_vector_deduced_types<int32_t>());
static_assert(consteval_static_vector_deduced_types<aligned_3_byte_struct>());

//---------------------------------------------------------------------------------------------------------
template<typename value_type>
constexpr bool constexpr_static_vector_basic()
  {
  auto constexpr elements = 10;
  using vector_type = static_vector<value_type,elements>;
  using enum vector_tune_e;
  using enum vector_outcome_e;

  vector_type vec;
  CONSTEXPR_TEST(size(vec) == 0);
  CONSTEXPR_TEST(capacity(vec) == elements );
  CONSTEXPR_TEST(free_space(vec) == elements );
  CONSTEXPR_TEST( (begin(vec) == end(vec)) );
  
  CONSTEXPR_TEST((std::same_as<vector_outcome_e,decltype(emplace_back(vec, 1))>));
  auto res = emplace_back(vec, 1);
  CONSTEXPR_TEST( (res == no_error) );
  CONSTEXPR_TEST(size(vec) == 1);
  CONSTEXPR_TEST(free_space(vec) == elements-1 );
  CONSTEXPR_TEST( vec[0u] == 1);
  
  emplace_back<unchecked>(vec, 2);
  CONSTEXPR_TEST((std::same_as<void,decltype(emplace_back<unchecked>(vec, 1))>));

  CONSTEXPR_TEST(size(vec) == 2);
  CONSTEXPR_TEST(free_space(vec) == elements-2 );
  CONSTEXPR_TEST( vec[0u] == 1);
  CONSTEXPR_TEST( vec[1u] == 2);
  
  res = emplace_back(vec, 3);
  CONSTEXPR_TEST( (res == no_error) );
  CONSTEXPR_TEST(size(vec) == 3);
  CONSTEXPR_TEST(free_space(vec) == elements-3 );
  CONSTEXPR_TEST( vec[0u] == 1);
  CONSTEXPR_TEST( vec[1u] == 2);
  CONSTEXPR_TEST( vec[2u] == 3);
  
  res = emplace_back(vec, 4);
  CONSTEXPR_TEST( (res == no_error) );
  CONSTEXPR_TEST(size(vec) == 4);
  CONSTEXPR_TEST(free_space(vec) == elements-4 );
  CONSTEXPR_TEST( std::distance(begin(vec), end(vec)) == size(vec) );
  CONSTEXPR_TEST( vec[0u] == 1);
  CONSTEXPR_TEST( vec[1u] == 2);
  CONSTEXPR_TEST( vec[2u] == 3);
  CONSTEXPR_TEST( vec[3u] == 4);
  return true;
  }
  
template<typename value_type>
consteval bool consteval_static_vector_basic()
  { return constexpr_static_vector_basic<value_type>();}
  
BOOST_AUTO_TEST_CASE_TEMPLATE( static_vector_basic_consteval, value_type, constexpr_traits_list )
  {
  BOOST_TEST((consteval_static_vector_basic<value_type>()));
  }
BOOST_AUTO_TEST_CASE_TEMPLATE( static_vector_basic, value_type, traits_list )
  {
  BOOST_TEST((constexpr_static_vector_basic<value_type>()));
  }
//---------------------------------------------------------------------------------------------------------
template<typename value_type>
constexpr bool constexpr_static_vector_move()
  {
  auto constexpr elements = 10;
  using vector_type = static_vector<value_type,elements>;
  vector_type vec;
  emplace_back(vec, 1);
  emplace_back(vec, 2);
  emplace_back(vec, 3);
  auto res = emplace_back(vec, 4);
  CONSTEXPR_TEST( (res == vector_outcome_e::no_error) );
  CONSTEXPR_TEST(size(vec) == 4);
  CONSTEXPR_TEST( std::distance(begin(vec), end(vec)) == size(vec) );
  std::array<value_type,4> tst{ 1,2,3,4};
  CONSTEXPR_TEST(equal(vec,tst));
  
  vector_type vec2{ std::move(vec) };
  CONSTEXPR_TEST(size(vec) == 0);
  CONSTEXPR_TEST(size(vec2) == 4);
  CONSTEXPR_TEST(equal(vec2,tst));
  
  return true;
  }
template<typename value_type>
consteval bool consteval_static_vector_move()
  { return constexpr_static_vector_move<value_type>(); }
BOOST_AUTO_TEST_CASE_TEMPLATE( static_vector_move_consteval, value_type, constexpr_traits_list )
  {
  BOOST_TEST((consteval_static_vector_move<value_type>()));
  }
BOOST_AUTO_TEST_CASE_TEMPLATE( static_vector_move, value_type, traits_list )
  {
  BOOST_TEST((constexpr_static_vector_move<value_type>()));
  }
//---------------------------------------------------------------------------------------------------------
template<typename value_type>
constexpr bool constexpr_static_vector_copy()
  {
  auto constexpr elements = 10;
  using vector_type = static_vector<value_type,elements>;
  vector_type vec;
  emplace_back(vec, 1);
  emplace_back(vec, 2);
  emplace_back(vec, 3);
  auto res = emplace_back(vec, 4);
  CONSTEXPR_TEST( (res == vector_outcome_e::no_error) );
  CONSTEXPR_TEST(size(vec) == 4);
  CONSTEXPR_TEST( std::distance(begin(vec), end(vec)) == size(vec) );
  std::array<value_type,4> tst{1,2,3,4};
  CONSTEXPR_TEST(equal(vec,tst));
  
  vector_type vec2{ const_cast<vector_type const &>(vec) };
  CONSTEXPR_TEST(size(vec) == 4);
  CONSTEXPR_TEST(size(vec2) == 4);
  CONSTEXPR_TEST(equal(vec,tst));
  CONSTEXPR_TEST(equal(vec2,tst));
  
  return true;
  }
  
template<typename value_type>
consteval bool consteval_static_vector_copy()
  { return constexpr_static_vector_copy<value_type>(); }
  
BOOST_AUTO_TEST_CASE_TEMPLATE( static_vector_copy_consteval, value_type, constexpr_traits_list )
  {
  BOOST_TEST((consteval_static_vector_copy<value_type>()));
  }
BOOST_AUTO_TEST_CASE_TEMPLATE( static_vector_copy, value_type, traits_list )
  {
  BOOST_TEST((constexpr_static_vector_copy<value_type>()));
  }
//---------------------------------------------------------------------------------------------------------
template<typename value_type>
constexpr bool constexpr_static_vector_emplace()
  {
  auto constexpr elements = 20;
  using vector_type = static_vector<value_type,elements>;
  using enum vector_outcome_e;
  using enum vector_tune_e;

  vector_type vec;
    {
    std::array<value_type,1> expected{1};
    value_type v = 1;
    auto res = emplace(vec, begin(vec), v );
    CONSTEXPR_TEST((std::same_as<vector_outcome_e,decltype(emplace(vec, begin(vec), v ))>));
    CONSTEXPR_TEST( (res == no_error) );
    CONSTEXPR_TEST( std::distance(begin(vec), end(vec)) == size(vec) );
    CONSTEXPR_TEST(size(vec) == 1);
    CONSTEXPR_TEST(equal(vec,expected));
    }
    {
    std::array<value_type,2> expected{2,1};
    value_type v = 2;
    emplace<unchecked>(vec, begin(vec), std::move(v) );
    CONSTEXPR_TEST((std::same_as<void,decltype(emplace<unchecked>(vec, begin(vec), v ))>));

    CONSTEXPR_TEST(size(vec) == 2);
    CONSTEXPR_TEST(equal(vec,expected));
    }
    {
    std::array<value_type,3> expected{2,3,1};
    value_type v = 3;
    auto res = emplace(vec, begin(vec)+1, std::move(v) );
    CONSTEXPR_TEST( (res == no_error) );
    CONSTEXPR_TEST(size(vec) == 3);
    CONSTEXPR_TEST(equal(vec,expected));
    }
    {
    std::array<value_type,4> expected{2,4,3,1};
    value_type v = 4;
    auto res = emplace(vec, begin(vec)+1, std::move(v) );
    CONSTEXPR_TEST( (res == no_error) );
    CONSTEXPR_TEST(size(vec) == 4);
    CONSTEXPR_TEST(equal(vec,expected));
    }
    {
    std::array<value_type,5> expected{2,4,3,1,5};
    value_type v = 5;
    auto res = emplace(vec, end(vec), std::move(v) );
    CONSTEXPR_TEST( (res == no_error) );
    CONSTEXPR_TEST(size(vec) == 5);
    CONSTEXPR_TEST(equal(vec,expected));
    }
  return true;
  }
  
template<typename value_type>
consteval bool consteval_static_vector_emplace()
  { return constexpr_static_vector_emplace<value_type>(); }
  
BOOST_AUTO_TEST_CASE_TEMPLATE( static_vector_emplace_consteval, value_type, constexpr_traits_list )
  {
  BOOST_TEST((consteval_static_vector_emplace<value_type>()));
  }
BOOST_AUTO_TEST_CASE_TEMPLATE( static_vector_test_emplace, value_type, traits_list )
  {
  BOOST_TEST((constexpr_static_vector_emplace<value_type>()));
  }
//---------------------------------------------------------------------------------------------------------
template<typename value_type>
constexpr bool constexpr_static_vector_insert()
  {
  auto constexpr elements = 20;
  using vector_type = static_vector<value_type,elements>;
  vector_type vec;
    {
    std::array<value_type,5> tst{1,2,3,4,5};
    auto res = insert(vec, begin(vec), begin(tst), end(tst) );
    CONSTEXPR_TEST( (res == vector_outcome_e::no_error) );
    CONSTEXPR_TEST(size(vec) == 5);
    CONSTEXPR_TEST(equal(vec,tst));
    }
    {
    std::array<value_type,3> tst{11,12,13};
    std::array<value_type,8> exp{11,12,13,1,2,3,4,5};
    auto res = insert(vec, begin(vec), begin(tst), end(tst) );
    CONSTEXPR_TEST( (res == vector_outcome_e::no_error) );
    CONSTEXPR_TEST(size(vec) == 8);
    CONSTEXPR_TEST(equal(vec,exp));
    }
    {
    std::array<value_type,2> tst{21,22};
    std::array<value_type,10> exp{11,12,21,22,13,1,2,3,4,5};
    auto res = insert(vec, begin(vec)+2, begin(tst), end(tst) );
    CONSTEXPR_TEST( (res == vector_outcome_e::no_error) );
    CONSTEXPR_TEST(size(vec) == 10);
    CONSTEXPR_TEST(equal(vec,exp));
    }
    {
    std::array<value_type,4> tst{31,32,33,34};
    std::array<value_type,14> exp{11,12,21,22,13,1,31,32,33,34,2,3,4,5};
    auto res = insert(vec, begin(vec)+6, begin(tst), end(tst) );
    CONSTEXPR_TEST( (res == vector_outcome_e::no_error) );
    CONSTEXPR_TEST(size(vec) == 14);
    CONSTEXPR_TEST(equal(vec,exp));
    }
    {
    std::array<value_type,4> tst{41,42,43,44};
    std::array<value_type,18> exp{11,12,21,22,13,1,31,32,33,34,2,3,41,42,43,44,4,5};
    auto res = insert(vec, begin(vec)+12, begin(tst), end(tst) );
    CONSTEXPR_TEST( (res == vector_outcome_e::no_error) );
    CONSTEXPR_TEST(size(vec) == 18);
    CONSTEXPR_TEST(equal(vec,exp));
    }
    {
    std::array<value_type,2> tst{51,52};
    std::array<value_type,20> exp{11,12,21,22,13,1,31,32,33,34,2,3,41,42,43,44,4,5,51,52};
    auto res = insert(vec, end(vec), begin(tst), end(tst) );
//     dump(vec);
    CONSTEXPR_TEST( (res == vector_outcome_e::no_error) );
    CONSTEXPR_TEST(size(vec) == 20);
    CONSTEXPR_TEST(equal(vec,exp));
    }
    {
    std::array<value_type,2> tst{51,52};
    std::array<value_type,20> exp{11,12,21,22,13,1,31,32,33,34,2,3,41,42,43,44,4,5,51,52};
    auto res = insert(vec, end(vec), begin(tst), end(tst) );
//     dump(vec);
    CONSTEXPR_TEST( (res == vector_outcome_e::out_of_storage) );
    CONSTEXPR_TEST(size(vec) == 20);
    CONSTEXPR_TEST(equal(vec,exp));
    }
  return true;
  }
template<typename value_type>
consteval bool consteval_static_vector_insert()
  { return constexpr_static_vector_insert<value_type>(); }
  
BOOST_AUTO_TEST_CASE_TEMPLATE( static_vector_test_insert_consteval, value_type, constexpr_traits_list )
  {
  BOOST_TEST((consteval_static_vector_insert<value_type>()));
  }
BOOST_AUTO_TEST_CASE_TEMPLATE( static_vector_test_insert, value_type, traits_list )
  {
  BOOST_TEST((constexpr_static_vector_insert<value_type>()));
  }
//---------------------------------------------------------------------------------------------------------
template<typename value_type>
constexpr bool constexpr_static_vector_test_insert_moving()
  {
  using std::make_move_iterator;
    {
    auto constexpr elements = 20;
    using vector_type = static_vector<value_type,elements>;
    vector_type vec;
      {
      std::array<value_type,5> tst{1,2,3,4,5};
      std::array<value_type,5> exp{1,2,3,4,5};
      std::array<value_type,5> exp2{};
      auto res = insert(vec, begin(vec), make_move_iterator(begin(tst)), make_move_iterator(end(tst)) );
      CONSTEXPR_TEST(res == vector_outcome_e::no_error);
      CONSTEXPR_TEST(size(vec) == 5);
      CONSTEXPR_TEST(equal(vec,exp));
      //trivial dont clean up after themselfs
      if constexpr(not std::is_trivially_destructible_v<value_type>)
        CONSTEXPR_TEST(equal(tst,exp2));
      }
      {
      std::array<value_type,3> tst{11,12,13};
      std::array<value_type,8> exp{11,12,13,1,2,3,4,5};
      auto res = insert(vec, begin(vec), make_move_iterator(begin(tst)), make_move_iterator(end(tst)) );
      CONSTEXPR_TEST(res == vector_outcome_e::no_error);
      CONSTEXPR_TEST(size(vec) == 8);
      CONSTEXPR_TEST(equal(vec,exp));
      }
    }
  return true;
  }
  
template<typename value_type>
consteval bool consteval_static_vector_test_insert_moving()
  { return constexpr_static_vector_test_insert_moving<value_type>(); }
///gcc libstdc++ bug missing constexpr
///https://gcc.gnu.org/bugzilla/show_bug.cgi?id=102358
BOOST_AUTO_TEST_CASE_TEMPLATE( static_vector_test_insert_moving_consteval, value_type, constexpr_traits_list )
  {
//   BOOST_TEST((consteval_static_vector_test_insert_moving<value_type>()));
  BOOST_TEST((constexpr_static_vector_test_insert_moving<value_type>()));
  }
BOOST_AUTO_TEST_CASE_TEMPLATE( static_vector_test_insert_moving, value_type, traits_list_move )
  {
  BOOST_TEST((constexpr_static_vector_test_insert_moving<value_type>()));
  }

//---------------------------------------------------------------------------------------------------------
template<typename value_type>
constexpr bool constexpr_static_vector_erase_at_end()
  {
  auto constexpr elements = 20;
  using vector_type = static_vector<value_type,elements>;
  vector_type vec;
  std::array<value_type,20> tst{11,12,21,22,13,1,31,32,33,34,2,3,41,42,43,44,4,5,51,52};
  insert(vec, end(vec), begin(tst), end(tst));
  auto res = erase_at_end(vec, begin(vec));
  CONSTEXPR_TEST(size(vec) == 0);
  CONSTEXPR_TEST(end(vec) == res);
  
  insert(vec, end(vec), begin(tst), end(tst));
  res = erase_at_end(vec, begin(vec)+5);
  CONSTEXPR_TEST(size(vec) == 5);
  CONSTEXPR_TEST(begin(vec)+5 == res);
  CONSTEXPR_TEST(equal(vec, std::span(begin(tst), 5)));
  return true;
  }
template<typename value_type>
consteval bool consteval_static_vector_erase_at_end()
  { return constexpr_static_vector_erase_at_end<value_type>(); }
  
BOOST_AUTO_TEST_CASE_TEMPLATE( static_vector_test_erase_at_end_consteval, value_type, constexpr_traits_list )
  {
  BOOST_TEST(consteval_static_vector_erase_at_end<value_type>());
  }
BOOST_AUTO_TEST_CASE_TEMPLATE( static_vector_test_erase_at_end, value_type, traits_list )
  {
  BOOST_TEST(constexpr_static_vector_erase_at_end<value_type>());
  }
//---------------------------------------------------------------------------------------------------------
template<typename value_type>
constexpr bool constexpr_static_vector_erase_range()
  {
  auto constexpr elements = 20;
  using vector_type = static_vector<value_type,elements>;
  vector_type vec;
  std::array<value_type,20> tst{11,12,21,22,13,1,31,32,33,34,2,3,41,42,43,44,4,5,51,52};
  insert(vec, end(vec), begin(tst), end(tst));
  
  auto res = erase(vec, begin(vec)+2, begin(vec)+4 );
  CONSTEXPR_TEST(size(vec) == 18);
  CONSTEXPR_TEST( res == begin(vec)+2 );
  CONSTEXPR_TEST( at(vec, 2u) == 13 );
  CONSTEXPR_TEST( at(vec, 3u) == 1 );
  CONSTEXPR_TEST( at(vec, 4u) == 31 );
  CONSTEXPR_TEST( at(vec, 17u) == 52 );
  
  res = erase(vec, begin(vec)+3, begin(vec)+3 );
  CONSTEXPR_TEST(size(vec) == 18);
  CONSTEXPR_TEST( res == begin(vec)+3 );
  CONSTEXPR_TEST( at(vec, 2u) == 13 );
  CONSTEXPR_TEST( at(vec, 3u) == 1 );
  CONSTEXPR_TEST( at(vec, 4u) == 31 );
  CONSTEXPR_TEST( at(vec, 17u) == 52 );

  return true;
  }
  
template<typename value_type>
consteval bool consteval_static_vector_erase_range()
  { return constexpr_static_vector_erase_range<value_type>(); }
  
BOOST_AUTO_TEST_CASE_TEMPLATE( static_vector_test_erase_range_consteval, value_type, constexpr_traits_list )
  {
  BOOST_TEST(consteval_static_vector_erase_range<value_type>());
  }
BOOST_AUTO_TEST_CASE_TEMPLATE( static_vector_test_erase_range, value_type, traits_list )
  {
  BOOST_TEST(constexpr_static_vector_erase_range<value_type>());
  }
//---------------------------------------------------------------------------------------------------------
template<typename value_type>
constexpr bool constexpr_static_vector_erase()
  {
  auto constexpr elements = 20;
  using vector_type = static_vector<value_type,elements>;
  vector_type vec;
  std::array<value_type,20> tst{11,12,21,22,13,1,31,32,33,34,2,3,41,42,43,44,4,5,51,52};
  insert(vec, end(vec), begin(tst), end(tst));
  
  auto res = erase(vec, begin(vec)+2 );
  CONSTEXPR_TEST(size(vec) == 19);
  CONSTEXPR_TEST( res == begin(vec)+2 );
  CONSTEXPR_TEST( at(vec, 2u) == 22 );
  CONSTEXPR_TEST( at(vec, 3u) == 13 );
  CONSTEXPR_TEST( at(vec, 4u) == 1 );
  CONSTEXPR_TEST( at(vec, 18u) == 52 );

  res = erase(vec, end(vec)-1 );
  CONSTEXPR_TEST( res == end(vec) );
  CONSTEXPR_TEST(size(vec) == 18);
  CONSTEXPR_TEST( at(vec, 17u) == 51 );
  
  res = erase(vec, begin(vec) );
  CONSTEXPR_TEST( res == begin(vec) );
  CONSTEXPR_TEST(size(vec) == 17);
  CONSTEXPR_TEST( at(vec, 0u) == 12 );
  CONSTEXPR_TEST( at(vec, 16u) == 51 );
  
  return true;
  }
template<typename value_type>
consteval bool consteval_static_vector_erase()
  { return constexpr_static_vector_erase<value_type>(); }
    
BOOST_AUTO_TEST_CASE_TEMPLATE( static_vector_test_erase_consteval, value_type, constexpr_traits_list )
  {
  BOOST_TEST(consteval_static_vector_erase<value_type>());
  }
  
BOOST_AUTO_TEST_CASE_TEMPLATE( static_vector_test_erase, value_type, traits_list )
  {
  BOOST_TEST(constexpr_static_vector_erase<value_type>());
  }
//---------------------------------------------------------------------------------------------------------
template<typename value_type>
constexpr bool constexpr_static_vector_split_by_half()
  {
    {
    auto constexpr elements = 20;
    using vector_type = static_vector<value_type,elements>;
    vector_type vec;
    std::array<value_type,20> tst{11,12,21,22,13,1,31,32,33,34,2,3,41,42,43,44,4,5,51,52};
    insert(vec, end(vec), begin(tst), end(tst));
    vector_type vec2;
    auto res = split_by_half(vec,vec2);
    CONSTEXPR_TEST( (res == vector_outcome_e::no_error) );
    CONSTEXPR_TEST(size(vec) == 10);
    CONSTEXPR_TEST(size(vec2) == 10);
    CONSTEXPR_TEST(equal(vec, std::span(begin(tst), 10)));
    CONSTEXPR_TEST(equal(vec2, std::span(begin(tst)+10, 10)));
    }
    {
    auto constexpr elements = 20;
    using vector_type = static_vector<value_type,elements>;
    vector_type vec;
    std::array<value_type,20> tst{11,12,21,22,13,1,31,32,33,34,2,3,41,42,43,44,4,5,51,52};
    insert(vec, end(vec), begin(tst), end(tst));
    vector_type vec2;
    insert(vec2, end(vec2), begin(tst), begin(tst)+10);
    auto res = split_by_half(vec,vec2);
    CONSTEXPR_TEST( (res == vector_outcome_e::no_error) );
    CONSTEXPR_TEST(size(vec) == 10);
    CONSTEXPR_TEST(size(vec2) == 20);
    CONSTEXPR_TEST(equal(vec, std::span(begin(tst), 10)));
    CONSTEXPR_TEST(equal(vec2, std::span(begin(tst), 20)));
    }
  return true;
  }
template<typename value_type>
consteval bool consteval_static_vector_split_by_half()
  { return constexpr_static_vector_split_by_half<value_type>(); }
    
BOOST_AUTO_TEST_CASE_TEMPLATE( static_vector_test_split_by_half_consteval, value_type, constexpr_traits_list )
  {
  BOOST_TEST(consteval_static_vector_split_by_half<value_type>());
  }
BOOST_AUTO_TEST_CASE_TEMPLATE( static_vector_test_split_by_half, value_type, traits_list )
{
  BOOST_TEST(constexpr_static_vector_split_by_half<value_type>());
}
//---------------------------------------------------------------------------------------------------------
template<typename value_type>
constexpr bool constexpr_static_vector_resize()
  {
  auto constexpr elements = 20;
  using vector_type = static_vector<value_type,elements>;
  vector_type vec;
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

  return true;
  }
template<typename value_type>
consteval bool consteval_static_vector_resize()
  { return constexpr_static_vector_resize<value_type>(); }
  
BOOST_AUTO_TEST_CASE_TEMPLATE( static_vector_resize_consteval, value_type, constexpr_traits_list )
  {
  BOOST_TEST(consteval_static_vector_resize<value_type>());
  }
BOOST_AUTO_TEST_CASE_TEMPLATE( static_vector_resize, value_type, traits_list )
{
  BOOST_TEST(constexpr_static_vector_resize<value_type>());
}

}
