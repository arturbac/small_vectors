#include <coll/small_vector.h>
#include <unit_test_core.h>

#include <iostream>

namespace coll
  {
using detail::small_vector_storage_type;
using enum detail::small_vector_storage_type;
using enum detail::vector_outcome_e;

using constexpr_size_type_traits_list = metatests::type_list<uint8_t, uint16_t, uint32_t, uint64_t>;
using metatests::constexpr_test;
using metatests::run_consteval_test;
using metatests::run_constexpr_test;

//---------------------------------------------------------------------------------------------------------------------
template<typename size_type, typename value_type>
inline void dump_storage_info()
  {
  using st = detail::small_vector_storage<value_type, size_type>;
  std::cout << "size_type " << sizeof(size_type) << " value_type " << sizeof(value_type) << " "
            << "buffered_capacity " << unsigned(st::buffered_capacity) << "\n"
            << "struct size " << sizeof(st) << " storage size " << sizeof(typename st::storage_type) << "\n\n";
  }

template<typename size_type, typename value_type>
inline void dump_estorage_info()
  {
  using st = detail::small_vector_storage<value_type, size_type, 0u>;
  std::cout << "size_type " << sizeof(size_type) << " value_type " << sizeof(value_type) << " "
            << "buffered_capacity " << unsigned(st::buffered_capacity) << "\n"
            << "struct size " << sizeof(st) << " storage size " << sizeof(typename st::storage_type) << "\n\n";
  }

//---------------------------------------------------------------------------------------------------------------------
constexpr bool equal(auto const & result, auto const & expected) noexcept
  {
  return std::equal(begin(result), end(result), begin(expected), end(expected));
  }

//---------------------------------------------------------------------------------------------------------------------
constexpr small_vector_storage_type expected_istorage(auto capacity_req) noexcept
  {
  return capacity_req == 0u ? dynamic : buffered;
  }

//---------------------------------------------------------------------------------------------------------------------
template<typename size_type, typename value_type>
consteval bool test_storage_info(std::size_t e_struct_size, std::size_t e_buff_capacity)
  {
  using st = detail::small_vector_storage<value_type, size_type>;
  constexpr_test(sizeof(st) == e_struct_size);
  constexpr_test(st::buffered_capacity == e_buff_capacity);
  return true;
  }

static_assert(sizeof(void *) == 8, "thise tests are written for 64bit platform");

consteval bool consteval_test_small_vector_storage_size()
  {
  test_storage_info<uint8_t, uint8_t>(16, 14);
  test_storage_info<uint16_t, uint8_t>(16, 13);
  test_storage_info<uint32_t, uint8_t>(24, 19);
  test_storage_info<uint64_t, uint8_t>(32, 23);

  test_storage_info<uint8_t, uint16_t>(16, 7);
  test_storage_info<uint16_t, uint16_t>(16, 6);
  test_storage_info<uint32_t, uint16_t>(24, 9);
  test_storage_info<uint64_t, uint16_t>(32, 11);

  test_storage_info<uint8_t, uint32_t>(16, 3);
  test_storage_info<uint16_t, uint32_t>(16, 3);
  test_storage_info<uint32_t, uint32_t>(24, 4);
  test_storage_info<uint64_t, uint32_t>(32, 5);

  test_storage_info<uint8_t, uint64_t>(16, 1);
  test_storage_info<uint16_t, uint64_t>(16, 1);
  test_storage_info<uint32_t, uint64_t>(24, 2);
  test_storage_info<uint64_t, uint64_t>(32, 2);

  static_assert(sizeof(non_trivial) == 4);
  test_storage_info<uint8_t, non_trivial>(16, 3);
  test_storage_info<uint16_t, non_trivial>(16, 3);
  test_storage_info<uint32_t, non_trivial>(24, 4);
  test_storage_info<uint64_t, non_trivial>(32, 5);
  return true;
  }

struct struct_3_byte
  {
  uint8_t tst[3];
  };

struct struct_5_byte
  {
  uint8_t tst[5];
  };

using consteval_test_type_list
  = metatests::type_list<uint8_t, uint16_t, uint32_t, uint64_t, struct_3_byte, struct_5_byte>;
using constexpr_test_type_list = metatests::type_list<
  uint8_t,
  uint16_t,
  uint32_t,
  uint64_t,
  struct_3_byte,
  struct_5_byte,
  non_trivial,
  non_trivial_ptr,
  non_trivial_ptr_except_copy>;
  }  // namespace coll

using namespace coll;
using namespace metatests;

static bool volatile vfalse{};

static bool test_failure() { return vfalse; }

