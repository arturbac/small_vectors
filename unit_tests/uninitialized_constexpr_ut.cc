#include <small_vectors/detail/uninitialized_constexpr.h>
#include <unit_test_core.h>

#include <atomic>
#include <iostream>
#include <algorithm>
#include <functional>

using traits_list_move
  = metatests::type_list<uint16_t, int32_t, int64_t, double, non_trivial, non_trivial_ptr, non_trivial_ptr_except>;

namespace small_vectors::detail
  {
template<typename value_type, unsigned size>
constexpr auto construct_vec()
  {
  std::array<value_type *, size> sz;
  std::for_each(begin(sz), end(sz), [](auto & el) { el = new value_type(); });
  return sz;
  }

template<typename value_type, auto size>
constexpr auto destroy_vec(std::array<value_type *, size> & sz)
  {
  std::for_each(
    begin(sz),
    end(sz),
    [](auto & el)
    {
      if(el)
        delete el;
    }
  );
  }

auto constexpr_uninitialized_move_n = []<typename value_type>(value_type const *) -> metatests::test_result
{
  {
  auto arr1{construct_vec<value_type, 10>()};
  std::array<value_type *, 10> out;
  uninitialized_move_n(begin(arr1), 10, begin(out));
  destroy_vec(out);
  }
return {};
};

  }  // namespace small_vectors::detail

int main()
  {
  using namespace metatests;

  test_result res = run_constexpr_test<traits_list_move>(small_vectors::detail::constexpr_uninitialized_move_n);
  res |= run_consteval_test<traits_list_move>(small_vectors::detail::constexpr_uninitialized_move_n);

  return res ? EXIT_SUCCESS : EXIT_FAILURE;
  }

