#include <coll/detail/uninitialized_constexpr.h>
#include <unit_test_core.h>

#include <atomic>
#include <iostream>

using traits_list_move = boost::mpl::list<uint16_t, int32_t, int64_t, double,
                                     non_trivial, non_trivial_ptr, non_trivial_ptr_except>;

namespace coll::detail
{
template<typename value_type,unsigned size>
constexpr auto construct_vec()
  {
  std::array<value_type *,size> sz;
  std::for_each(begin(sz), end(sz), [](auto & el){ el= new value_type(); });
  return sz;
  }

template<typename value_type,auto size>
constexpr auto destroy_vec(std::array<value_type *,size> & sz)
  {
  std::for_each(begin(sz), end(sz), [](auto & el){ if(el) delete el; });
  }

template<typename value_type>
bool constexpr constexpr_uninitialized_move_n()
  {
    {
    auto arr1{ construct_vec<value_type,10>()};
    std::array<value_type *,10> out;
    uninitialized_move_n(begin(arr1),10,begin(out));
    destroy_vec(out);
    }
  return true;
  }

template<typename value_type>
bool consteval consteval_uninitialized_move_n()
  {
  return constexpr_uninitialized_move_n<value_type>();
  }
BOOST_AUTO_TEST_CASE_TEMPLATE( static_vector_copy, value_type, traits_list_move )
  {
  BOOST_TEST((consteval_uninitialized_move_n<value_type>()));
  BOOST_TEST((constexpr_uninitialized_move_n<value_type>()));
  }
}
