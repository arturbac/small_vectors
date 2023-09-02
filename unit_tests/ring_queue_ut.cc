#include <unit_test_core.h>
#include <interprocess/fork.h>
#include <interprocess/shared_mem_utils.h>
#include <interprocess/ring_queue.h>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <array>

namespace ut = boost::ut;
using boost::ut::operator""_test;
using namespace ut::operators::terse;
using metatests::constexpr_test;

namespace bip = boost::interprocess;
using namespace std::string_view_literals;

// constexpr auto shmem_name {"ring buffer shmem test"};
// struct message 
// {
//   std::array<char,4> value;
// };
// constexpr std::array messages
//   {
//   "Lorem ipsum dolor"sv, // 17
//   "sit amet,"sv,
//   "consectetur adipiscing"sv,
//   "elit, sed do eiusmo"sv,
//   "tempor incididunt ut"sv,
//   "labore et dolore"sv,
//   "magna aliqua."sv,
//   "Ut enim ad minim veniam,"sv,
//   "quis nostrud exercitation"sv
//   };
int main()
{
metatests::test_result result;
"ring_queue_impl_basic"_test = [&]
  {
  
  auto fn_test = [] -> metatests::test_result
    {
    using queue_type = ip::detail::ring_queue_impl_t<16, ip::detail::constexpr_index>;
    queue_type queue;
    constexpr_test(queue.empty());
      {
      std::array<uint8_t,4> value{ 'a','b','c','d'};
      queue.push( value.begin(), value.end() );
      }
    constexpr_test(!queue.empty());
      {
      std::array<uint8_t,11> value{ 'a','b','c','d','e','f','g','h','i','j','k'};
      queue.push( value.begin(), value.end() );
      }
    return {};
    };
  result |= metatests::run_constexpr_test(fn_test);
  result |= metatests::run_consteval_test(fn_test);
  };
}

