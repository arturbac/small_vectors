#include <small_vectors/detail/uninitialized_constexpr.h>
#include <unit_test_core.h>

#include <atomic>
#include <iostream>
#include <algorithm>
#include <functional>
#include <ranges>
#include <array>
#include <string>
#include <small_vectors/relocatable/std/vector.h>

using traits_list_move
  = metatests::type_list<uint16_t, int32_t, int64_t, double, non_trivial, non_trivial_ptr, non_trivial_ptr_except>;

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

using namespace boost::ext::ut;

static int * instance_counter{};

struct explicit_relocatable_t
  {
  int i_;

  explicit explicit_relocatable_t(int i = 0) noexcept : i_(i) { ++(*instance_counter); }

  explicit_relocatable_t(explicit_relocatable_t && rhs) noexcept : i_(rhs.i_)
    {
    ++(*instance_counter);
    i_ = rhs.i_;
    }

  explicit_relocatable_t(explicit_relocatable_t const & rhs) noexcept : i_(rhs.i_)
    {
    ++(*instance_counter);
    i_ = rhs.i_;
    }

  explicit_relocatable_t & operator=(explicit_relocatable_t const &) noexcept = default;

  // S & operator=(S &&) noexcept;

  ~explicit_relocatable_t() { --(*instance_counter); }
  };

inline bool operator==(explicit_relocatable_t const & lh, explicit_relocatable_t const & rh) noexcept
  {
  return lh.i_ == rh.i_;
  }

consteval bool adl_decl_trivially_destructible_after_move(explicit_relocatable_t const *) { return true; }

struct implicit_relocatable_t
  {
  int i_;

  explicit implicit_relocatable_t(int i) noexcept : i_(i) { ++(*instance_counter); }

  implicit_relocatable_t(implicit_relocatable_t && rhs) noexcept : i_(rhs.i_)
    {
    ++(*instance_counter);
    i_ = rhs.i_;
    }

  implicit_relocatable_t(implicit_relocatable_t const & rhs) noexcept : i_(rhs.i_)
    {
    ++(*instance_counter);
    i_ = rhs.i_;
    }

  implicit_relocatable_t & operator=(implicit_relocatable_t const &) noexcept = default;

  // S & operator=(S &&) noexcept;

  ~implicit_relocatable_t() = default;
  };

inline bool operator==(implicit_relocatable_t const & lh, implicit_relocatable_t const & rh) noexcept
  {
  return lh.i_ == rh.i_;
  }

struct non_relocatable_t
  {
  int i_;

  explicit non_relocatable_t(int i) noexcept : i_(i) { ++(*instance_counter); }

  non_relocatable_t(non_relocatable_t && rhs) noexcept : i_(rhs.i_)
    {
    ++(*instance_counter);
    i_ = rhs.i_;
    }

  non_relocatable_t(non_relocatable_t const & rhs) noexcept : i_(rhs.i_)
    {
    ++(*instance_counter);
    i_ = rhs.i_;
    }

  non_relocatable_t & operator=(non_relocatable_t const &) noexcept = default;

  // S & operator=(S &&) noexcept;

  ~non_relocatable_t() { --(*instance_counter); }
  };

inline bool operator==(non_relocatable_t const & lh, non_relocatable_t const & rh) noexcept { return lh.i_ == rh.i_; }

struct copy_non_relocatable_t
  {
  int i_;

  explicit copy_non_relocatable_t(int i) : i_(i) { ++(*instance_counter); }

  copy_non_relocatable_t(copy_non_relocatable_t const & rhs) : i_(rhs.i_)
    {
    ++(*instance_counter);
    i_ = rhs.i_;
    }

  copy_non_relocatable_t & operator=(copy_non_relocatable_t const &) = default;

  ~copy_non_relocatable_t() { --(*instance_counter); }
  };

inline bool operator==(copy_non_relocatable_t const & lh, copy_non_relocatable_t const & rh) noexcept
  {
  return lh.i_ == rh.i_;
  }