int main()
  {
  test_result result;
  using boost::ut::operator""_test;
  using boost::ut::eq;
  using boost::ut::neq;

  result = constexpr_test(consteval_test_small_vector_storage_size());

  //---------------------------------------------------------------------------------------------------------------------
  "test_small_vector_constr"_test = [&result]
  {
    auto fn_tmpl
      = []<typename value_type, typename size_type>(value_type const *, size_type const *) -> metatests::test_result
    {
      test_result tr;
        {
        constexpr auto capacity_req = at_least<value_type>(size_type(8));
        using st = small_vector<value_type, size_type, capacity_req>;
        st v;
        tr |= constexpr_test(size(v) == 0u) | constexpr_test(empty(v)) | constexpr_test(begin(v) == end(v))
              | constexpr_test(capacity(v) >= 8u) | constexpr_test(data(v) != nullptr);
        }
        {
        using st = small_vector<value_type, size_type, 0>;
        st v;
        tr |= constexpr_test(size(v) == 0u) | constexpr_test(empty(v)) | constexpr_test(begin(v) == end(v))
              | constexpr_test(capacity(v) == 0u) | constexpr_test(data(v) == nullptr);
        }
      return tr;
    };

    result |= run_consteval_test_dual<consteval_test_type_list, constexpr_size_type_traits_list>(fn_tmpl);
    result |= run_constexpr_test_dual<constexpr_test_type_list, constexpr_size_type_traits_list>(fn_tmpl);
  };
  result |= constexpr_test(!test_failure());
  //---------------------------------------------------------------------------------------------------------------------
  "test_small_vector_emplace_back"_test = [&result]
  {
    auto constexpr_test_small_vector_emplace_back = []() -> metatests::test_result
    {
      using value_type = uint32_t;
      using st = small_vector<value_type, uint16_t, 3>;
      st v;
      test_result tr = constexpr_test(coll::detail::growth(uint16_t(3), uint16_t(1)) == 7);

      auto res = emplace_back(v, 0xf5u);
      tr |= constexpr_test(res == 0xf5u) | constexpr_test(size(v) == 1u) | constexpr_test(!empty(v))
            | constexpr_test(begin(v) != end(v)) | constexpr_test(*begin(v) == 0xf5u) | constexpr_test(back(v) == 0xf5u)
            | constexpr_test(v.active_storage() == buffered);

      res = emplace_back(v, 0x15u);
        {
        std::array<value_type, 2> expected{0xf5u, 0x15u};
        tr |= constexpr_test(res == 0x15u) | constexpr_test(size(v) == 2u) | constexpr_test(!empty(v))
              | constexpr_test(begin(v) != end(v)) | constexpr_test(v.active_storage() == buffered)
              | constexpr_test(equal(v, expected));
        }

      res = emplace_back(v, 0x25u);
        {
        std::array<value_type, 3> expected{0xf5u, 0x15u, 0x25u};
        tr |= constexpr_test(res == 0x25u) | constexpr_test(size(v) == 3u) | constexpr_test(!empty(v))
              | constexpr_test(begin(v) != end(v)) | constexpr_test(v.active_storage() == buffered)
              | constexpr_test(equal(v, expected));
        }

        {
        res = emplace_back(v, 0x35u);
        std::array<value_type, 4> expected{0xf5u, 0x15u, 0x25u, 0x35u};
        tr |= constexpr_test(res == 0x35u) | constexpr_test(v.active_storage() == dynamic)
              | constexpr_test(size(v) == 4u) | constexpr_test(!empty(v)) | constexpr_test(begin(v) != end(v))
              | constexpr_test(equal(v, expected));
        }
        {
        res = emplace_back(v, 0x45u);
        std::array<value_type, 5> expected{0xf5u, 0x15u, 0x25u, 0x35u, 0x45u};
        tr |= constexpr_test(res == 0x45u) | constexpr_test(v.active_storage() == dynamic)
              | constexpr_test(size(v) == 5u) | constexpr_test(!empty(v)) | constexpr_test(begin(v) != end(v))
              | constexpr_test(equal(v, expected));
        }
        {
        res = emplace_back(v, 0x55u);
        std::array<value_type, 6> expected{0xf5u, 0x15u, 0x25u, 0x35u, 0x45u, 0x55u};
        tr |= constexpr_test(res == 0x55u) | constexpr_test(v.active_storage() == dynamic)
              | constexpr_test(size(v) == 6u) | constexpr_test(!empty(v)) | constexpr_test(begin(v) != end(v))
              | constexpr_test(equal(v, expected));
        }
        {
        res = emplace_back(v, 1u);
        std::array<value_type, 7> expected{0xf5u, 0x15u, 0x25u, 0x35u, 0x45u, 0x55u, 1u};
        tr |= constexpr_test(res == 1u) | constexpr_test(v.active_storage() == dynamic)
              | constexpr_test(size(v) == expected.size()) | constexpr_test(!empty(v))
              | constexpr_test(begin(v) != end(v)) | constexpr_test(equal(v, expected));
        }
        {
        res = emplace_back(v, 2u);
        std::array<value_type, 8> expected{0xf5u, 0x15u, 0x25u, 0x35u, 0x45u, 0x55u, 1u, 2u};
        tr |= constexpr_test(res == 2u) | constexpr_test(v.active_storage() == dynamic)
              | constexpr_test(size(v) == expected.size()) | constexpr_test(!empty(v))
              | constexpr_test(begin(v) != end(v)) | constexpr_test(equal(v, expected));
        }
        {
        res = emplace_back(v, 3u);
        std::array<value_type, 9> expected{0xf5u, 0x15u, 0x25u, 0x35u, 0x45u, 0x55u, 1u, 2u, 3u};
        tr |= constexpr_test(res == 3u) | constexpr_test(v.active_storage() == dynamic)
              | constexpr_test(size(v) == expected.size()) | constexpr_test(!empty(v))
              | constexpr_test(begin(v) != end(v)) | constexpr_test(equal(v, expected));
        }
      return tr;
    };

    result |= constexpr_test(constexpr_test_small_vector_emplace_back());
  };

  "test_small_vector_push_back"_test = [&result]
  {
    auto fn = []() -> metatests::test_result
    {
      using value_type = uint32_t;
      using st = small_vector<value_type, uint16_t, 3>;
      st v;

      v.push_back(value_type{1});

      constexpr_test(size(v) == 1);
      constexpr_test(v[0u] == 1);

      value_type const v2{2};
      v.push_back(v2);

      value_type v3{3};
      v.push_back(std::move(v3));
      constexpr_test(size(v) == 3);
      constexpr_test(v[0u] == 1);
      constexpr_test(v[1u] == 2);
      constexpr_test(v[2u] == 3);

      push_back(v, value_type{1});
      push_back(v, v2);
      v3 = 3;
      push_back(v, std::move(v3));
      constexpr_test(size(v) == 6);

      return {};
    };

    result |= constexpr_test(fn());
  };
  //---------------------------------------------------------------------------------------------------------------------
  "test_small_vector_emplace_back_0"_test = [&result]
  {
    auto fn_tmpl = []() -> metatests::test_result
    {
      using value_type = uint32_t;
      using st = small_vector<value_type, uint16_t, 0u>;
      st v;
      test_result tr;
      auto res = emplace_back(v, 0xf5u);
      tr |= constexpr_test(res == 0xf5u) | constexpr_test(size(v) == 1u) | constexpr_test(!empty(v))
            | constexpr_test(begin(v) != end(v)) | constexpr_test(*begin(v) == 0xf5u) | constexpr_test(back(v) == 0xf5u)
            | constexpr_test(v.active_storage() == dynamic);

      res = emplace_back(v, 0x15u);
        {
        std::array<value_type, 2> expected{0xf5u, 0x15u};
        tr |= constexpr_test(res == 0x15u) | constexpr_test(size(v) == 2u) | constexpr_test(!empty(v))
              | constexpr_test(begin(v) != end(v)) | constexpr_test(v.active_storage() == dynamic)
              | constexpr_test(equal(v, expected));
        }

      res = emplace_back(v, 0x25u);
        {
        std::array<value_type, 3> expected{0xf5u, 0x15u, 0x25u};
        tr |= constexpr_test(res == 0x25u) | constexpr_test(size(v) == 3u) | constexpr_test(!empty(v))
              | constexpr_test(begin(v) != end(v)) | constexpr_test(v.active_storage() == dynamic)
              | constexpr_test(equal(v, expected));
        }
      return tr;
    };

    result |= constexpr_test(fn_tmpl());
  };

  "test_small_vector_push_back_0"_test = [&result]
  {
    auto fn = []() -> metatests::test_result
    {
      using value_type = uint32_t;
      using st = small_vector<value_type, uint16_t, 0u>;
      st v;

      v.push_back(value_type{1});

      constexpr_test(size(v) == 1);
      constexpr_test(v[0u] == 1);

      value_type const v2{2};
      v.push_back(v2);

      value_type v3{3};
      v.push_back(std::move(v3));
      constexpr_test(size(v) == 3);
      constexpr_test(v[0u] == 1);
      constexpr_test(v[1u] == 2);
      constexpr_test(v[2u] == 3);

      push_back(v, value_type{1});
      push_back(v, v2);
      v3 = 3;
      push_back(v, std::move(v3));
      constexpr_test(size(v) == 6);

      return {};
    };

    result |= constexpr_test(fn());
  };
  //---------------------------------------------------------------------------------------------------------------------
  "test_small_vector_emplace_back_tmpl"_test = [&result]
  {
    auto fn_tmpl = []<typename value_type, typename size_type, typename szreq_ic>(
                     value_type const *, size_type const *, szreq_ic
                   ) -> metatests::test_result
    {
      constexpr size_type capacity_req = szreq_ic::value;
      std::array<value_type, 127> test_values{};
      std::iota(begin(test_values), end(test_values), value_type(1));
      test_result tr;
      //   constexpr size_type capacity_req =at_least<value_type>(size_type(4));
      using st = small_vector<value_type, size_type, capacity_req>;
      st v;
      constexpr size_type first_growth = coll::detail::growth(capacity_req, size_type(1));
      constexpr size_type second_growth = coll::detail::growth(first_growth, size_type(1));
      static_assert(second_growth + 1u < test_values.size());
      for(size_type s{}; s != capacity_req; ++s)
        {
        auto res = emplace_back(v, test_values[s]);
        tr |= constexpr_test(res == test_values[s])
              | constexpr_test(v.active_storage() == expected_istorage(capacity_req)) | constexpr_test(size(v) == s + 1)
              | constexpr_test(!empty(v));
        std::span expected{test_values.data(), s + 1u};
        tr |= constexpr_test(equal(v, expected)) | constexpr_test(back(v) == test_values[s]);
        }
      for(size_type s{capacity_req}; s != first_growth; ++s)
        {
        auto res = emplace_back(v, test_values[s]);
        tr |= constexpr_test(res == test_values[s]) | constexpr_test(v.active_storage() == dynamic)
              | constexpr_test(size(v) == s + 1) | constexpr_test(!empty(v));
        std::span expected{test_values.data(), s + 1u};
        tr |= constexpr_test(equal(v, expected)) | constexpr_test(back(v) == test_values[s]);
        }
      //   dump(v);
      for(size_type s{first_growth}; s != (second_growth + 1u); ++s)
        {
        auto res = emplace_back(v, test_values[s]);
        tr |= constexpr_test(res == test_values[s]) | constexpr_test(v.active_storage() == dynamic)
              | constexpr_test(size(v) == s + 1) | constexpr_test(!empty(v));
        std::span expected{test_values.data(), s + 1u};
        //     dump(expected);
        //     dump(v);
        tr |= constexpr_test(equal(v, expected)) | constexpr_test(back(v) == test_values[s]);
        }
      return tr;
    };

    auto fn_size = [fn_tmpl]<typename value_type, typename size_type>(
                     value_type const * t, size_type const * u
                   ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, coll::at_least<value_type>(size_type(4))>;
      return fn_tmpl(t, u, size_req{});
    };
    // size_type capacity_req = coll::at_least<value_type>(size_type(4))
    auto fn_0 = [fn_tmpl]<typename value_type, typename size_type>(
                  value_type const * t, size_type const * u
                ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, 0u>;
      return fn_tmpl(t, u, size_req{});
    };

    using consteval_test_type_list_1 = metatests::type_list<uint8_t, uint16_t, uint32_t, uint64_t>;
    using consteval_test_type_list_2 = metatests::type_list<
      uint8_t,
      uint16_t,
      uint32_t,
      uint64_t,
      non_trivial_ptr,
      non_trivial_ptr_except,
      non_trivial_ptr_except_copy>;

    result |= run_consteval_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_size);
    result |= run_consteval_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_0);
    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_size);
    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_0);
  };
  //---------------------------------------------------------------------------------------------------------------------
  // test for use with runtime asan to check proper unwinding and memory reclamation
  "small_vector_emplace_back_except"_test = [&result]
  {
    auto fn_tmpl = []<typename value_type, typename size_type, typename szreq_ic>(
                     value_type const *, size_type const *, szreq_ic
                   ) -> metatests::test_result
    {
      constexpr size_type capacity_req = szreq_ic::value;

      std::array<value_type, 127> test_values{};
      std::iota(begin(test_values), end(test_values), value_type(1));
      test_result tr;
      using st = small_vector<value_type, size_type, capacity_req>;
        {
        st v;
        reserve(v, 4u);
        insert(v, begin(v), begin(test_values), std::next(begin(test_values), int(capacity(v))));
        tr |= require_throw<std::runtime_error>([&v]() { emplace_back(v, value_type(128)); });  //,

        value_type const cobj(128);
        tr |= require_throw<std::runtime_error>([&v, &cobj]() { emplace_back(v, cobj); });
        }
        {
        st v;
        reserve(v, 4u);
        insert(v, begin(v), begin(test_values), std::next(begin(test_values), int(capacity(v))));
        back(v).set_value(128);
        tr |= require_throw<std::runtime_error>([&v]() { emplace_back(v, value_type(1)); });
        back(v).set_value(128);
        value_type const cobj(1);
        tr |= require_throw<std::runtime_error>([&v, &cobj]() { emplace_back(v, cobj); });
        }
        {
        st v;
        reserve(v, 40u);
        insert(v, begin(v), begin(test_values), std::next(begin(test_values), int(capacity(v))));
        tr |= require_throw<std::runtime_error>([&v]() { emplace_back(v, value_type(128)); });

        value_type const cobj(128);
        tr |= require_throw<std::runtime_error>([&v, &cobj]() { emplace_back(v, cobj); });
        }
        {
        st v;
        reserve(v, 40u);
        insert(v, begin(v), begin(test_values), std::next(begin(test_values), int(capacity(v))));
        back(v).set_value(128);
        tr |= require_throw<std::runtime_error>([&v]() { emplace_back(v, value_type(1)); });
        back(v).set_value(128);
        value_type const cobj(1);
        tr |= require_throw<std::runtime_error>([&v, &cobj]() { emplace_back(v, cobj); });
        }
      return tr;
    };
    auto fn_size = [fn_tmpl]<typename value_type, typename size_type>(
                     value_type const * t, size_type const * u
                   ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, coll::at_least<value_type>(size_type(4))>;
      return fn_tmpl(t, u, size_req{});
    };
    // size_type capacity_req = coll::at_least<value_type>(size_type(4))
    auto fn_0 = [fn_tmpl]<typename value_type, typename size_type>(
                  value_type const * t, size_type const * u
                ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, 0u>;
      return fn_tmpl(t, u, size_req{});
    };

    using consteval_test_type_list_1 = metatests::type_list<non_trivial_throwing<128>, non_trivial_throwing_copy<128>>;
    result |= run_constexpr_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_size);
    result |= run_constexpr_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_0);
  };
  //---------------------------------------------------------------------------------------------------------------------
  "test_small_vector_erase_at_end"_test = [&result]
  {
    auto fn_tmpl = []<typename value_type, typename size_type, typename szreq_ic>(
                     value_type const *, size_type const *, szreq_ic
                   ) -> metatests::test_result
    {
      constexpr size_type capacity_req = szreq_ic::value;

      std::array<value_type, 127> test_values;
      std::iota(begin(test_values), end(test_values), value_type(1));
      test_result tr;
      using st = small_vector<value_type, size_type, capacity_req>;
      st v;

      constexpr size_type second_growth
        = coll::detail::growth(coll::detail::growth(capacity_req, size_type(1)), size_type(1));
      static_assert(second_growth + 1u < test_values.size());
      for(size_type s{}; s != second_growth; ++s)
        emplace_back(v, test_values[s]);
        {
        tr |= constexpr_test(v.active_storage() == dynamic) | constexpr_test(size(v) == second_growth)
              | constexpr_test(!empty(v));
        std::span expected{test_values.data(), second_growth};
        tr |= constexpr_test(equal(v, expected));
        }
      for(size_type s{second_growth}; s > 1u; s -= 2)
        {
        auto it = erase_at_end(v, std::prev(end(v), 2));
        tr |= constexpr_test(it == end(v)) | constexpr_test(size(v) == s - 2);
        std::span expected{test_values.data(), s - 2u};
        tr |= constexpr_test(equal(v, expected));
        if(s > 2)
          tr |= constexpr_test(back(v) == test_values[s - 3u]);
        }
      return tr;
    };

    auto fn_size = [fn_tmpl]<typename value_type, typename size_type>(
                     value_type const * t, size_type const * u
                   ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, coll::at_least<value_type>(size_type(4))>;
      return fn_tmpl(t, u, size_req{});
    };

    auto fn_0 = [fn_tmpl]<typename value_type, typename size_type>(
                  value_type const * t, size_type const * u
                ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, 0u>;
      return fn_tmpl(t, u, size_req{});
    };

    using consteval_test_type_list_1 = metatests::type_list<uint8_t, uint16_t, uint32_t, uint64_t>;
    using consteval_test_type_list_2 = metatests::type_list<
      uint8_t,
      uint16_t,
      uint32_t,
      uint64_t,
      non_trivial_ptr,
      non_trivial_ptr_except,
      non_trivial_ptr_except_copy>;

    result |= run_consteval_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_size);
    result |= run_consteval_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_0);

    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_size);
    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_0);
  };

  //---------------------------------------------------------------------------------------------------------------------
  "test_small_vector_pop_back"_test = [&result]
  {
    auto test_small_vector_pop_back_tmpl = []<typename value_type, typename size_type, typename szreq_ic>(
                                             value_type const *, size_type const *, szreq_ic
                                           ) -> metatests::test_result
    {
      constexpr size_type capacity_req = szreq_ic::value;
      std::array<value_type, 127> test_values;
      std::iota(begin(test_values), end(test_values), value_type(1));
      test_result tr;
      using st = small_vector<value_type, size_type, capacity_req>;
      st v;

      constexpr size_type second_growth
        = coll::detail::growth(coll::detail::growth(capacity_req, size_type(1)), size_type(1));
      static_assert(second_growth + 1u < test_values.size());
      for(size_type s{}; s != second_growth; ++s)
        emplace_back(v, test_values[s]);
        {
        tr |= constexpr_test(v.active_storage() == dynamic) | constexpr_test(size(v) == second_growth)
              | constexpr_test(!empty(v));
        std::span expected{test_values.data(), second_growth};
        tr |= constexpr_test(equal(v, expected));
        }
      size_type curr_capacity{capacity(v)};
      for(size_type s{second_growth}; s != 0; --s)
        {
        pop_back(v);
        tr |= constexpr_test(size(v) == s - 1);
        std::span expected{test_values.data(), s - 1u};
        tr |= constexpr_test(equal(v, expected));
        tr |= constexpr_test(capacity(v) == curr_capacity);
        if(s > 1)
          tr |= constexpr_test(back(v) == test_values[s - 2u]);
        }
      return tr;
    };

    auto test_small_vector_pop_back_sz = [test_small_vector_pop_back_tmpl]<typename value_type, typename size_type>(
                                           value_type const * t, size_type const * u
                                         ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, coll::at_least<value_type>(size_type(4))>;
      return test_small_vector_pop_back_tmpl(t, u, size_req{});
    };
    // size_type capacity_req = coll::at_least<value_type>(size_type(4))
    auto test_small_vector_pop_back_0 = [test_small_vector_pop_back_tmpl]<typename value_type, typename size_type>(
                                          value_type const * t, size_type const * u
                                        ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, 0u>;
      return test_small_vector_pop_back_tmpl(t, u, size_req{});
    };

    using consteval_test_type_list_1 = metatests::type_list<uint8_t, uint16_t, uint32_t, uint64_t>;
    using consteval_test_type_list_2 = metatests::type_list<
      uint8_t,
      uint16_t,
      uint32_t,
      uint64_t,
      non_trivial_ptr,
      non_trivial_ptr_except,
      non_trivial_ptr_except_copy>;

    result |= run_consteval_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(
      test_small_vector_pop_back_sz
    );
    result |= run_consteval_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(
      test_small_vector_pop_back_0
    );

    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(
      test_small_vector_pop_back_sz
    );
    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(
      test_small_vector_pop_back_0
    );
  };
  //---------------------------------------------------------------------------------------------------------------------
  "test_small_vector_clear"_test = [&result]
  {
    auto fn_tmpl = []<typename value_type, typename size_type, typename szreq_ic>(
                     value_type const *, size_type const *, szreq_ic
                   ) -> metatests::test_result
    {
      constexpr size_type capacity_req = szreq_ic::value;
      std::array<value_type, 127> test_values;
      std::iota(begin(test_values), end(test_values), value_type(1));
      test_result tr;
      using st = small_vector<value_type, size_type, capacity_req>;
      st v;

      constexpr size_type second_growth
        = coll::detail::growth(coll::detail::growth(capacity_req, size_type(1)), size_type(1));
      static_assert(second_growth + 1u < test_values.size());
      for(size_type s{}; s != second_growth; ++s)
        emplace_back(v, test_values[s]);
        {
        tr |= constexpr_test(v.active_storage() == dynamic) | constexpr_test(size(v) == second_growth)
              | constexpr_test(!empty(v));
        std::span expected{test_values.data(), second_growth};
        tr |= constexpr_test(equal(v, expected));
        }
      size_type curr_capacity{capacity(v)};
      clear(v);
      tr |= constexpr_test(capacity(v) == curr_capacity) | constexpr_test(size(v) == 0u) | constexpr_test(empty(v))
            | constexpr_test(v.active_storage() == dynamic);

      return tr;
    };
    auto fn_size = [fn_tmpl]<typename value_type, typename size_type>(
                     value_type const * t, size_type const * u
                   ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, coll::at_least<value_type>(size_type(4))>;
      return fn_tmpl(t, u, size_req{});
    };
    // size_type capacity_req = coll::at_least<value_type>(size_type(4))
    auto fn_0 = [fn_tmpl]<typename value_type, typename size_type>(
                  value_type const * t, size_type const * u
                ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, 0u>;
      return fn_tmpl(t, u, size_req{});
    };

    using consteval_test_type_list_1 = metatests::type_list<uint8_t, uint16_t, uint32_t, uint64_t>;
    using consteval_test_type_list_2 = metatests::type_list<
      uint8_t,
      uint16_t,
      uint32_t,
      uint64_t,
      non_trivial_ptr,
      non_trivial_ptr_except,
      non_trivial_ptr_except_copy>;

    result |= run_consteval_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_size);
    result |= run_consteval_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_0);

    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_size);
    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_0);
  };

  //---------------------------------------------------------------------------------------------------------------------
  // storage buffered is covered by static_vector tests
  "test_small_vector_insert"_test = [&result]
  {
    auto fn_tmpl = []<typename value_type, typename size_type, typename szreq_ic>(
                     value_type const *, size_type const *, szreq_ic
                   ) -> metatests::test_result
    {
      constexpr size_type capacity_req = szreq_ic::value;
      std::array<value_type, 127> test_values;
      std::iota(begin(test_values), end(test_values), value_type(1));
      test_result tr;
      using st = small_vector<value_type, size_type, capacity_req>;
      constexpr size_type first_growth = coll::detail::growth(capacity_req, size_type(1));
      constexpr size_type second_growth = coll::detail::growth(first_growth, size_type(1));
      static_assert(second_growth + 1u < test_values.size());

        {
        st v;
        emplace_back(v, value_type(0xfe));
        emplace_back(v, value_type(0xff));
        std::span span1{test_values.data(), second_growth};
        auto res = insert(v, begin(v), begin(span1), end(span1));

        tr |= constexpr_test(res == no_error);
        std::array<value_type, second_growth + 2> expected;
        std::iota(begin(expected), std::prev(end(expected), 2), value_type(1));
        expected[expected.size() - 2] = value_type(0xfe);
        expected[expected.size() - 1] = value_type(0xff);
        tr |= constexpr_test(size(v) == expected.size()) | constexpr_test(equal(v, expected))
              | constexpr_test(v.active_storage() == dynamic);
        }

        {
        st v;
        emplace_back(v, value_type(0xfe));
        emplace_back(v, value_type(0xff));
        std::span span1{test_values.data(), second_growth};
        auto res = insert(v, std::next(begin(v), 1), begin(span1), end(span1));

        tr |= constexpr_test(res == no_error);
        std::array<value_type, second_growth + 2> expected;
        std::iota(std::next(begin(expected), 1), std::prev(end(expected), 1), value_type(1));
        expected[0] = value_type(0xfe);
        expected[expected.size() - 1] = value_type(0xff);
        tr |= constexpr_test(size(v) == expected.size()) | constexpr_test(equal(v, expected))
              | constexpr_test(v.active_storage() == dynamic);
        }

        {
        st v;
        emplace_back(v, value_type(0xfe));
        emplace_back(v, value_type(0xff));
        std::span span1{test_values.data(), second_growth};
        auto res = insert(v, std::next(begin(v), 2), begin(span1), end(span1));

        tr |= constexpr_test(res == no_error);
        std::array<value_type, second_growth + 2> expected;
        std::iota(std::next(begin(expected), 2), end(expected), value_type(1));
        expected[0] = value_type(0xfe);
        expected[1] = value_type(0xff);
        tr |= constexpr_test(size(v) == expected.size()) | constexpr_test(equal(v, expected))
              | constexpr_test(v.active_storage() == dynamic);
        }
      return tr;
    };

    auto fn_size = [fn_tmpl]<typename value_type, typename size_type>(
                     value_type const * t, size_type const * u
                   ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, coll::at_least<value_type>(size_type(4))>;
      return fn_tmpl(t, u, size_req{});
    };
    // size_type capacity_req = coll::at_least<value_type>(size_type(4))
    auto fn_0 = [fn_tmpl]<typename value_type, typename size_type>(
                  value_type const * t, size_type const * u
                ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, 0u>;
      return fn_tmpl(t, u, size_req{});
    };

    using consteval_test_type_list_1 = metatests::type_list<uint8_t, uint16_t, uint32_t, uint64_t>;
    using consteval_test_type_list_2 = metatests::type_list<
      uint8_t,
      uint16_t,
      uint32_t,
      uint64_t,
      non_trivial_ptr,
      non_trivial_ptr_except,
      non_trivial_ptr_except_copy>;

    result |= run_consteval_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_size);
    result |= run_consteval_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_0);

    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_size);
    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_0);
  };

  //---------------------------------------------------------------------------------------------------------------------
  // test for use with runtime asan to check proper unwinding and memory reclamation
  "test_small_vector_insert_except"_test = [&result]
  {
    auto fn_tmpl = []<typename value_type, typename size_type, typename szreq_ic>(
                     value_type const *, size_type const *, szreq_ic
                   ) -> metatests::test_result
    {
      constexpr size_type capacity_req = szreq_ic::value;
      std::array<value_type, 127> test_values{};
      std::iota(begin(test_values), end(test_values), value_type(1));
      std::array<value_type, 2> throwing;
      std::iota(begin(throwing), end(throwing), value_type(126));
      ++throwing.back();
      test_result tr;
      using st = small_vector<value_type, size_type, capacity_req>;
      st v;
      reserve(v, 4);
      insert(v, begin(v), begin(test_values), std::next(begin(test_values), int(capacity(v))));
      require_throw<std::runtime_error>([&v, &throwing]() { insert(v, v.begin(), begin(throwing), end(throwing)); });

      require_throw<std::runtime_error>(
        [&v, &throwing]()
        { insert(v, v.begin(), std::make_move_iterator(begin(throwing)), std::make_move_iterator(end(throwing))); }
      );
      v.front().set_value(128);
      require_throw<std::runtime_error>([&v, &test_values]()
                                        { insert(v, v.begin(), begin(test_values), std::next(begin(test_values), 10)); }
      );

      v.front().set_value(0);
      v.back().set_value(128);
      require_throw<std::runtime_error>([&v, &test_values]()
                                        { insert(v, v.begin(), begin(test_values), std::next(begin(test_values), 10)); }
      );
      return tr;
    };

    auto fn_size = [fn_tmpl]<typename value_type, typename size_type>(
                     value_type const * t, size_type const * u
                   ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, coll::at_least<value_type>(size_type(4))>;
      return fn_tmpl(t, u, size_req{});
    };
    // size_type capacity_req = coll::at_least<value_type>(size_type(4))
    auto fn_0 = [fn_tmpl]<typename value_type, typename size_type>(
                  value_type const * t, size_type const * u
                ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, 0u>;
      return fn_tmpl(t, u, size_req{});
    };

    using consteval_test_type_list_1 = metatests::type_list<non_trivial_throwing<128>, non_trivial_throwing_copy<128>>;

    result |= run_constexpr_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_size);
    result |= run_constexpr_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_0);
  };

  //---------------------------------------------------------------------------------------------------------------------
  // storage buffered is covered by static_vector tests
  "test_small_vector_emplace"_test = [&result]
  {
    auto fn_tmpl = []<typename value_type, typename size_type, typename szreq_ic>(
                     value_type const *, size_type const *, szreq_ic
                   ) -> metatests::test_result
    {
      constexpr size_type capacity_req = szreq_ic::value;
      std::array<value_type, 127> test_values;
      std::iota(begin(test_values), end(test_values), value_type(1));
      test_result tr;
      using st = small_vector<value_type, size_type, capacity_req>;
      constexpr size_type first_growth = coll::detail::growth(capacity_req, size_type(1));
      constexpr size_type second_growth = coll::detail::growth(first_growth, size_type(1));
      static_assert(second_growth + 1u < test_values.size());

        {
        st v;
        std::span span1{test_values.data(), second_growth};
        insert(v, begin(v), begin(span1), end(span1));

        auto res = emplace(v, begin(v), value_type(0x1f));

        tr |= constexpr_test(*res == value_type(0x1f));
        std::array<value_type, second_growth + 1> expected;
        std::iota(std::next(begin(expected), 1), end(expected), value_type(1));
        expected[0] = value_type(0x1f);
        tr |= constexpr_test(size(v) == expected.size()) | constexpr_test(equal(v, expected));
        }
        {
        st v;
        std::span span1{test_values.data(), second_growth};
        insert(v, begin(v), begin(span1), end(span1));

        auto res = emplace(v, std::next(begin(v), 1), value_type(0x1f));

        tr |= constexpr_test(*res == value_type(0x1f));
        std::array<value_type, second_growth + 1> expected;
        std::iota(begin(expected), end(expected), value_type(0));
        expected[0] = value_type(1);
        expected[1] = value_type(0x1f);
        tr |= constexpr_test(size(v) == expected.size()) | constexpr_test(equal(v, expected));
        }
        {
        st v;
        std::span span1{test_values.data(), second_growth};
        insert(v, begin(v), begin(span1), end(span1));

        auto res = emplace(v, end(v), value_type(0x1f));

        tr |= constexpr_test(*res == value_type(0x1f));
        std::array<value_type, second_growth + 1> expected;
        std::iota(begin(expected), std::prev(end(expected), 1), value_type(1));
        expected[expected.size() - 1] = value_type(0x1f);
        tr |= constexpr_test(size(v) == expected.size()) | constexpr_test(equal(v, expected));
        }
      return tr;
    };

    auto fn_size = [fn_tmpl]<typename value_type, typename size_type>(
                     value_type const * t, size_type const * u
                   ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, coll::at_least<value_type>(size_type(4))>;
      return fn_tmpl(t, u, size_req{});
    };
    // size_type capacity_req = coll::at_least<value_type>(size_type(4))
    auto fn_0 = [fn_tmpl]<typename value_type, typename size_type>(
                  value_type const * t, size_type const * u
                ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, 0u>;
      return fn_tmpl(t, u, size_req{});
    };

    using consteval_test_type_list_1 = metatests::type_list<uint8_t, uint16_t, uint32_t, uint64_t>;
    using consteval_test_type_list_2 = metatests::type_list<
      uint8_t,
      uint16_t,
      uint32_t,
      uint64_t,
      non_trivial_ptr,
      non_trivial_ptr_except,
      non_trivial_ptr_except_copy>;

    result |= run_consteval_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_size);
    result |= run_consteval_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_0);

    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_size);
    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_0);
  };

  //---------------------------------------------------------------------------------------------------------------------
  // test for use with runtime asan to check proper unwinding and memory reclamation

  "test_small_vector_emplace_except"_test = [&result]
  {
    auto fn_tmpl = []<typename value_type, typename size_type, typename szreq_ic>(
                     value_type const *, size_type const *, szreq_ic
                   ) -> metatests::test_result
    {
      constexpr size_type capacity_req = szreq_ic::value;
      std::array<value_type, 127> test_values{};
      std::iota(begin(test_values), end(test_values), value_type(1));
      test_result tr;
      using st = small_vector<value_type, size_type, capacity_req>;
      st v;
      reserve(v, 4);
      insert(v, begin(v), begin(test_values), std::next(begin(test_values), int(capacity(v))));
      require_throw<std::runtime_error>([&]() { emplace(v, v.begin(), value_type{128}); });

      value_type const tst{128};
      require_throw<std::runtime_error>([&]() { emplace(v, v.begin(), tst); });
      v.front().set_value(128);
      require_throw<std::runtime_error>([&]() { emplace(v, v.begin(), 1); });

      v.front().set_value(0);
      v.back().set_value(128);
      require_throw<std::runtime_error>([&]() { emplace(v, v.begin(), 1); });
      return tr;
    };

    auto fn_size = [fn_tmpl]<typename value_type, typename size_type>(
                     value_type const * t, size_type const * u
                   ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, coll::at_least<value_type>(size_type(4))>;
      return fn_tmpl(t, u, size_req{});
    };
    // size_type capacity_req = coll::at_least<value_type>(size_type(4))
    auto fn_0 = [fn_tmpl]<typename value_type, typename size_type>(
                  value_type const * t, size_type const * u
                ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, 0u>;
      return fn_tmpl(t, u, size_req{});
    };

    using consteval_test_type_list_1 = metatests::type_list<non_trivial_throwing<128>, non_trivial_throwing_copy<128>>;

    result |= run_constexpr_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_size);
    result |= run_constexpr_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_0);
  };

  //---------------------------------------------------------------------------------------------------------------------
  // template<typename value_type, typename size_type,
  //           size_type capacity_req =at_least<value_type>(size_type(4)),
  //           size_type capacity_req2 = capacity_req>

  "test_small_vector_copy_constr"_test = [&result]
  {
    auto fn_tmpl = []<typename value_type, typename size_type, typename szreq_ic, typename szreq_ic2>(
                     value_type const *, size_type const *, szreq_ic, szreq_ic2
                   ) -> metatests::test_result
    {
      constexpr size_type capacity_req = szreq_ic::value;
      constexpr size_type capacity_req2 = szreq_ic2::value;
      std::array<value_type, 127> test_values;
      std::iota(begin(test_values), end(test_values), value_type(1));
      test_result tr;
      using sa = small_vector<value_type, size_type, capacity_req>;
      using st = small_vector<value_type, size_type, capacity_req2>;
      constexpr size_type first_growth = coll::detail::growth(capacity_req, size_type(1));
      constexpr size_type second_growth = coll::detail::growth(first_growth, size_type(1));
      static_assert(second_growth + 1u < test_values.size());

      if constexpr(capacity_req != 0u)
        {
        st v;
        std::span expected{test_values.data(), capacity_req};
        insert(v, begin(v), begin(expected), end(expected));

        if constexpr(capacity_req == capacity_req2)
          tr |= constexpr_test(v.active_storage() == expected_istorage(capacity_req));
        tr |= constexpr_test(size(v) == expected.size()) | constexpr_test(equal(v, expected));
        st const & cv{v};
        sa cp{cv};
        if constexpr(capacity_req == capacity_req2)
          tr |= constexpr_test(cp.active_storage() == expected_istorage(capacity_req));
        tr |= constexpr_test(size(cp) == expected.size()) | constexpr_test(equal(cp, expected));
        }
        {
        st v;
        std::span expected{test_values.data(), second_growth};
        insert(v, begin(v), begin(expected), end(expected));

        if constexpr(capacity_req == capacity_req2)
          tr |= constexpr_test(v.active_storage() == dynamic);
        tr |= constexpr_test(size(v) == expected.size()) | constexpr_test(equal(v, expected));
        st const & cv{v};
        sa cp{cv};
        if constexpr(capacity_req == capacity_req2)
          tr |= constexpr_test(cp.active_storage() == dynamic);
        tr |= constexpr_test(size(cp) == expected.size()) | constexpr_test(equal(cp, expected));
        }
      return tr;
    };

    auto fn_size = [fn_tmpl]<typename value_type, typename size_type>(
                     value_type const * t, size_type const * u
                   ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, coll::at_least<value_type>(size_type(4))>;
      return fn_tmpl(t, u, size_req{}, size_req{});
    };
    // size_type capacity_req = coll::at_least<value_type>(size_type(4))
    auto fn_size_0 = [fn_tmpl]<typename value_type, typename size_type>(
                       value_type const * t, size_type const * u
                     ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, coll::at_least<value_type>(size_type(4))>;
      using size_req2 = std::integral_constant<size_type, 0u>;
      return fn_tmpl(t, u, size_req{}, size_req2{});
    };
    auto fn_0 = [fn_tmpl]<typename value_type, typename size_type>(
                  value_type const * t, size_type const * u
                ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, 0u>;
      return fn_tmpl(t, u, size_req{}, size_req{});
    };
    auto fn_size_25 = [fn_tmpl]<typename value_type, typename size_type>(
                        value_type const * t, size_type const * u
                      ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, coll::at_least<value_type>(size_type(4))>;
      using size_req2 = std::integral_constant<size_type, 25u>;
      return fn_tmpl(t, u, size_req{}, size_req2{});
    };
    auto fn_25_0 = [fn_tmpl]<typename value_type, typename size_type>(
                     value_type const * t, size_type const * u
                   ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, 25>;
      using size_req2 = std::integral_constant<size_type, 0u>;
      return fn_tmpl(t, u, size_req{}, size_req2{});
    };
    using consteval_test_type_list_1 = metatests::type_list<uint8_t, uint16_t, uint32_t, uint64_t>;
    using consteval_test_type_list_2 = metatests::type_list<
      uint8_t,
      uint16_t,
      uint32_t,
      uint64_t,
      non_trivial_ptr,
      non_trivial_ptr_except,
      non_trivial_ptr_except_copy>;

    result |= run_consteval_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_size);
    result |= run_consteval_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_size_0);
    result |= run_consteval_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_0);
    result |= run_consteval_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_size_25);
    result |= run_consteval_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_25_0);

    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_size);
    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_size_0);
    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_0);
    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_size_25);
    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_25_0);
  };

  //---------------------------------------------------------------------------------------------------------------------
  // test for use with runtime asan to check proper unwinding and memory reclamation
  "test_small_vector_copy_constr_except"_test = [&result]
  {
    auto fn_tmpl = []<typename value_type, typename size_type, typename szreq_ic>(
                     value_type const *, size_type const *, szreq_ic
                   ) -> metatests::test_result
    {
      constexpr size_type capacity_req = szreq_ic::value;

      std::array<value_type, 127> test_values;
      std::iota(begin(test_values), end(test_values), value_type(1));
      test_result tr;
      using st = small_vector<value_type, size_type, capacity_req>;
      constexpr size_type first_growth = coll::detail::growth(capacity_req, size_type(1));
      constexpr size_type second_growth = coll::detail::growth(first_growth, size_type(1));
      static_assert(second_growth + 1u < test_values.size());

      if constexpr(capacity_req != 0u)
        {
        st v;
        std::span expected{test_values.data(), capacity_req};
        insert(v, begin(v), begin(expected), end(expected));

        tr |= constexpr_test(v.active_storage() == expected_istorage(capacity_req))
              | constexpr_test(size(v) == expected.size()) | constexpr_test(equal(v, expected));
        v.front().set_value(128);
        st const & cv{v};
        bool except = false;
        try
          {
          st cp{cv};
          }
        catch(...)
          {
          except = true;
          }
        tr |= constexpr_test(except);
        }
      if constexpr(capacity_req != 0u)
        {
        st v;
        std::span expected{test_values.data(), capacity_req};
        insert(v, begin(v), begin(expected), end(expected));

        tr |= constexpr_test(v.active_storage() == expected_istorage(capacity_req))
              | constexpr_test(size(v) == expected.size()) | constexpr_test(equal(v, expected));
        v.front().set_value(0);
        v.back().set_value(128);
        st const & cv{v};
        bool except = false;
        try
          {
          st cp{cv};
          }
        catch(...)
          {
          except = true;
          }
        tr |= constexpr_test(except);
        }
        {
        st v;
        std::span expected{test_values.data(), second_growth};
        insert(v, begin(v), begin(expected), end(expected));

        tr |= constexpr_test(v.active_storage() == dynamic) | constexpr_test(size(v) == expected.size())
              | constexpr_test(equal(v, expected));
        v.front().set_value(128);
        st const & cv{v};
        bool except = false;
        try
          {
          st cp{cv};
          }
        catch(...)
          {
          except = true;
          }
        tr |= constexpr_test(except);
        }
        {
        st v;
        std::span expected{test_values.data(), second_growth};
        insert(v, begin(v), begin(expected), end(expected));

        tr |= constexpr_test(v.active_storage() == dynamic) | constexpr_test(size(v) == expected.size())
              | constexpr_test(equal(v, expected));
        v.front().set_value(0);
        v.back().set_value(128);
        st const & cv{v};
        bool except = false;
        try
          {
          st cp{cv};
          }
        catch(...)
          {
          except = true;
          }
        tr |= constexpr_test(except);
        }
      return tr;
    };

    auto fn_size = [fn_tmpl]<typename value_type, typename size_type>(
                     value_type const * t, size_type const * u
                   ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, coll::at_least<value_type>(size_type(4))>;
      return fn_tmpl(t, u, size_req{});
    };

    auto fn_0 = [fn_tmpl]<typename value_type, typename size_type>(
                  value_type const * t, size_type const * u
                ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, 0u>;
      return fn_tmpl(t, u, size_req{});
    };

    using consteval_test_type_list_1 = metatests::type_list<non_trivial_throwing<128>, non_trivial_throwing_copy<128>>;

    result |= run_constexpr_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_size);
    result |= run_constexpr_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_0);
  };

  //---------------------------------------------------------------------------------------------------------------------
  "test_small_vector_move_constr"_test = [&result]
  {
    auto fn_tmpl = []<typename value_type, typename size_type, typename szreq_ic>(
                     value_type const *, size_type const *, szreq_ic
                   ) -> metatests::test_result
    {
      constexpr size_type capacity_req = szreq_ic::value;

      std::array<value_type, 127> test_values;
      std::iota(begin(test_values), end(test_values), value_type(1));
      test_result tr;
      using st = small_vector<value_type, size_type, capacity_req>;
      constexpr size_type first_growth = coll::detail::growth(capacity_req, size_type(1));
      constexpr size_type second_growth = coll::detail::growth(first_growth, size_type(1));
      static_assert(second_growth + 1u < test_values.size());

      if constexpr(capacity_req != 0u)
        {
        st v;

        std::span expected{test_values.data(), capacity_req};
        insert(v, begin(v), begin(expected), end(expected));

        tr |= constexpr_test(v.active_storage() == expected_istorage(capacity_req))
              | constexpr_test(size(v) == expected.size()) | constexpr_test(equal(v, expected));

        st cp{std::move(v)};
        tr |= constexpr_test(cp.active_storage() == expected_istorage(capacity_req))
              | constexpr_test(size(cp) == expected.size()) | constexpr_test(equal(cp, expected));

        tr |= constexpr_test(v.active_storage() == expected_istorage(capacity_req)) | constexpr_test(size(v) == 0u);
        }

        {
        st v;
        std::span expected{test_values.data(), second_growth};
        insert(v, begin(v), begin(expected), end(expected));

        tr |= constexpr_test(v.active_storage() == dynamic) | constexpr_test(size(v) == expected.size())
              | constexpr_test(equal(v, expected));

        st cp{std::move(v)};
        tr |= constexpr_test(cp.active_storage() == dynamic) | constexpr_test(size(cp) == expected.size())
              | constexpr_test(equal(cp, expected));

        tr |= constexpr_test(v.active_storage() == expected_istorage(capacity_req)) | constexpr_test(size(v) == 0u);
        }
      return tr;
    };

    auto fn_size = [fn_tmpl]<typename value_type, typename size_type>(
                     value_type const * t, size_type const * u
                   ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, coll::at_least<value_type>(size_type(4))>;
      return fn_tmpl(t, u, size_req{});
    };
    // size_type capacity_req = coll::at_least<value_type>(size_type(4))
    auto fn_0 = [fn_tmpl]<typename value_type, typename size_type>(
                  value_type const * t, size_type const * u
                ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, 0u>;
      return fn_tmpl(t, u, size_req{});
    };

    using consteval_test_type_list_1 = metatests::type_list<uint8_t, uint16_t, uint32_t, uint64_t>;
    using consteval_test_type_list_2 = metatests::type_list<
      uint8_t,
      uint16_t,
      uint32_t,
      uint64_t,
      non_trivial_ptr,
      non_trivial_ptr_except,
      non_trivial_ptr_except_copy>;

    result |= run_consteval_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_size);
    result |= run_consteval_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_0);

    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_size);
    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_0);
  };

  //---------------------------------------------------------------------------------------------------------------------
  "test_small_vector_copy_assignment"_test = [&result]
  {
    auto fn_tmpl = []<typename value_type, typename size_type, typename szreq_ic, typename szreq_ic2>(
                     value_type const *, size_type const *, szreq_ic, szreq_ic2
                   ) -> metatests::test_result
    {
      constexpr size_type capacity_req = szreq_ic::value;
      constexpr size_type capacity_req2 = szreq_ic2::value;

      std::array<value_type, 127> test_values;
      std::iota(begin(test_values), end(test_values), value_type(1));
      test_result tr;
      using st = small_vector<value_type, size_type, capacity_req>;
      using sa = small_vector<value_type, size_type, capacity_req2>;
      constexpr size_type first_growth = coll::detail::growth(capacity_req, size_type(1));
      constexpr size_type second_growth = coll::detail::growth(first_growth, size_type(1));
      static_assert(second_growth + 1u < test_values.size());

      // buffered = buffered
      if constexpr(expected_istorage(capacity_req) == buffered)
        {
        sa v;
        std::span expected{test_values.data(), capacity_req};
        insert(v, begin(v), begin(expected), end(expected));
        if constexpr(capacity_req == capacity_req2)
          tr |= constexpr_test(v.active_storage() == buffered);
        tr |= constexpr_test(size(v) == expected.size()) | constexpr_test(equal(v, expected));
        sa const & cv{v};

        std::span redzone{test_values.data() + 5, capacity_req - 1};
        st cp;
        insert(cp, begin(cp), begin(redzone), end(redzone));
        if constexpr(capacity_req == capacity_req2)
          tr |= constexpr_test(cp.active_storage() == buffered);
        tr |= constexpr_test(size(cp) == redzone.size()) | constexpr_test(equal(cp, redzone));
        cp.assign(cv);
        if constexpr(capacity_req == capacity_req2)
          tr |= constexpr_test(cp.active_storage() == buffered);
        tr |= constexpr_test(size(cp) == expected.size()) | constexpr_test(equal(v, expected));
        }
      // dyanmic = buffered
      if constexpr(expected_istorage(capacity_req) == buffered)
        {
        sa v;
        std::span expected{test_values.data(), capacity_req};
        insert(v, begin(v), begin(expected), end(expected));
        if constexpr(capacity_req == capacity_req2)
          tr |= constexpr_test(v.active_storage() == buffered);
        tr |= constexpr_test(size(v) == expected.size()) | constexpr_test(equal(v, expected));
        sa const & cv{v};

        std::span redzone{test_values.data() + 5, second_growth};
        st cp;
        insert(cp, begin(cp), begin(redzone), end(redzone));
        if constexpr(capacity_req == capacity_req2)
          tr |= constexpr_test(cp.active_storage() == dynamic);
        tr |= constexpr_test(size(cp) == redzone.size()) | constexpr_test(equal(cp, redzone));
        cp.assign(cv);
        tr |= constexpr_test(cp.active_storage() == buffered) | constexpr_test(size(cp) == expected.size())
              | constexpr_test(equal(v, expected));
        }
      // buffered = dynamic
      if constexpr(expected_istorage(capacity_req) == buffered)
        {
        sa v;
        std::span expected{test_values.data(), second_growth};
        insert(v, begin(v), begin(expected), end(expected));
        if constexpr(capacity_req == capacity_req2)
          tr |= constexpr_test(v.active_storage() == dynamic);
        tr |= constexpr_test(size(v) == expected.size()) | constexpr_test(equal(v, expected));
        sa const & cv{v};

        std::span redzone{test_values.data() + 5, capacity_req - 1};
        st cp;
        insert(cp, begin(cp), begin(redzone), end(redzone));
        if constexpr(capacity_req == capacity_req2)
          tr |= constexpr_test(cp.active_storage() == buffered) | constexpr_test(size(cp) == redzone.size())
                | constexpr_test(equal(cp, redzone));
        cp.assign(cv);
        tr |= constexpr_test(cp.active_storage() == dynamic) | constexpr_test(size(cp) == expected.size())
              | constexpr_test(equal(v, expected));
        }
        // dyanmic = dynamic
        {
        sa v;
        std::span expected{test_values.data(), second_growth};
        insert(v, begin(v), begin(expected), end(expected));
        if constexpr(capacity_req == capacity_req2)
          tr |= constexpr_test(v.active_storage() == dynamic);
        tr |= constexpr_test(size(v) == expected.size()) | constexpr_test(equal(v, expected));
        sa const & cv{v};

        std::span redzone{test_values.data() + 5, second_growth};
        st cp;
        insert(cp, begin(cp), begin(redzone), end(redzone));
        tr |= constexpr_test(cp.active_storage() == dynamic) | constexpr_test(size(cp) == redzone.size())
              | constexpr_test(equal(cp, redzone));
        cp.assign(cv);
        tr |= constexpr_test(cp.active_storage() == dynamic) | constexpr_test(size(cp) == expected.size())
              | constexpr_test(equal(v, expected));
        }
      return tr;
    };

    auto fn_size = [fn_tmpl]<typename value_type, typename size_type>(
                     value_type const * t, size_type const * u
                   ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, coll::at_least<value_type>(size_type(4))>;
      return fn_tmpl(t, u, size_req{}, size_req{});
    };
    // size_type capacity_req = coll::at_least<value_type>(size_type(4))
    auto fn_size_0 = [fn_tmpl]<typename value_type, typename size_type>(
                       value_type const * t, size_type const * u
                     ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, coll::at_least<value_type>(size_type(4))>;
      using size_req2 = std::integral_constant<size_type, 0u>;
      return fn_tmpl(t, u, size_req{}, size_req2{});
    };
    auto fn_0 = [fn_tmpl]<typename value_type, typename size_type>(
                  value_type const * t, size_type const * u
                ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, 0u>;
      return fn_tmpl(t, u, size_req{}, size_req{});
    };
    auto fn_size_25 = [fn_tmpl]<typename value_type, typename size_type>(
                        value_type const * t, size_type const * u
                      ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, coll::at_least<value_type>(size_type(4))>;
      using size_req2 = std::integral_constant<size_type, 25u>;
      return fn_tmpl(t, u, size_req{}, size_req2{});
    };
    auto fn_25_0 = [fn_tmpl]<typename value_type, typename size_type>(
                     value_type const * t, size_type const * u
                   ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, 25>;
      using size_req2 = std::integral_constant<size_type, 0u>;
      return fn_tmpl(t, u, size_req{}, size_req2{});
    };
    using consteval_test_type_list_1 = metatests::type_list<uint8_t, uint16_t, uint32_t, uint64_t>;
    using consteval_test_type_list_2 = metatests::type_list<
      uint8_t,
      uint16_t,
      uint32_t,
      uint64_t,
      non_trivial_ptr,
      non_trivial_ptr_except,
      non_trivial_ptr_except_copy>;

    result |= run_consteval_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_size);
    result |= run_consteval_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_size_0);
    result |= run_consteval_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_0);
    result |= run_consteval_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_size_25);
    result |= run_consteval_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_25_0);

    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_size);
    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_size_0);
    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_0);
    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_size_25);
    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_25_0);
  };
  //---------------------------------------------------------------------------------------------------------------------
  "test_small_vector_copy_assignment_except"_test = [&result]
  {
    auto fn_tmpl = []<typename value_type, typename size_type, typename szreq_ic>(
                     value_type const *, size_type const *, szreq_ic
                   ) -> metatests::test_result
    {
      constexpr size_type capacity_req = szreq_ic::value;
      std::array<value_type, 127> test_values;
      std::iota(begin(test_values), end(test_values), value_type(1));
      test_result tr;
      using st = small_vector<value_type, size_type, capacity_req>;
      constexpr size_type first_growth = coll::detail::growth(capacity_req, size_type(1));
      constexpr size_type second_growth = coll::detail::growth(first_growth, size_type(1));
      static_assert(second_growth + 1u < test_values.size());

      // buffered = buffered left throws
      if constexpr(expected_istorage(capacity_req) == buffered)
        {
        st v;
        std::span expected{test_values.data(), capacity_req};
        insert(v, begin(v), begin(expected), end(expected));

        tr |= constexpr_test(v.active_storage() == buffered) | constexpr_test(size(v) == expected.size())
              | constexpr_test(equal(v, expected));
        st const & cv{v};

        std::span redzone{test_values.data() + 5, capacity_req - 1};
        st cp;
        insert(cp, begin(cp), begin(redzone), end(redzone));
        tr |= constexpr_test(cp.active_storage() == buffered) | constexpr_test(size(cp) == redzone.size())
              | constexpr_test(equal(cp, redzone));
        cp.back().set_value(128);
        bool except = false;

        try
          {
          cp = cv;
          }
        catch(...)
          {
          except = true;
          }
        tr |= constexpr_test(not except);
        }
      // buffered = buffered right throws
      if constexpr(expected_istorage(capacity_req) == buffered)
        {
        st v;
        std::span expected{test_values.data(), capacity_req};
        insert(v, begin(v), begin(expected), end(expected));

        tr |= constexpr_test(v.active_storage() == buffered) | constexpr_test(size(v) == expected.size())
              | constexpr_test(equal(v, expected));
        st const & cv{v};

        std::span redzone{test_values.data() + 5, capacity_req - 1};
        st cp;
        insert(cp, begin(cp), begin(redzone), end(redzone));
        tr |= constexpr_test(cp.active_storage() == buffered) | constexpr_test(size(cp) == redzone.size())
              | constexpr_test(equal(cp, redzone));
        v.back().set_value(128);
        bool except = false;
        try
          {
          cp = cv;
          }
        catch(...)
          {
          except = true;
          }
        tr |= constexpr_test(except);
        }

      // dyanmic = buffered left throws
      if constexpr(expected_istorage(capacity_req) == buffered)
        {
        st v;
        std::span expected{test_values.data(), capacity_req};
        insert(v, begin(v), begin(expected), end(expected));

        tr |= constexpr_test(v.active_storage() == buffered) | constexpr_test(size(v) == expected.size())
              | constexpr_test(equal(v, expected));
        st const & cv{v};

        std::span redzone{test_values.data() + 5, second_growth};
        st cp;
        insert(cp, begin(cp), begin(redzone), end(redzone));
        tr |= constexpr_test(cp.active_storage() == dynamic) | constexpr_test(size(cp) == redzone.size())
              | constexpr_test(equal(cp, redzone));
        cp.back().set_value(128);
        bool except = false;
        try
          {
          cp = cv;
          }
        catch(...)
          {
          except = true;
          }
        tr |= constexpr_test(not except);
        }
      // dyanmic = buffered right throws
      if constexpr(expected_istorage(capacity_req) == buffered)
        {
        st v;
        std::span expected{test_values.data(), capacity_req};
        insert(v, begin(v), begin(expected), end(expected));

        tr |= constexpr_test(v.active_storage() == buffered) | constexpr_test(size(v) == expected.size())
              | constexpr_test(equal(v, expected));
        st const & cv{v};

        std::span redzone{test_values.data() + 5, second_growth};
        st cp;
        insert(cp, begin(cp), begin(redzone), end(redzone));
        tr |= constexpr_test(cp.active_storage() == dynamic) | constexpr_test(size(cp) == redzone.size())
              | constexpr_test(equal(cp, redzone));
        v.back().set_value(128);
        bool except = false;
        try
          {
          cp = cv;
          }
        catch(...)
          {
          except = true;
          }
        tr |= constexpr_test(except);
        }

      // buffered = dynamic buffered left throws
      if constexpr(expected_istorage(capacity_req) == buffered)
        {
        st v;
        std::span expected{test_values.data(), second_growth};
        insert(v, begin(v), begin(expected), end(expected));

        tr |= constexpr_test(v.active_storage() == dynamic) | constexpr_test(size(v) == expected.size())
              | constexpr_test(equal(v, expected));
        st const & cv{v};

        std::span redzone{test_values.data() + 5, capacity_req - 1};
        st cp;
        insert(cp, begin(cp), begin(redzone), end(redzone));
        tr |= constexpr_test(cp.active_storage() == buffered) | constexpr_test(size(cp) == redzone.size())
              | constexpr_test(equal(cp, redzone));
        cp.back().set_value(128);
        bool except = false;
        try
          {
          cp = cv;
          }
        catch(...)
          {
          except = true;
          }
        tr |= constexpr_test(not except);
        }
      // buffered = dynamic buffered right throws
      if constexpr(expected_istorage(capacity_req) == buffered)
        {
        st v;
        std::span expected{test_values.data(), second_growth};
        insert(v, begin(v), begin(expected), end(expected));

        tr |= constexpr_test(v.active_storage() == dynamic) | constexpr_test(size(v) == expected.size())
              | constexpr_test(equal(v, expected));
        st const & cv{v};

        std::span redzone{test_values.data() + 5, capacity_req - 1};
        st cp;
        insert(cp, begin(cp), begin(redzone), end(redzone));
        tr |= constexpr_test(cp.active_storage() == buffered) | constexpr_test(size(cp) == redzone.size())
              | constexpr_test(equal(cp, redzone));
        v.back().set_value(128);
        bool except = false;
        try
          {
          cp = cv;
          }
        catch(...)
          {
          except = true;
          }
        tr |= constexpr_test(except);
        }

        // dyanmic = dynamic left throws
        {
        st v;
        std::span expected{test_values.data(), second_growth};
        insert(v, begin(v), begin(expected), end(expected));

        tr |= constexpr_test(v.active_storage() == dynamic) | constexpr_test(size(v) == expected.size())
              | constexpr_test(equal(v, expected));
        st const & cv{v};

        std::span redzone{test_values.data() + 1, second_growth};
        st cp;
        insert(cp, begin(cp), begin(redzone), end(redzone));
        tr |= constexpr_test(cp.active_storage() == dynamic) | constexpr_test(size(cp) == redzone.size())
              | constexpr_test(equal(cp, redzone));

        cp.back().set_value(128);
        bool except = false;
        try
          {
          cp = cv;
          }
        catch(...)
          {
          except = true;
          }
        tr |= constexpr_test(not except);
        }

        // dyanmic = dynamic right throws
        {
        st v;
        std::span expected{test_values.data(), second_growth};
        insert(v, begin(v), begin(expected), end(expected));

        tr |= constexpr_test(v.active_storage() == dynamic) | constexpr_test(size(v) == expected.size())
              | constexpr_test(equal(v, expected));
        st const & cv{v};

        std::span redzone{test_values.data() + 5, second_growth};
        st cp;
        insert(cp, begin(cp), begin(redzone), end(redzone));
        tr |= constexpr_test(cp.active_storage() == dynamic) | constexpr_test(size(cp) == redzone.size())
              | constexpr_test(equal(cp, redzone));
        v.back().set_value(128);
        bool except = false;
        try
          {
          cp = cv;
          }
        catch(...)
          {
          except = true;
          }
        tr |= constexpr_test(except);
        }
      return tr;
    };

    auto fn_size = [fn_tmpl]<typename value_type, typename size_type>(
                     value_type const * t, size_type const * u
                   ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, coll::at_least<value_type>(size_type(4))>;
      return fn_tmpl(t, u, size_req{});
    };
    // size_type capacity_req = coll::at_least<value_type>(size_type(4))
    auto fn_0 = [fn_tmpl]<typename value_type, typename size_type>(
                  value_type const * t, size_type const * u
                ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, 0u>;
      return fn_tmpl(t, u, size_req{});
    };

    using consteval_test_type_list_1 = metatests::type_list<non_trivial_throwing<128>, non_trivial_throwing_copy<128>>;

    result |= run_constexpr_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_size);
    result |= run_constexpr_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_0);
  };

  //---------------------------------------------------------------------------------------------------------------------
  "test_small_vector_move_assignment"_test = [&result]
  {
    auto fn_tmpl = []<typename value_type, typename size_type, typename szreq_ic>(
                     value_type const *, size_type const *, szreq_ic
                   ) -> metatests::test_result
    {
      constexpr size_type capacity_req = szreq_ic::value;
      std::array<value_type, 127> test_values;
      std::iota(begin(test_values), end(test_values), value_type(1));
      test_result tr;
      using st = small_vector<value_type, size_type, capacity_req>;
      constexpr size_type first_growth = coll::detail::growth(capacity_req, size_type(1));
      constexpr size_type second_growth = coll::detail::growth(first_growth, size_type(1));
      static_assert(second_growth + 1u < test_values.size());

      // buffered = buffered
      if constexpr(expected_istorage(capacity_req) == buffered)
        {
        st v;
        std::span expected{test_values.data(), capacity_req};
        insert(v, begin(v), begin(expected), end(expected));

        tr |= constexpr_test(v.active_storage() == buffered) | constexpr_test(size(v) == expected.size())
              | constexpr_test(equal(v, expected));

        std::span redzone{test_values.data() + 5, capacity_req - 1};
        st cp;
        insert(cp, begin(cp), begin(redzone), end(redzone));
        tr |= constexpr_test(cp.active_storage() == buffered) | constexpr_test(size(cp) == redzone.size())
              | constexpr_test(equal(cp, redzone));
        cp = std::move(v);
        tr |= constexpr_test(cp.active_storage() == buffered) | constexpr_test(size(cp) == expected.size())
              | constexpr_test(equal(cp, expected));

        tr |= constexpr_test(v.active_storage() == buffered) | constexpr_test(size(v) == 0u);
        }
      // buffered = dynamic
      if constexpr(expected_istorage(capacity_req) == buffered)
        {
        st v;
        std::span expected{test_values.data(), second_growth};
        insert(v, begin(v), begin(expected), end(expected));

        tr |= constexpr_test(v.active_storage() == dynamic) | constexpr_test(size(v) == expected.size())
              | constexpr_test(equal(v, expected));

        std::span redzone{test_values.data() + 5, capacity_req - 1};
        st cp;
        insert(cp, begin(cp), begin(redzone), end(redzone));
        tr |= constexpr_test(cp.active_storage() == buffered) | constexpr_test(size(cp) == redzone.size())
              | constexpr_test(equal(cp, redzone));
        cp = std::move(v);
        tr |= constexpr_test(cp.active_storage() == dynamic) | constexpr_test(size(cp) == expected.size())
              | constexpr_test(equal(cp, expected));

        tr |= constexpr_test(v.active_storage() == buffered) | constexpr_test(size(v) == 0u);
        }
      // dynamic = buffered
      if constexpr(expected_istorage(capacity_req) == buffered)
        {
        st v;
        std::span expected{test_values.data(), capacity_req};
        insert(v, begin(v), begin(expected), end(expected));

        tr |= constexpr_test(v.active_storage() == buffered) | constexpr_test(size(v) == expected.size())
              | constexpr_test(equal(v, expected));

        std::span redzone{test_values.data() + 5, second_growth};
        st cp;
        insert(cp, begin(cp), begin(redzone), end(redzone));
        tr |= constexpr_test(cp.active_storage() == dynamic) | constexpr_test(size(cp) == redzone.size())
              | constexpr_test(equal(cp, redzone));
        cp = std::move(v);
        tr |= constexpr_test(cp.active_storage() == buffered) | constexpr_test(size(cp) == expected.size())
              | constexpr_test(equal(cp, expected));

        tr |= constexpr_test(v.active_storage() == buffered) | constexpr_test(size(v) == 0u);
        }
        // dynamic = dynamic
        {
        st v;
        std::span expected{test_values.data(), second_growth};
        insert(v, begin(v), begin(expected), end(expected));

        tr |= constexpr_test(v.active_storage() == dynamic) | constexpr_test(size(v) == expected.size())
              | constexpr_test(equal(v, expected));

        std::span redzone{test_values.data() + 5, second_growth + 1};
        st cp;
        insert(cp, begin(cp), begin(redzone), end(redzone));
        tr |= constexpr_test(cp.active_storage() == dynamic) | constexpr_test(size(cp) == redzone.size())
              | constexpr_test(equal(cp, redzone));
        cp = std::move(v);
        tr |= constexpr_test(cp.active_storage() == dynamic) | constexpr_test(size(cp) == expected.size())
              | constexpr_test(equal(cp, expected));

        tr |= constexpr_test(v.active_storage() == expected_istorage(capacity_req)) | constexpr_test(size(v) == 0u);
        }
      return tr;
    };

    auto fn_size = [fn_tmpl]<typename value_type, typename size_type>(
                     value_type const * t, size_type const * u
                   ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, coll::at_least<value_type>(size_type(4))>;
      return fn_tmpl(t, u, size_req{});
    };
    // size_type capacity_req = coll::at_least<value_type>(size_type(4))
    auto fn_0 = [fn_tmpl]<typename value_type, typename size_type>(
                  value_type const * t, size_type const * u
                ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, 0u>;
      return fn_tmpl(t, u, size_req{});
    };

    using consteval_test_type_list_1 = metatests::type_list<uint8_t, uint16_t, uint32_t, uint64_t>;
    using consteval_test_type_list_2 = metatests::type_list<
      uint8_t,
      uint16_t,
      uint32_t,
      uint64_t,
      non_trivial_ptr,
      non_trivial_ptr_except,
      non_trivial_ptr_except_copy>;

    result |= run_consteval_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_size);
    result |= run_consteval_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_0);

    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_size);
    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_0);
  };

  //---------------------------------------------------------------------------------------------------------------------

  "test_at_max_size"_test = [&result]
  {
    auto fn_tmpl = []<typename value_type, typename szreq_ic>(value_type const *, szreq_ic) -> metatests::test_result
    {
      using size_type = uint8_t;
      constexpr size_type capacity_req = szreq_ic::value;

      using st = small_vector<value_type, size_type, capacity_req>;
      test_result tr;
      tr |= constexpr_test(st::max_size() == std::numeric_limits<size_type>::max());

      std::array<value_type, st::max_size()> test_values;
      std::iota(begin(test_values), std::prev(end(test_values), 1), value_type(1));
      test_values[st::max_size() - 1] = std::numeric_limits<size_type>::max();

      st v;
        {
        std::span expected{test_values.data(), st::max_size() - 1};
        insert(v, begin(v), begin(expected), end(expected));
        tr |= constexpr_test(v.active_storage() == dynamic) | constexpr_test(size(v) == expected.size())
              | constexpr_test(equal(v, expected));
        }
        {
        std::span expected{test_values.data(), st::max_size()};
        emplace_back(v, expected.back());
        tr |= constexpr_test(size(v) == expected.size()) | constexpr_test(equal(v, expected));
        }
      return tr;
    };
    auto fn_size = [fn_tmpl]<typename value_type>(value_type const * t) -> metatests::test_result
    {
      using size_req = std::integral_constant<uint8_t, coll::at_least<value_type>(uint8_t(4))>;
      return fn_tmpl(t, size_req{});
    };

    auto fn_0 = [fn_tmpl]<typename value_type>(value_type const * t) -> metatests::test_result
    {
      using size_req = std::integral_constant<uint8_t, 0u>;
      return fn_tmpl(t, size_req{});
    };
    using consteval_test_type_list_1 = metatests::type_list<uint8_t, uint16_t, uint32_t, uint64_t>;
    using consteval_test_type_list_2 = metatests::type_list<
      uint8_t,
      uint16_t,
      uint32_t,
      uint64_t,
      non_trivial_ptr,
      non_trivial_ptr_except,
      non_trivial_ptr_except_copy>;

    result |= run_consteval_test<consteval_test_type_list_1>(fn_size);
    result |= run_consteval_test<consteval_test_type_list_2>(fn_0);

    result |= run_constexpr_test<consteval_test_type_list_2>(fn_size);
    result |= run_constexpr_test<consteval_test_type_list_2>(fn_0);
  };

  //---------------------------------------------------------------------------------------------------------
  "test_small_vector_resize"_test = [&result]
  {
    auto fn_tmpl = []<typename value_type, typename size_type, typename szreq_ic>(
                     value_type const *, size_type const *, szreq_ic
                   ) -> metatests::test_result
    {
      constexpr size_type capacity_req = szreq_ic::value;
      test_result tr;
      using vector_type = small_vector<value_type, size_type, capacity_req>;
      vector_type vec;
        {
        std::array<value_type, 2> expected;
        std::iota(begin(expected), end(expected), value_type(1));
        emplace_back(vec, value_type{1});
        reserve(vec, 5);
        tr |= constexpr_test(size(vec) == 1);
        emplace_back(vec, value_type{2});
        tr |= constexpr_test(equal(vec, expected));
        }
      clear(vec);
        {
        std::array<value_type, 10> expected{};
        resize(vec, 10);
        tr |= constexpr_test(size(vec) == 10) | constexpr_test(equal(vec, expected));
        }
        {
        resize(vec, 20);
        std::array<value_type, 20> expected{};
        tr |= constexpr_test(size(vec) == 20) | constexpr_test(equal(vec, expected));
        }
        {
        resize(vec, 15);
        std::array<value_type, 15> expected{};
        tr |= constexpr_test(size(vec) == 15) | constexpr_test(equal(vec, expected));
        }
      clear(vec);
        {
        std::array<value_type, 10> expected;
        std::iota(begin(expected), end(expected), value_type(1));
        insert(vec, begin(vec), begin(expected), end(expected));
        tr |= constexpr_test(size(vec) == 10) | constexpr_test(equal(vec, expected));
        }
        {
        resize(vec, 15);
        std::array<value_type, 15> expected{};
        std::iota(begin(expected), begin(expected) + 10, value_type(1));
        tr |= constexpr_test(size(vec) == 15) | constexpr_test(equal(vec, expected));
        }
        {
        resize(vec, 5);
        std::array<value_type, 5> expected{};
        std::iota(begin(expected), end(expected), value_type(1));
        tr |= constexpr_test(size(vec) == 5) | constexpr_test(equal(vec, expected));
        }
        {
        resize(vec, 125);
        std::array<value_type, 125> expected{};
        std::iota(begin(expected), begin(expected) + 5, value_type(1));
        tr |= constexpr_test(size(vec) == 125) | constexpr_test(equal(vec, expected));
        }
        {
        resize(vec, 25);
        std::array<value_type, 25> expected{};
        std::iota(begin(expected), begin(expected) + 5, value_type(1));
        tr |= constexpr_test(size(vec) == 25) | constexpr_test(equal(vec, expected));
        }
      return tr;
    };

    auto fn_size = [fn_tmpl]<typename value_type, typename size_type>(
                     value_type const * t, size_type const * u
                   ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, coll::at_least<value_type>(size_type(4))>;
      return fn_tmpl(t, u, size_req{});
    };
    // size_type capacity_req = coll::at_least<value_type>(size_type(4))
    auto fn_0 = [fn_tmpl]<typename value_type, typename size_type>(
                  value_type const * t, size_type const * u
                ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, 0u>;
      return fn_tmpl(t, u, size_req{});
    };

    using consteval_test_type_list_1 = metatests::type_list<uint8_t, uint16_t, uint32_t, uint64_t>;
    using consteval_test_type_list_2 = metatests::type_list<
      uint8_t,
      uint16_t,
      uint32_t,
      uint64_t,
      non_trivial_ptr,
      non_trivial_ptr_except,
      non_trivial_ptr_except_copy>;

    result |= run_consteval_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_size);
    result |= run_consteval_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_0);

    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_size);
    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_0);
  };

  //---------------------------------------------------------------------------------------------------------------------
  "test_small_vector_shrink_to_fit"_test = [&result]
  {
    auto fn_tmpl = []<typename value_type, typename size_type, typename szreq_ic>(
                     value_type const *, size_type const *, szreq_ic
                   ) -> metatests::test_result
    {
      constexpr size_type capacity_req = szreq_ic::value;
      test_result tr;
      using vector_type = small_vector<value_type, size_type, capacity_req>;
      constexpr size_type cap{vector_type::buffered_capacity()};
      constexpr size_type first_growth = coll::detail::growth(capacity_req, size_type(1));
        {
        vector_type vec;
        std::array<value_type, first_growth> expected;
        std::iota(begin(expected), end(expected), value_type(1));
        insert(vec, begin(vec), begin(expected), end(expected));
        reserve(vec, 2 * first_growth);
        tr |= constexpr_test(size(vec) == first_growth) | constexpr_test(capacity(vec) == 2 * first_growth)
              | constexpr_test(equal(vec, expected));
        auto res = shrink_to_fit(vec);
        tr |= constexpr_test(res == no_error) | constexpr_test(size(vec) == first_growth)
              | constexpr_test(capacity(vec) == first_growth) | constexpr_test(vec.active_storage() == dynamic)
              | constexpr_test(equal(vec, expected));
        }
      if constexpr(expected_istorage(capacity_req) == buffered)
        {
          {
          vector_type vec;
          std::array<value_type, cap> expected;
          std::iota(begin(expected), end(expected), value_type(1));
          insert(vec, begin(vec), begin(expected), end(expected));
          tr |= constexpr_test(size(vec) == cap);
          reserve(vec, first_growth);
          tr |= constexpr_test(size(vec) == cap) | constexpr_test(capacity(vec) == first_growth)
                | constexpr_test(equal(vec, expected));
          auto res = shrink_to_fit(vec);
          tr |= constexpr_test(res == no_error) | constexpr_test(capacity(vec) == cap)
                | constexpr_test(size(vec) == cap) | constexpr_test(vec.active_storage() == buffered)
                | constexpr_test(equal(vec, expected));
          }
          {
          vector_type vec;
          std::array<value_type, cap> expected;
          std::iota(begin(expected), end(expected), value_type(1));
          insert(vec, begin(vec), begin(expected), end(expected));
          tr |= constexpr_test(size(vec) == cap) | constexpr_test(capacity(vec) == cap)
                | constexpr_test(equal(vec, expected));
          auto res = shrink_to_fit(vec);
          tr |= constexpr_test(res == no_error) | constexpr_test(capacity(vec) == cap)
                | constexpr_test(vec.active_storage() == buffered) | constexpr_test(size(vec) == cap)
                | constexpr_test(equal(vec, expected));
          }
        }
      return tr;
    };

    auto fn_size = [fn_tmpl]<typename value_type, typename size_type>(
                     value_type const * t, size_type const * u
                   ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, coll::at_least<value_type>(size_type(4))>;
      return fn_tmpl(t, u, size_req{});
    };
    // size_type capacity_req = coll::at_least<value_type>(size_type(4))
    auto fn_0 = [fn_tmpl]<typename value_type, typename size_type>(
                  value_type const * t, size_type const * u
                ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, 0u>;
      return fn_tmpl(t, u, size_req{});
    };

    using consteval_test_type_list_1 = metatests::type_list<uint8_t, uint16_t, uint32_t, uint64_t>;
    using consteval_test_type_list_2 = metatests::type_list<
      uint8_t,
      uint16_t,
      uint32_t,
      uint64_t,
      non_trivial_ptr,
      non_trivial_ptr_except,
      non_trivial_ptr_except_copy>;

    result |= run_consteval_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_size);
    result |= run_consteval_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_0);

    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_size);
    result |= run_constexpr_test_dual<consteval_test_type_list_2, constexpr_size_type_traits_list>(fn_0);
  };

  //---------------------------------------------------------------------------------------------------------------------

  "test_small_vector_shrink_to_fit_except"_test = [&result]
  {
    auto fn_tmpl = []<typename value_type, typename size_type, typename szreq_ic>(
                     value_type const *, size_type const *, szreq_ic
                   ) -> metatests::test_result
    {
      constexpr size_type capacity_req = szreq_ic::value;

      test_result tr;
      using vector_type = small_vector<value_type, size_type, capacity_req>;
      constexpr size_type cap{vector_type::buffered_capacity()};
      constexpr size_type first_growth = coll::detail::growth(capacity_req, size_type(1));
        {
        vector_type vec;
        std::array<value_type, first_growth> expected;
        std::iota(begin(expected), end(expected), value_type(1));
        insert(vec, begin(vec), begin(expected), end(expected));
        reserve(vec, 2 * first_growth);
        tr |= constexpr_test(size(vec) == first_growth) | constexpr_test(capacity(vec) == 2 * first_growth)
              | constexpr_test(equal(vec, expected));
        back(vec).set_value(128);
        expected.back().set_value(128);
        bool except = false;
        try
          {
          shrink_to_fit(vec);
          }
        catch(...)
          {
          except = true;
          }
        tr |= constexpr_test(except) | constexpr_test(size(vec) == first_growth) | constexpr_test(equal(vec, expected));
        }
      if constexpr(expected_istorage(capacity_req) == buffered)
        {
          {
          vector_type vec;
          std::array<value_type, cap> expected;
          std::iota(begin(expected), end(expected), value_type(1));
          insert(vec, begin(vec), begin(expected), end(expected));
          tr |= constexpr_test(size(vec) == cap);
          reserve(vec, first_growth);
          tr |= constexpr_test(size(vec) == cap) | constexpr_test(capacity(vec) == first_growth)
                | constexpr_test(equal(vec, expected));
          back(vec).set_value(128);
          expected.back().set_value(128);
          bool except = false;
          try
            {
            shrink_to_fit(vec);
            }
          catch(...)
            {
            except = true;
            }
          tr |= constexpr_test(except) | constexpr_test(size(vec) == cap) | constexpr_test(equal(vec, expected));
          }
          {
          vector_type vec;
          std::array<value_type, cap> expected;
          std::iota(begin(expected), end(expected), value_type(1));
          insert(vec, begin(vec), begin(expected), end(expected));
          tr |= constexpr_test(size(vec) == cap) | constexpr_test(capacity(vec) == cap)
                | constexpr_test(equal(vec, expected));
          back(vec).set_value(128);
          expected.back().set_value(128);
          bool except = false;
          try
            {
            shrink_to_fit(vec);
            }
          catch(...)
            {
            except = true;
            }
          tr |= constexpr_test(not except) | constexpr_test(size(vec) == cap) | constexpr_test(equal(vec, expected));
          }
        }
      return tr;
    };

    auto fn_size = [fn_tmpl]<typename value_type, typename size_type>(
                     value_type const * t, size_type const * u
                   ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, coll::at_least<value_type>(size_type(4))>;
      return fn_tmpl(t, u, size_req{});
    };
    // size_type capacity_req = coll::at_least<value_type>(size_type(4))
    auto fn_0 = [fn_tmpl]<typename value_type, typename size_type>(
                  value_type const * t, size_type const * u
                ) -> metatests::test_result
    {
      using size_req = std::integral_constant<size_type, 0u>;
      return fn_tmpl(t, u, size_req{});
    };

    using consteval_test_type_list_1 = metatests::type_list<non_trivial_throwing<128>, non_trivial_throwing_copy<128>>;

    result |= run_constexpr_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_size);
    result |= run_constexpr_test_dual<consteval_test_type_list_1, constexpr_size_type_traits_list>(fn_0);
  };

  return result ? EXIT_SUCCESS : EXIT_FAILURE;
  }

