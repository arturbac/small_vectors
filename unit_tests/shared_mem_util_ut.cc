#include <unit_test_core.h>
#include <small_vectors/interprocess/fork.h>
#include <small_vectors/interprocess/shared_mem_utils.h>
#include <small_vectors/interprocess/atomic_mutex.h>
#include <small_vectors/static_vector.h>
#include <small_vectors/basic_string.h>

#include <chrono>
#include <iostream>
#include <atomic>
#include <type_traits>
#include <cstdint>
#include <mutex>
#include <memory>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

namespace ut = boost::ut;
using boost::ut::operator""_test;
using namespace ut::operators::terse;
using enum std::memory_order;
namespace ip = small_vectors::ip;
namespace bip = boost::interprocess;

using clock_type = std::chrono::high_resolution_clock;
using time_point = clock_type::time_point;
using namespace std::chrono_literals;
using namespace std::string_view_literals;

static_assert(ip::detail::concept_aligned_offset<int, 2> == false);
static_assert(ip::detail::concept_aligned_offset<int, 0> == true);
static_assert(ip::detail::concept_aligned_offset<int, 4> == true);
static_assert(ip::detail::concept_aligned_offset<int, 5> == false);

struct destruction_only
  {
  template<typename T>
#if defined(__cpp_static_call_operator)
  static
#endif
    void
    operator()(T * addr)
#if !defined(__cpp_static_call_operator)
      const
#endif
    noexcept
    {
    addr->~T();
    }
  };

constexpr auto shmem_name{"shmem test for " SMALL_VECTORS_COMPILER_INFO};