static_assert(std::is_trivially_destructible_v<implicit_relocatable_t>);
static_assert(!std::is_trivially_destructible_v<explicit_relocatable_t>);
static_assert(!std::is_trivially_destructible_v<non_relocatable_t>);
static_assert(!std::is_trivially_destructible_v<copy_non_relocatable_t>);

static_assert(small_vectors::concepts::trivially_destructible_after_move<explicit_relocatable_t>);
static_assert(small_vectors::concepts::trivially_destructible_after_move<implicit_relocatable_t>);
static_assert(small_vectors::concepts::trivially_destructible_after_move<int>);
static_assert(!small_vectors::concepts::trivially_destructible_after_move<non_relocatable_t>);
static_assert(!small_vectors::concepts::trivially_destructible_after_move<copy_non_relocatable_t>);

int main()
  {
  using namespace metatests;

  "test_uninitialized_move_n"_test = []
  {
    auto constexpr_uninitialized_move_n = []<typename value_type>(value_type const *) -> metatests::test_result
    {
      {
      auto const arr1{construct_vec<value_type, 10>()};
      auto arr2{arr1};
      std::array<value_type *, 10> out;
      small_vectors::detail::uninitialized_move_n(begin(arr2), 10, begin(out));
      constexpr_test(std::ranges::equal(arr1, out));
      destroy_vec(out);
      return {};
      }
    };
    std::ignore = run_constexpr_test<traits_list_move>(constexpr_uninitialized_move_n);
    std::ignore = run_consteval_test<traits_list_move>(constexpr_uninitialized_move_n);
  };
  "test_uninitialized_relocate_n"_test = []
  {
    int ctr;
    instance_counter = &ctr;
    "relocatable"_test = [&]
    {
      using T = explicit_relocatable_t;
      ctr = 0;
      std::allocator<T> alloc;
      std::array<T, 4> source{T{1}, T{2}, T{3}, T{4}};
      T * ptr{alloc.allocate(4)};
      small_vectors::detail::uninitialized_relocate_if_noexcept_n(source.begin(), 4u, ptr);
      expect(ctr == 8) << "expecting destructor was not called 8 times" << ctr;
      small_vectors_clang_unsafe_buffer_usage_begin  //
        expect(std::ranges::equal(std::span<T const>{source}, std::span<T const>{ptr, 4}));
      small_vectors_clang_unsafe_buffer_usage_end  //
        alloc.deallocate(ptr, 4);
    };
    "movable"_test = [&]
    {
      using T = non_relocatable_t;
      ctr = 0;
      std::allocator<T> alloc;
      std::array<T, 4> source{T{1}, T{2}, T{3}, T{4}};
      T * ptr{alloc.allocate(4)};
      small_vectors::detail::uninitialized_relocate_if_noexcept_n(source.begin(), 4u, ptr);
      // range in ptr is not destroyed
      expect(ctr == 4) << "expecting destructor was called 4 times by uninitialized_relocate_n" << ctr;
      small_vectors_clang_unsafe_buffer_usage_begin  //
        expect(std::ranges::equal(std::span{source}, std::span{ptr, 4}));
      small_vectors_clang_unsafe_buffer_usage_end  //
        alloc.deallocate(ptr, 4);
    };
    "copyable"_test = [&]
    {
      using T = copy_non_relocatable_t;
      ctr = 0;
      std::allocator<T> alloc;
      std::array<T, 4> source{T{1}, T{2}, T{3}, T{4}};
      T * ptr{alloc.allocate(4)};
      small_vectors::detail::uninitialized_relocate_if_noexcept_n(source.begin(), 4u, ptr);
      // range in ptr is not destroyed
      expect(ctr == 4) << "expecting destructor was called 4 times by uninitialized_relocate_if_noexcept_n" << ctr;
      small_vectors_clang_unsafe_buffer_usage_begin  //
        expect(std::ranges::equal(std::span{source}, std::span{ptr, 4}));
      small_vectors_clang_unsafe_buffer_usage_end  //
        alloc.deallocate(ptr, 4);
    };
  };
  }

