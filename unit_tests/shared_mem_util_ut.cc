#include <unit_test_core.h>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <interprocess/fork.h>
#include <interprocess/shared_mem_utils.h>
#include <chrono>
#include <iostream>
#include <atomic>
#include <type_traits>
#include <coll/static_vector.h>
#include <cstdint>
#include <interprocess/atomic_mutex.h>
#include <mutex>

namespace ut = boost::ut;
using boost::ut::operator""_test;
using namespace ut::operators::terse;

namespace bip = boost::interprocess;

using clock_type = std::chrono::high_resolution_clock;
using time_point = clock_type::time_point;
using namespace std::chrono_literals;
using namespace std::string_view_literals;

constexpr auto shmem_name {"shmem test"};
int main()
{
"test_basic_shared_mem"_test = [&]
  {
  using sync_counter_decl = ip::shared_type_decl<std::size_t,0u>;
  using child_counter_decl = ip::shared_type_decl<std::size_t,8u>;

  //common shared memory
  bip::shared_memory_object shm{ bip::open_or_create, shmem_name, bip::read_write };
  shm.truncate(4096);
  bip::mapped_region region ( shm, bip::read_write );
  decltype(auto) counter_obj{ ip::construct_ref<sync_counter_decl>(region) };
  
  constexpr size_t number_of_counts { 100000 };
  // child process
  auto child = ip::fork([](std::string_view shared_mem_name )
    {
    bip::shared_memory_object shm_obj{ bip::open_only, shared_mem_name.data() , bip::read_write };
    bip::mapped_region cregion{ shm_obj, bip::read_write };
    decltype(auto) ccounter_obj { ip::ref<sync_counter_decl>(cregion) };
    decltype(auto) child_counter_obj{ ip::construct_ref<child_counter_decl>(cregion) };
    std::atomic_ref ccounter { ccounter_obj };
    auto curr_value = ccounter.load(std::memory_order_acquire);
    while( curr_value < number_of_counts)
      {
      if( (curr_value & 1) == 0)
        ccounter.compare_exchange_strong(curr_value, curr_value+1, std::memory_order_release);
      ++child_counter_obj;
      curr_value = ccounter.load(std::memory_order_acquire);
      }
    ut::expect(child_counter_obj >= number_of_counts);
    return child_counter_obj >= number_of_counts;
    },
    shmem_name );

  ut::expect( static_cast<bool>(child) ) >> ut::fatal;

  // parent process
  std::atomic_ref counter { counter_obj };
  auto beg {clock_type::now()};
  auto curr_value = counter.load(std::memory_order_acquire);
  std::size_t parent_counter_obj {};
  while( curr_value < number_of_counts)
    {
    if( (curr_value & 1) != 0)
      counter.compare_exchange_strong(curr_value, curr_value+1, std::memory_order_release);
    curr_value = counter.load(std::memory_order_acquire);
    ++parent_counter_obj;
    }
  auto end {clock_type::now()};
  auto measurment{ std::chrono::duration_cast<std::chrono::milliseconds>(end-beg)};
  std::cout << "Measured " << measurment.count() << "ms" << std::endl;

  // summary 
  ut::expect(child->join());
  decltype(auto) child_counter_obj{ ip::ref<child_counter_decl>(region) };
  std::atomic_ref child_counter { child_counter_obj };
  ut::expect(counter_obj == number_of_counts);
  ut::expect(child_counter >= number_of_counts);
  ut::expect(parent_counter_obj >= number_of_counts);
  
  std::cout << "counter_obj " << counter_obj << std::endl;
  std::cout << "child_counter " << child_counter << std::endl;
  std::cout << "parent_counter_obj " << parent_counter_obj << std::endl;

  bip::shared_memory_object::remove(shmem_name);
  };
  
"test_complex_objects"_test = [&]
  {
  using vector_type = coll::static_vector<uint32_t,512u>;
  using shared_vector_decl = ip::shared_type_decl<vector_type,0u>;
  bip::shared_memory_object shm{ bip::open_or_create, shmem_name, bip::read_write };
  shm.truncate(4096);
  bip::mapped_region region ( shm, bip::read_write );
  vector_type & vector_obj{ ip::construct_ref<shared_vector_decl>(region) };
  resize(vector_obj,1);
  front(vector_obj) = 2;
  
  auto child = ip::fork([](std::string_view shared_mem_name )
    {
    bip::shared_memory_object shm_obj{ bip::open_only, shared_mem_name.data() , bip::read_write };
    bip::mapped_region cregion{ shm_obj, bip::read_write };
    vector_type & vector { ip::ref<shared_vector_decl>(cregion) };
    ut::expect(size(vector) == 1u );
    ut::expect(front(vector) == 2u );
    ut::expect(resize(vector,512) == coll::vector_outcome_e::no_error ) >> ut::fatal;
    std::iota(begin(vector), end(vector), 2);
    
    return true;
    },
    shmem_name );
  
  ut::expect(static_cast<bool>(child)) >> ut::fatal;
  ut::expect(child->join()) >> ut::fatal;
  ut::expect(size(vector_obj) == 512u );
  ut::expect(front(vector_obj) == 2 );
  ut::expect(back(vector_obj) == 513 );
  };
  
"test_atomic_mutex"_test = [&]
  {
  using semaphore_decl = ip::shared_type_decl<std::atomic<std::size_t>,0u>;
  using atomic_mutex_decl = ip::shared_type_decl<ip::atomic_mutex,8u>;
  
  bip::shared_memory_object shm{ bip::open_or_create, shmem_name, bip::read_write };
  shm.truncate(4096);
  bip::mapped_region region ( shm, bip::read_write );
  ip::atomic_mutex & sync_obj{ ip::construct_ref<atomic_mutex_decl>(region) };
  std::atomic<std::size_t> & counter_obj{ ip::construct_ref<semaphore_decl>(region, 0u) };
  
  auto beg {clock_type::now()};
  auto child = ip::fork([](std::string_view shared_mem_name )
    {
    bip::shared_memory_object shm_obj{ bip::open_only, shared_mem_name.data() , bip::read_write };
    bip::mapped_region cregion{ shm_obj, bip::read_write };
    ip::atomic_mutex & csync_obj { ip::ref<atomic_mutex_decl>(cregion) };
    std::atomic<std::size_t> & ccounter_obj{ ip::construct_ref<semaphore_decl>(cregion, 0u) };
    std::unique_lock lck(csync_obj);
    ccounter_obj.store(1u, std::memory_order_release );
    std::this_thread::sleep_for(1500ms);
    return true;
    },
    shmem_name );
  ut::expect(static_cast<bool>(child)) >> ut::fatal;
  
  auto curr_value = counter_obj.load(std::memory_order_acquire);
  while(curr_value == 0)
    {
    std::this_thread::yield();
    curr_value = counter_obj.load(std::memory_order_acquire);
    }
    
    {
    std::unique_lock lck(sync_obj);
    auto end {clock_type::now()};
    auto measurment{ std::chrono::duration_cast<std::chrono::milliseconds>(end-beg)};
    ut::expect(measurment >= 1500ms );
    std::cout << "atomix mutex measurment " << measurment << std::endl;
    }
  ut::expect(child->join()) >> ut::fatal;
  };
  
}