int main()
  {
  bip::shared_memory_object shm{bip::open_or_create, shmem_name, bip::read_write};
  shm.truncate(8192);

#if defined(__cpp_lib_atomic_ref)
  "test_basic_shared_mem"_test = [&]
  {
    using sync_counter_decl = ip::shared_type_decl<std::size_t>;
    using child_counter_decl = ip::shared_type_decl<std::size_t, sync_counter_decl>;

    // common shared memory
    bip::mapped_region region(shm, bip::read_write);
    decltype(auto) counter_obj{*ip::construct_at<sync_counter_decl>(region)};

    constexpr size_t number_of_counts{100000};
    // child process
    auto child = ip::fork(
      [](std::string_view shared_mem_name)
      {
        bip::shared_memory_object shm_obj{bip::open_only, shared_mem_name.data(), bip::read_write};
        bip::mapped_region cregion{shm_obj, bip::read_write};
        decltype(auto) ccounter_obj{ip::ref<sync_counter_decl>(cregion)};
        decltype(auto) child_counter_obj{*ip::construct_at<child_counter_decl>(cregion)};
        std::atomic_ref ccounter{ccounter_obj};
        auto curr_value = ccounter.load(acquire);
        while(curr_value < number_of_counts)
          {
          if((curr_value & 1) == 0)
            ccounter.compare_exchange_strong(curr_value, curr_value + 1, release);
          else
            std::this_thread::yield();
          ++child_counter_obj;
          curr_value = ccounter.load(acquire);
          }
        ut::expect(child_counter_obj >= number_of_counts);
        return child_counter_obj >= number_of_counts;
      },
      shmem_name
    );

    ut::expect(static_cast<bool>(child)) >> ut::fatal;

    // parent process
    std::atomic_ref counter{counter_obj};
    auto beg{clock_type::now()};
    auto curr_value = counter.load(acquire);
    std::size_t parent_counter_obj{};
    while(curr_value < number_of_counts)
      {
      if((curr_value & 1) != 0)
        counter.compare_exchange_strong(curr_value, curr_value + 1, release);
      else
        std::this_thread::yield();
      curr_value = counter.load(acquire);
      ++parent_counter_obj;
      }
    auto end{clock_type::now()};
    auto measurment{std::chrono::duration_cast<std::chrono::milliseconds>(end - beg)};
    std::cout << "Measured " << measurment.count() << "ms" << std::endl;

    // summary
    ut::expect(child->join());
    decltype(auto) child_counter_obj{ip::ref<child_counter_decl>(region)};
    std::atomic_ref child_counter{child_counter_obj};
    ut::expect(counter_obj == number_of_counts);
    ut::expect(child_counter >= number_of_counts);
    ut::expect(parent_counter_obj >= number_of_counts);

    std::cout << "counter_obj " << counter_obj << std::endl;
    std::cout << "child_counter " << child_counter << std::endl;
    std::cout << "parent_counter_obj " << parent_counter_obj << std::endl;
  };
#endif

  "test_complex_objects"_test = [&]
  {
    using vector_type = small_vectors::static_vector<uint32_t, 512u>;
    using shared_vector_decl = ip::shared_type_decl<vector_type>;
    bip::mapped_region region(shm, bip::read_write);
    vector_type & vector_obj{*ip::construct_at<shared_vector_decl>(region)};
    resize(vector_obj, 1);
    front(vector_obj) = 2;

    auto child = ip::fork(
      [](std::string_view shared_mem_name)
      {
        bip::shared_memory_object shm_obj{bip::open_only, shared_mem_name.data(), bip::read_write};
        bip::mapped_region cregion{shm_obj, bip::read_write};

        vector_type & vector{ip::ref<shared_vector_decl>(cregion)};
        ut::expect(size(vector) == 1u);
        ut::expect(front(vector) == 2u);
        ut::expect(resize(vector, 512) == small_vectors::vector_outcome_e::no_error) >> ut::fatal;
        pop_back(vector);
        std::iota(begin(vector), end(vector), 2);

        return true;
      },
      shmem_name
    );

    ut::expect(static_cast<bool>(child)) >> ut::fatal;
    ut::expect(child->join()) >> ut::fatal;
    ut::expect(size(vector_obj) == 511u);
    ut::expect(front(vector_obj) == 2);
    ut::expect(back(vector_obj) == 512);
  };

  "test_atomic_mutex"_test = [&]
  {
    //
    using semaphore_decl = ip::shared_type_decl<std::atomic<std::size_t>>;
    using atomic_mutex_decl = ip::shared_type_decl<ip::atomic_mutex, semaphore_decl>;
    //

    bip::mapped_region region(shm, bip::read_write);
    ip::atomic_mutex & sync_obj{*ip::construct_at<atomic_mutex_decl>(region)};
    std::atomic<std::size_t> & shemaphore{*ip::construct_at<semaphore_decl>(region, 0u)};

    auto beg{clock_type::now()};
    auto child = ip::fork(
      [](std::string_view shared_mem_name)
      {
        bip::shared_memory_object shm_obj{bip::open_only, shared_mem_name.data(), bip::read_write};
        bip::mapped_region cregion{shm_obj, bip::read_write};

        ip::atomic_mutex & csync_obj{ip::ref<atomic_mutex_decl>(cregion)};
        std::atomic<std::size_t> & cshemaphore{ip::ref<semaphore_decl>(cregion)};

        std::unique_lock lck(csync_obj);
        cshemaphore.store(1u, release);
        std::this_thread::sleep_for(1500ms);
        return true;
      },
      shmem_name
    );
    ut::expect(static_cast<bool>(child)) >> ut::fatal;

    auto curr_value = shemaphore.load(acquire);
    while(curr_value == 0)
      {
      std::this_thread::yield();
      curr_value = shemaphore.load(acquire);
      }

      {
      std::unique_lock lck(sync_obj);
      auto end{clock_type::now()};
      auto measurment{std::chrono::duration_cast<std::chrono::milliseconds>(end - beg)};
      ut::expect(measurment >= 1500ms);
      std::cout << "atomic mutex measurment " << measurment.count() << std::endl;
      }
    ut::expect(child->join()) >> ut::fatal;
  };

  "test_multiple_data"_test = [&]
  {
    // used types between processes
    struct foo
      {
      int a, a_;
      double b;
      int64_t c;
      };

    using message = small_vectors::static_u8string<512>;
    using vector_type = small_vectors::static_vector<uint32_t, 128u>;

    // memory offset table
    using foo_obj_decl = ip::shared_type_decl<foo>;
    using shared_vector_decl = ip::shared_type_decl<vector_type, foo_obj_decl>;
    using ref_obj_decl = ip::shared_type_decl<int, shared_vector_decl>;
    using message_decl = ip::shared_type_decl<message, ref_obj_decl>;

    bip::mapped_region region(shm, bip::read_write);

    // construct objects in main process
    foo & foo_obj{*ip::construct_at<foo_obj_decl>(region, foo{.a = 1, .a_ = 0, .b = 0.5, .c = 0xffffffff})};
    auto & ref_obj{*ip::construct_at<ref_obj_decl>(region, 2u)};
    auto & ref_string{*ip::construct_at<message_decl>(region, u8"message hello world"sv)};
    vector_type & vector_obj{*ip::construct_at<shared_vector_decl>(region)};
    resize(vector_obj, 1);
    front(vector_obj) = 2;

    // alter data at forked process
    auto child = ip::fork(
      [](std::string_view shared_mem_name)
      {
        bip::shared_memory_object shm_obj{bip::open_only, shared_mem_name.data(), bip::read_write};
        bip::mapped_region cregion{shm_obj, bip::read_write};

        // reference shared objects
        foo & cfoo_obj{ip::ref<foo_obj_decl>(cregion)};
        vector_type & vector{ip::ref<shared_vector_decl>(cregion)};
        auto & cref_string{ip::ref<message_decl>(cregion)};
        auto & cref_obj{ip::ref<ref_obj_decl>(cregion)};

        // read write data
        ut::expect(cfoo_obj.a == 1);
        ut::expect(cfoo_obj.b == 0.5);
        ut::expect(cfoo_obj.c == 0xffffffff);
        cfoo_obj.a = 2;
        cfoo_obj.b = 1.5;
        cfoo_obj.c = -0x1ffffffff;

        ut::expect(size(vector) == 1u);
        ut::expect(front(vector) == 2u);
        ut::expect(resize(vector, 128) == small_vectors::vector_outcome_e::no_error) >> ut::fatal;
        pop_back(vector);
        std::iota(begin(vector), end(vector), 2);

        ut::expect(cref_string.view() == u8"message hello world"sv);
        cref_string = u8"hello world from child"sv;
        cref_obj += 2;

        return true;
      },
      shmem_name
    );

    // check modified data at forked process
    ut::expect(child->join()) >> ut::fatal;
    ut::expect(foo_obj.a == 2);
    ut::expect(foo_obj.b == 1.5);
    ut::expect(foo_obj.c == -0x1ffffffff);

    ut::expect(ref_string.view() == u8"hello world from child"sv);

    ut::expect(ref_obj == 4);

    ut::expect(size(vector_obj) == 127u);
    ut::expect(front(vector_obj) == 2);
    ut::expect(back(vector_obj) == 128);
  };

  "test_semaphore"_test = [&]
  {
    using semaphore_decl = ip::shared_type_decl<bip::interprocess_semaphore>;
    using semaphore2_decl = ip::shared_type_decl<bip::interprocess_semaphore, semaphore_decl>;
    using integer_decl = ip::shared_type_decl<std::int64_t, semaphore2_decl>;

    bip::mapped_region region(shm, bip::read_write);
    std::unique_ptr<bip::interprocess_semaphore, destruction_only> semaphore_obj{
      ip::construct_at<semaphore_decl>(region, 0)
    },
      semaphore2_obj{ip::construct_at<semaphore2_decl>(region, 0)};

    decltype(auto) integer_obj{*ip::construct_at<integer_decl>(region, 0xf1)};

    auto child = ip::fork(
      [](std::string_view shared_mem_name)
      {
        bip::shared_memory_object shm_obj{bip::open_only, shared_mem_name.data(), bip::read_write};
        bip::mapped_region cregion{shm_obj, bip::read_write};

        decltype(auto) child_semaphore_obj{ip::ref<semaphore_decl>(cregion)};
        decltype(auto) child_semaphore2_obj{ip::ref<semaphore2_decl>(cregion)};
        decltype(auto) child_integer_obj{ip::ref<integer_decl>(cregion)};
        child_semaphore2_obj.post();
        child_semaphore_obj.wait();
        ut::expect(child_integer_obj == 2);
        return true;
      },
      shmem_name
    );
    auto child2 = ip::fork(
      [](std::string_view shared_mem_name)
      {
        bip::shared_memory_object shm_obj{bip::open_only, shared_mem_name.data(), bip::read_write};
        bip::mapped_region cregion{shm_obj, bip::read_write};

        decltype(auto) child_semaphore_obj{ip::ref<semaphore_decl>(cregion)};
        decltype(auto) child_semaphore2_obj{ip::ref<semaphore2_decl>(cregion)};
        decltype(auto) child_integer_obj{ip::ref<integer_decl>(cregion)};
        child_semaphore2_obj.wait();
        child_integer_obj = 2;
        child_semaphore_obj.post();

        return true;
      },
      shmem_name
    );

    ut::expect(child->join()) >> ut::fatal;
    ut::expect(child2->join()) >> ut::fatal;
    ut::expect(integer_obj == 2);
  };

  bip::shared_memory_object::remove(shmem_name);
  }

