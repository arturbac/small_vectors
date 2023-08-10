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

                                  
using traits_list = metatests::type_list<uint16_t, int32_t, int64_t, double,
                                     non_trivial, non_trivial_ptr, non_trivial_ptr_except,
                                     non_trivial_ptr_except_copy>;
using traits_list_move = metatests::type_list<uint16_t, int32_t, int64_t, double,
                                     non_trivial, non_trivial_ptr, non_trivial_ptr_except>;
using constexpr_traits_list = metatests::type_list<uint16_t, int32_t, int64_t, double>;

constexpr bool equal( auto const & result, auto const & expected ) noexcept
  {
  return std::equal(begin(result),end(result),
                    begin(expected), end(expected));
  }
  
using namespace metatests;

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

static_assert( detail::detail_concepts::vector_storage<detail::static_vector_storage<uint8_t,7>>);

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
test_result consteval consteval_static_vector_deduced_types()
  {
  using vec_type = static_vector<value_type,7>;
  using iterator = typename vec_type::iterator;
  using const_iterator = typename vec_type::const_iterator;
  vec_type v;
  vec_type const & vc{v};
  vec_type & vr{v};
  test_result res =
  constexpr_test(( std::same_as<value_type &, decltype( at( v,0u ) )> ))
  | constexpr_test(( std::same_as<value_type &, decltype( at( vr,2u ) )> ))
  | constexpr_test(( std::same_as<value_type const &, decltype( at(vc, 1u) )> ))

  | constexpr_test(( std::same_as<iterator, decltype( begin( v ) )> ))
  | constexpr_test(( std::same_as<iterator, decltype( begin( vr ) )> ))
  | constexpr_test(( std::same_as<const_iterator, decltype( begin( vc ) )> ))

  | constexpr_test(( std::same_as<iterator, decltype( end( v ) )> ))
  | constexpr_test(( std::same_as<iterator, decltype( end( vr ) )> ))
  | constexpr_test(( std::same_as<const_iterator, decltype( end( vc ) )> ));
  return res;
  }

static_assert(consteval_static_vector_deduced_types<int32_t>());
static_assert(consteval_static_vector_deduced_types<aligned_3_byte_struct>());
}
using namespace coll;
using boost::ut::operator""_test;

