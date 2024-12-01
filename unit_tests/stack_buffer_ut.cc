#include <small_vectors/interprocess/fork.h>
#include <small_vectors/interprocess/shared_mem_utils.h>
#include <small_vectors/interprocess/stack_buffer.h>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <array>
#include <memory>
#include <thread>
#include <unit_test_core.h>

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
namespace ip = small_vectors::ip;
namespace ut = boost::ut;
using boost::ut::operator""_test;
using namespace ut::operators::terse;
using metatests::constexpr_test;

namespace bip = boost::interprocess;
using namespace std::string_view_literals;

int main()
  {
  using ip::push_status;
  using enum ip::push_status;
  using ip::pop_status;
  using enum ip::pop_status;
  metatests::test_result result;
  namespace ranges = std::ranges;

  "stack_buffer_basic"_test = [&]
  {
    auto fn_test = []() -> metatests::test_result
    {
      using stack_type = ip::stack_buffer_t<32, ip::constexpr_index>;
      stack_type stack;
      constexpr_test(ip::empty(stack));
        {
        std::array<uint8_t, 4> value{'a', 'b', 'c', 'd'};
        push_status res{ip::push(stack, value.begin(), value.end())};
        constexpr_test(res == succeed);
        auto [range, index] = ip::top(stack);
        constexpr_test(ranges::equal(range, value));
        constexpr_test(!ip::empty(stack));
        pop_status popres = ip::pop(stack, index);
        constexpr_test(popres == succeed_range_valid);
        constexpr_test(ip::empty(stack));
        res = ip::push(stack, value.begin(), value.end());
        constexpr_test(res == succeed);
        std::tie(range, index) = ip::top(stack);
        constexpr_test(ranges::equal(range, value));
        constexpr_test(!ip::empty(stack));
        }

        {
        std::array<uint8_t, 11> value{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k'};
        push_status res{ip::push(stack, value.begin(), value.end())};
        constexpr_test(res == succeed);
        auto [range, index] = ip::top(stack);
        constexpr_test(ranges::equal(range, value));
        pop_status popres = ip::pop(stack, index);
        constexpr_test(popres == succeed_range_valid);
        }
        {
        std::array<uint8_t, 4> value{'a', 'b', 'c', 'd'};
        auto [range, index] = ip::top(stack);
        constexpr_test(ranges::equal(range, value));
        pop_status popres = ip::pop(stack, index);
        constexpr_test(popres == succeed_range_valid);
        constexpr_test(ip::empty(stack));
        }
        {
        auto [range, index] = ip::top(stack);
        constexpr_test(range.empty());
        }
        {
        std::array<uint8_t, 24> value{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k'};
        push_status res{ip::push(stack, value.begin(), value.end())};
        constexpr_test(res == succeed);
        auto [range, index] = ip::top(stack);
        constexpr_test(ranges::equal(range, value));
        constexpr_test(!ip::empty(stack));
        }
        {
        std::array<uint8_t, 4> value{'a', 'b', 'c', 'd'};
        push_status res{ip::push(stack, value.begin(), value.end())};
        constexpr_test(res == error_not_enough_space);
        auto [range, index] = ip::top(stack);
        pop_status popres = ip::pop(stack, index);
        constexpr_test(popres == succeed_range_valid);
        constexpr_test(ip::empty(stack));
        }
        {
        std::array<uint8_t, 4> value{'a', 'b', 'c', 'd'};
        push_status res{ip::push(stack, value.begin(), value.end())};
        constexpr_test(res == succeed);
        auto [range, index] = ip::top(stack);
        res = ip::push(stack, value.begin(), value.end());
        constexpr_test(res == succeed);
        pop_status popres = ip::pop(stack, index);
        constexpr_test(popres == failed_stack_changed);
        }
      return {};
    };
    result |= metatests::run_constexpr_test(fn_test);
    result |= metatests::run_consteval_test(fn_test);
  };

  constexpr auto shmem_name{"shmem test for " SMALL_VECTORS_COMPILER_INFO};
  bip::shared_memory_object shm{bip::open_or_create, shmem_name, bip::read_write};
  shm.truncate(8192);

  "test_semaphore"_test = [&]
  {
    using stack_type = ip::stack_buffer_t<32, ip::atomic_index>;

    using stack_decl = ip::shared_type_decl<stack_type>;
    using semaphore_decl = ip::shared_type_decl<bip::interprocess_semaphore, stack_decl>;
    using semaphore2_decl = ip::shared_type_decl<bip::interprocess_semaphore, semaphore_decl>;
    using writter_done_decl = ip::shared_type_decl<bool, semaphore2_decl>;
    using namespace std::chrono_literals;

    bip::mapped_region region(shm, bip::read_write);

    std::unique_ptr<stack_type, destruction_only> stack_obj{ip::construct_at<stack_decl>(region)};
    std::unique_ptr<bip::interprocess_semaphore, destruction_only> reader_finished_semaphore{
      ip::construct_at<semaphore_decl>(region, 0)
    },
      writter_finished_semaphore{ip::construct_at<semaphore2_decl>(region, 0)};
    auto writter_done{ip::construct_at<writter_done_decl>(region, false)};

    auto writter = ip::fork(
      [](std::string_view shared_mem_name)
      {
        bip::shared_memory_object shm_obj{bip::open_only, shared_mem_name.data(), bip::read_write};
        bip::mapped_region cregion{shm_obj, bip::read_write};
        decltype(auto) stack{ip::ref<stack_decl>(cregion)};
        decltype(auto) writter_finished_semaphore_obj{ip::ref<semaphore2_decl>(cregion)};
        decltype(auto) writter_done_obj{ip::ref<writter_done_decl>(cregion)};
        std::array<uint8_t, 24> value{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k'};
        auto delay{4us};
        for(auto i = 1000; i != 0; --i)
          for(auto j = 1u; j != value.size(); ++j)
            {
            ip::push_status push_res{};
            do
              {
              push_res = ip::push(stack, value.begin(), ranges::next(value.begin(), j));
              if(push_res == push_status::succeed)
                {
                std::cout << "delay w" << delay.count() << std::endl;
                delay = 4us;
                }
              else if(push_res == error_not_enough_space)
                {
                // std::this_thread::sleep_for(2000us);
                std::this_thread::sleep_for(delay);
                delay = delay + 3us;
                // std::cout << "push error_not_enough_space" << std::endl;
                }
              else if(push_res == error_stack_changed)
                std::cout << "push error_stack_changed" << std::endl;
              else if(push_res == logic_error_not_enough_space)
                std::cout << "push logic_error_not_enough_space" << std::endl;
              } while(push_res != succeed);
            }
        std::cout << "writter done" << std::endl;
        writter_done_obj = true;
        writter_finished_semaphore_obj.post();
        return true;
      },
      shmem_name
    );

    auto reader = ip::fork(
      [](std::string_view shared_mem_name)
      {
        bip::shared_memory_object shm_obj{bip::open_only, shared_mem_name.data(), bip::read_write};
        bip::mapped_region cregion{shm_obj, bip::read_write};
        decltype(auto) stack{ip::ref<stack_decl>(cregion)};
        decltype(auto) reader_finished_semaphore_obj{ip::ref<semaphore_decl>(cregion)};
        decltype(auto) writter_done_obj{ip::ref<writter_done_decl>(cregion)};
        std::array<uint8_t, 24> value{};
        std::array<uint8_t, 24> refvalue{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k'};
        int i{};
        auto it_beg{ranges::begin(value)};
        auto delay{3us};
        while(!writter_done_obj || !ip::empty(stack))
          {
          if(ip::empty(stack))
            {
            // std::this_thread::sleep_for(500us);
            std::this_thread::sleep_for(delay);
            delay = delay + 2us;
            // std::cout << "stack empty" << std::endl;
            }
          else
            {
            std::cout << "delay r" << delay.count() << std::endl;
            delay = 3us;
            auto [range, index] = ip::top(stack);
            auto it_end{ranges::copy(range, it_beg).out};
            pop_status popres{ip::pop(stack, index)};
            if(succeed_range_valid == popres)
              {
              small_vectors_clang_unsafe_buffer_usage_begin  //
                std::span indata{it_beg, it_end};
              small_vectors_clang_unsafe_buffer_usage_end  //
                ranges::subrange refsubrange{
                  ranges::begin(refvalue), ranges::next(ranges::begin(refvalue), static_cast<ptrdiff_t>(indata.size()))
                };
              auto match{ranges::equal(indata, refsubrange)};
              constexpr_test(match);
              if(!match)
                {
                std::cout << i << " pop_status " << ranges::size(indata);
                for(auto c: indata)
                  std::cout << c;
                std::cout << std::endl;
                }
              ++i;
              }
            else
              std::cout << "pop_status failed_stack_changed" << std::endl;
            }
          }
        std::cout << "reader_finished" << std::endl;
        // writter_finished_semaphore_obj.wait();
        constexpr_test(writter_done_obj);
        constexpr_test(ip::empty(stack));
        reader_finished_semaphore_obj.post();
        return true;
      },
      shmem_name
    );

    reader_finished_semaphore->wait();
    writter_finished_semaphore->wait();
    ut::expect(writter->join()) >> ut::fatal;
    ut::expect(reader->join()) >> ut::fatal;
    constexpr_test(*writter_done);
    constexpr_test(ip::empty(*stack_obj));
  };

  bip::shared_memory_object::remove(shmem_name);
  }