//---------------------------------------------------------------------------------------------------------
int main()
  {
  test_result result;
  "test_static_vector_basic"_test = [&result]
    {
    auto fn_tmpl = []<typename value_type>( value_type const * ) -> metatests::test_result
      {
      auto constexpr elements = 10;
      using vector_type = static_vector<value_type,elements>;
      using enum vector_tune_e;
      using enum vector_outcome_e;
      test_result tr;

      vector_type vec;
      tr |= constexpr_test(size(vec) == 0)
          | constexpr_test(capacity(vec) == elements )
          | constexpr_test(free_space(vec) == elements )
          | constexpr_test( (begin(vec) == end(vec)) );

      tr |= constexpr_test((std::same_as<vector_outcome_e,decltype(emplace_back(vec, 1))>));

      auto res = emplace_back(vec, 1);
      tr |= constexpr_test( (res == no_error) )
          | constexpr_test(size(vec) == 1)
          | constexpr_test(free_space(vec) == elements-1 )
          | constexpr_test( vec[0u] == 1);

      emplace_back<unchecked>(vec, 2);
      tr |= constexpr_test((std::same_as<void,decltype(emplace_back<unchecked>(vec, 1))>));

      tr |= constexpr_test(size(vec) == 2)
          | constexpr_test(free_space(vec) == elements-2 )
          | constexpr_test( vec[0u] == 1)
          | constexpr_test( vec[1u] == 2);

      res = emplace_back(vec, 3);
      tr |= constexpr_test( (res == no_error) )
          | constexpr_test(size(vec) == 3)
          | constexpr_test(free_space(vec) == elements-3 )
          | constexpr_test( vec[0u] == 1)
          | constexpr_test( vec[1u] == 2)
          | constexpr_test( vec[2u] == 3);

      res = vec.emplace_back(4);
      tr |= constexpr_test( (res == no_error) )
          | constexpr_test(size(vec) == 4)
          | constexpr_test(free_space(vec) == elements-4 )
          | constexpr_test( std::distance(begin(vec), end(vec)) == size(vec) )
          | constexpr_test( vec[0u] == 1)
          | constexpr_test( vec[1u] == 2)
          | constexpr_test( vec[2u] == 3)
          | constexpr_test( vec[3u] == 4);

      return tr;
      };
    result |= run_constexpr_test<traits_list>(fn_tmpl);
    result |= run_consteval_test<constexpr_traits_list>(fn_tmpl);
    };

  "test_static_vector_push_back"_test = [&result]
    {
    auto fn_tmpl = []<typename value_type>( value_type const * ) -> metatests::test_result
      {
      auto constexpr elements = 10;
      using vector_type = static_vector<value_type,elements>;
      using enum vector_tune_e;
      using enum vector_outcome_e;
      

      vector_type vec;
      vec.push_back(value_type{1});
      
      constexpr_test(size(vec) == 1);
      constexpr_test( vec[0u] == 1);
      
      value_type const v2{2};
      vec.push_back(v2);
      
      value_type v3{3};
      vec.push_back(std::move(v3));
      constexpr_test(size(vec) == 3);
      constexpr_test( vec[0u] == 1);
      constexpr_test( vec[1u] == 2);
      constexpr_test( vec[2u] == 3);
      
      push_back(vec,value_type{1});
      push_back(vec,v2);
      v3 = 3;
      push_back(vec,std::move(v3));
      constexpr_test(size(vec) == 6);
      return {};
      };
    result |= run_constexpr_test<traits_list>(fn_tmpl);
    result |= run_consteval_test<constexpr_traits_list>(fn_tmpl);
    };
//---------------------------------------------------------------------------------------------------------
  "test_static_vector_move"_test = [&result]
    {
    auto fn_tmpl = []<typename value_type>( value_type const * ) -> metatests::test_result
      {
      auto constexpr elements = 10;
      using vector_type = static_vector<value_type,elements>;
      vector_type vec;
      emplace_back(vec, 1);
      emplace_back(vec, 2);
      vec.emplace_back(3);
      auto res = emplace_back(vec, 4);
      test_result tr;
      tr = constexpr_test( (res == vector_outcome_e::no_error) )
        | constexpr_test(size(vec) == 4)
        | constexpr_test( std::distance(begin(vec), end(vec)) == size(vec) );

      std::array<value_type,4> tst{ 1,2,3,4};
      tr |= constexpr_test(equal(vec,tst));

      vector_type vec2{ std::move(vec) };
      tr |= constexpr_test(size(vec) == 0)
      | constexpr_test(size(vec2) == 4)
      | constexpr_test(equal(vec2,tst));

      return tr;
      };

    result |= run_constexpr_test<traits_list>(fn_tmpl);
    result |= run_consteval_test<constexpr_traits_list>(fn_tmpl);
    };
    
//---------------------------------------------------------------------------------------------------------
  "test_static_vector_copy"_test = [&result]
    {
    auto fn_tmpl = []<typename value_type>( value_type const * ) -> metatests::test_result
      {
      auto constexpr elements = 10;
      using vector_type = static_vector<value_type,elements>;
      vector_type vec;
      emplace_back(vec, 1);
      emplace_back(vec, 2);
      vec.emplace_back(3);
      auto res = emplace_back(vec, 4);

      test_result tr =
      constexpr_test( (res == vector_outcome_e::no_error) )
      | constexpr_test(size(vec) == 4)
      | constexpr_test( std::distance(begin(vec), end(vec)) == size(vec) );

      std::array<value_type,4> tst{1,2,3,4};
      tr |= constexpr_test(equal(vec,tst));

      vector_type vec2{ const_cast<vector_type const &>(vec) };
      tr |= constexpr_test(size(vec) == 4)
        | constexpr_test(size(vec2) == 4)
        | constexpr_test(equal(vec,tst))
        | constexpr_test(equal(vec2,tst));

      return tr;
      };

    result |= run_constexpr_test<traits_list>(fn_tmpl);
    result |= run_consteval_test<constexpr_traits_list>(fn_tmpl);
    };
//---------------------------------------------------------------------------------------------------------
  "test_static_vector_emplace"_test = [&result]
    {
    auto fn_tmpl = []<typename value_type>( value_type const * ) -> metatests::test_result
      {
      auto constexpr elements = 20;
      using vector_type = static_vector<value_type,elements>;
      using enum vector_outcome_e;
      using enum vector_tune_e;

      test_result tr;
      vector_type vec;
        {
        std::array<value_type,1> expected{1};
        value_type v = 1;
        auto res = emplace(vec, begin(vec), v );
        tr |= constexpr_test((std::same_as<vector_outcome_e,decltype(emplace(vec, begin(vec), v ))>))
          | constexpr_test( (res == no_error) )
          | constexpr_test( std::distance(begin(vec), end(vec)) == size(vec) )
          | constexpr_test(size(vec) == 1)
          | constexpr_test(equal(vec,expected));
        }
        {
        std::array<value_type,2> expected{2,1};
        value_type v = 2;
        emplace<unchecked>(vec, begin(vec), std::move(v) );
        tr |= constexpr_test((std::same_as<void,decltype(emplace<unchecked>(vec, begin(vec), v ))>));

        tr |= constexpr_test(size(vec) == 2)
          | constexpr_test(equal(vec,expected));
        }
        {
        std::array<value_type,3> expected{2,3,1};
        value_type v = 3;
        auto res = vec.emplace(begin(vec)+1, std::move(v) );
        tr |= constexpr_test( (res == no_error) )
          | constexpr_test(size(vec) == 3)
          | constexpr_test(equal(vec,expected));
        }
        {
        std::array<value_type,4> expected{2,4,3,1};
        value_type v = 4;
        auto res = emplace(vec, begin(vec)+1, std::move(v) );
        tr |= constexpr_test( (res == no_error) )
          | constexpr_test(size(vec) == 4)
          | constexpr_test(equal(vec,expected));
        }
        {
        std::array<value_type,5> expected{2,4,3,1,5};
        value_type v = 5;
        auto res = emplace(vec, end(vec), std::move(v) );
        tr |= constexpr_test( (res == no_error) )
          | constexpr_test(size(vec) == 5)
          | constexpr_test(equal(vec,expected));
        }
      return tr;
      };

    result |= run_constexpr_test<traits_list>(fn_tmpl);
    result |= run_consteval_test<constexpr_traits_list>(fn_tmpl);
    };
//---------------------------------------------------------------------------------------------------------
  "test_static_vector_insert"_test = [&result]
    {
    auto fn_tmpl = []<typename value_type>( value_type const * ) -> metatests::test_result
      {
      auto constexpr elements = 20;
      using vector_type = static_vector<value_type,elements>;

      test_result tr;

      vector_type vec;
        {
        std::array<value_type,5> tst{1,2,3,4,5};
        auto res = insert(vec, begin(vec), begin(tst), end(tst) );
        tr |= constexpr_test( (res == vector_outcome_e::no_error) )
          | constexpr_test(size(vec) == 5)
          | constexpr_test(equal(vec,tst));
        }
        {
        std::array<value_type,3> tst{11,12,13};
        std::array<value_type,8> exp{11,12,13,1,2,3,4,5};
        auto res = insert(vec, begin(vec), begin(tst), end(tst) );
        tr |= constexpr_test( (res == vector_outcome_e::no_error) )
          | constexpr_test(size(vec) == 8)
          | constexpr_test(equal(vec,exp));
        }
        {
        std::array<value_type,2> tst{21,22};
        std::array<value_type,10> exp{11,12,21,22,13,1,2,3,4,5};
        auto res = insert(vec, begin(vec)+2, begin(tst), end(tst) );
        tr |= constexpr_test( (res == vector_outcome_e::no_error) )
          | constexpr_test(size(vec) == 10)
          | constexpr_test(equal(vec,exp));
        }
        {
        std::array<value_type,4> tst{31,32,33,34};
        std::array<value_type,14> exp{11,12,21,22,13,1,31,32,33,34,2,3,4,5};
        auto res = vec.insert(begin(vec)+6, begin(tst), end(tst) );
        tr |= constexpr_test( (res == vector_outcome_e::no_error) )
          | constexpr_test(size(vec) == 14)
          | constexpr_test(equal(vec,exp));
        }
        {
        std::array<value_type,4> tst{41,42,43,44};
        std::array<value_type,18> exp{11,12,21,22,13,1,31,32,33,34,2,3,41,42,43,44,4,5};
        auto res = insert(vec, begin(vec)+12, begin(tst), end(tst) );
        tr |= constexpr_test( (res == vector_outcome_e::no_error) )
          | constexpr_test(size(vec) == 18)
          | constexpr_test(equal(vec,exp));
        }
        {
        std::array<value_type,2> tst{51,52};
        std::array<value_type,20> exp{11,12,21,22,13,1,31,32,33,34,2,3,41,42,43,44,4,5,51,52};
        auto res = insert(vec, end(vec), begin(tst), end(tst) );
    //     dump(vec);
        tr |= constexpr_test( (res == vector_outcome_e::no_error) )
          | constexpr_test(size(vec) == 20)
          | constexpr_test(equal(vec,exp));
        }
        {
        std::array<value_type,2> tst{51,52};
        std::array<value_type,20> exp{11,12,21,22,13,1,31,32,33,34,2,3,41,42,43,44,4,5,51,52};
        auto res = insert(vec, end(vec), begin(tst), end(tst) );
    //     dump(vec);
        tr |= constexpr_test( (res == vector_outcome_e::out_of_storage) )
          | constexpr_test(size(vec) == 20)
          | constexpr_test(equal(vec,exp));
        }
      return tr;
      };

    result |= run_constexpr_test<traits_list>(fn_tmpl);
    result |= run_consteval_test<constexpr_traits_list>(fn_tmpl);
    };
//---------------------------------------------------------------------------------------------------------
  "test_static_vector_insert_moving"_test = [&result]
    {
    auto fn_tmpl = []<typename value_type>( value_type const * ) -> metatests::test_result
      {
      using std::make_move_iterator;

      auto constexpr elements = 20;
      using vector_type = static_vector<value_type,elements>;
      test_result tr;
      vector_type vec;
        {
        std::array<value_type,5> tst{1,2,3,4,5};
        std::array<value_type,5> exp{1,2,3,4,5};
        std::array<value_type,5> exp2{};
        auto res = insert(vec, begin(vec), make_move_iterator(begin(tst)), make_move_iterator(end(tst)) );
        tr |= constexpr_test(res == vector_outcome_e::no_error)
          | constexpr_test(size(vec) == 5)
          | constexpr_test(equal(vec,exp));
        //trivial dont clean up after themselfs
        if constexpr(not std::is_trivially_destructible_v<value_type>)
          tr |= constexpr_test(equal(tst,exp2));
        }
        {
        std::array<value_type,3> tst{11,12,13};
        std::array<value_type,8> exp{11,12,13,1,2,3,4,5};
        auto res = vec.insert(begin(vec), make_move_iterator(begin(tst)), make_move_iterator(end(tst)) );
        tr |= constexpr_test(res == vector_outcome_e::no_error)
          | constexpr_test(size(vec) == 8)
          | constexpr_test(equal(vec,exp));
        }

      return tr;
      };

    result |= run_constexpr_test<traits_list_move>(fn_tmpl);
    //gcc libstdc++ bug missing constexpr
    //https://gcc.gnu.org/bugzilla/show_bug.cgi?id=102358
  //   result |= run_consteval_test<constexpr_traits_list>(constexpr_static_vector_test_insert_moving);
    result |= run_constexpr_test<constexpr_traits_list>(fn_tmpl);
    };

//---------------------------------------------------------------------------------------------------------
  "test_static_vector_erase_at_end"_test = [&result]
    {
    auto fn_tmpl = []<typename value_type>( value_type const * ) -> metatests::test_result
      {
      auto constexpr elements = 20;
      using vector_type = static_vector<value_type,elements>;
      test_result tr;
      vector_type vec;
      std::array<value_type,20> tst{11,12,21,22,13,1,31,32,33,34,2,3,41,42,43,44,4,5,51,52};
      insert(vec, end(vec), begin(tst), end(tst));
      auto res = erase_at_end(vec, begin(vec));
      tr |= constexpr_test(size(vec) == 0)
        | constexpr_test(end(vec) == res);

      insert(vec, end(vec), begin(tst), end(tst));
      res = vec.erase_at_end(begin(vec)+5);
      tr |= constexpr_test(size(vec) == 5)
        | constexpr_test(begin(vec)+5 == res)
        | constexpr_test(equal(vec, std::span(begin(tst), 5)));

      return tr;
      };

    result |= run_constexpr_test<traits_list>(fn_tmpl);
    result |= run_consteval_test<constexpr_traits_list>(fn_tmpl);
    };
//---------------------------------------------------------------------------------------------------------
  "test_static_vector_erase_range"_test = [&result]
    {
    auto fn_tmpl = []<typename value_type>( value_type const * ) -> metatests::test_result
      {
      auto constexpr elements = 20;
      using vector_type = static_vector<value_type,elements>;
      test_result tr;
      vector_type vec;
      std::array<value_type,20> tst{11,12,21,22,13,1,31,32,33,34,2,3,41,42,43,44,4,5,51,52};
      insert(vec, end(vec), begin(tst), end(tst));

      auto res = erase(vec, begin(vec)+2, begin(vec)+4 );
      tr |= constexpr_test(size(vec) == 18)
        | constexpr_test( res == begin(vec)+2 )
        | constexpr_test( at(vec, 2u) == 13 )
        | constexpr_test( at(vec, 3u) == 1 )
        | constexpr_test( at(vec, 4u) == 31 )
        | constexpr_test( at(vec, 17u) == 52 );

      res = vec.erase(begin(vec)+3, begin(vec)+3 );
      tr |= constexpr_test(size(vec) == 18)
        | constexpr_test( res == begin(vec)+3 )
        | constexpr_test( at(vec, 2u) == 13 )
        | constexpr_test( at(vec, 3u) == 1 )
        | constexpr_test( at(vec, 4u) == 31 )
        | constexpr_test( at(vec, 17u) == 52 );

      return tr;
      };

    result |= run_constexpr_test<traits_list>(fn_tmpl);
    result |= run_consteval_test<constexpr_traits_list>(fn_tmpl);
    };
//---------------------------------------------------------------------------------------------------------
  "test_static_vector_erase"_test = [&result]
    {
    auto fn_tmpl = []<typename value_type>( value_type const * ) -> metatests::test_result
      {
      auto constexpr elements = 20;
      using vector_type = static_vector<value_type,elements>;
      test_result tr;
      vector_type vec;
      std::array<value_type,20> tst{11,12,21,22,13,1,31,32,33,34,2,3,41,42,43,44,4,5,51,52};
      insert(vec, end(vec), begin(tst), end(tst));

      auto res = erase(vec, begin(vec)+2 );
      tr |= constexpr_test(size(vec) == 19)
        | constexpr_test( res == begin(vec)+2 )
        | constexpr_test( at(vec, 2u) == 22 )
        | constexpr_test( at(vec, 3u) == 13 )
        | constexpr_test( at(vec, 4u) == 1 )
        | constexpr_test( at(vec, 18u) == 52 );

      res = erase(vec, end(vec)-1 );
      tr |= constexpr_test( res == end(vec) )
        | constexpr_test(size(vec) == 18)
        | constexpr_test( at(vec, 17u) == 51 );

      res = vec.erase(begin(vec) );
      tr |= constexpr_test( res == begin(vec) )
        | constexpr_test(size(vec) == 17)
        | constexpr_test( at(vec, 0u) == 12 )
        | constexpr_test( at(vec, 16u) == 51 );

      return tr;
      };

    result |= run_constexpr_test<traits_list>(fn_tmpl);
    result |= run_consteval_test<constexpr_traits_list>(fn_tmpl);
    };
//---------------------------------------------------------------------------------------------------------
  "test_static_vector_split_by_half"_test = [&result]
    {
    auto fn_tmpl = []<typename value_type>( value_type const * ) -> metatests::test_result
      {
      test_result tr;
        {
        auto constexpr elements = 20;
        using vector_type = static_vector<value_type,elements>;
        vector_type vec;
        std::array<value_type,20> tst{11,12,21,22,13,1,31,32,33,34,2,3,41,42,43,44,4,5,51,52};
        insert(vec, end(vec), begin(tst), end(tst));
        vector_type vec2;
        auto res = split_by_half(vec,vec2);
        tr |= constexpr_test( (res == vector_outcome_e::no_error) )
          | constexpr_test(size(vec) == 10)
          | constexpr_test(size(vec2) == 10)
          | constexpr_test(equal(vec, std::span(begin(tst), 10)))
          | constexpr_test(equal(vec2, std::span(begin(tst)+10, 10)));
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
        tr |= constexpr_test( (res == vector_outcome_e::no_error) )
          | constexpr_test(size(vec) == 10)
          | constexpr_test(size(vec2) == 20)
          | constexpr_test(equal(vec, std::span(begin(tst), 10)))
          | constexpr_test(equal(vec2, std::span(begin(tst), 20)));
        }
      return tr;
      };

    result |= run_constexpr_test<traits_list>(fn_tmpl);
    result |= run_consteval_test<constexpr_traits_list>(fn_tmpl);
    };
//---------------------------------------------------------------------------------------------------------
  "test_static_vector_resize"_test = [&result]
    {
    auto constexpr_static_vector_resize = []<typename value_type>( value_type const * ) -> metatests::test_result
      {
      auto constexpr elements = 20;
      using vector_type = static_vector<value_type,elements>;
      test_result tr;
      vector_type vec;
        {
        std::array<value_type,10> expected{};
        resize(vec,10);
        tr |= constexpr_test(size(vec) == 10)
          | constexpr_test(equal(vec,expected));
        }
        {
        resize(vec,20);
        std::array<value_type,20> expected{};
        tr |= constexpr_test(size(vec) == 20)
          | constexpr_test(equal(vec,expected));
        }
        {
        resize(vec,15);
        std::array<value_type,15> expected{};
        tr |= constexpr_test(size(vec) == 15)
          | constexpr_test(equal(vec,expected));
        }
      clear(vec);
        {
        std::array<value_type,10> expected;
        std::iota( begin(expected), end(expected), value_type(1) );
        insert(vec, begin(vec), begin(expected), end(expected) );
        tr |= constexpr_test(size(vec) == 10)
          | constexpr_test(equal(vec,expected));
        }
        {
        resize(vec,15);
        std::array<value_type,15> expected{};
        std::iota( begin(expected), begin(expected)+10, value_type(1) );
        tr |= constexpr_test(size(vec) == 15)
          | constexpr_test(equal(vec,expected));
        }
        {
        vec.resize(5);
        std::array<value_type,5> expected{};
        std::iota( begin(expected), end(expected), value_type(1) );
        tr |= constexpr_test(size(vec) == 5)
          | constexpr_test(equal(vec,expected));
        }

      return tr;
      };

    result |= run_constexpr_test<traits_list>(constexpr_static_vector_resize);
    result |= run_consteval_test<constexpr_traits_list>(constexpr_static_vector_resize);
    };
  return result ? EXIT_SUCCESS : EXIT_FAILURE;
  }

