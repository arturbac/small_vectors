#pragma once

#include <atomic>
#include <thread>

namespace ip
  {
/* NOTE
memory_order_seq_cst - is used intentionaly, as read operations after store within mutex have to observe results by all
threads

A load operation with this memory order performs an acquire operation, a store performs a release operation, and
read-modify-write performs both an acquire operation and a release operation, plus a single total order exists in which
all threads observe all modifications in the same order
*/

struct atomic_mutex
  {
  std::atomic<bool> state_;

  explicit operator bool() const noexcept { return state_.load(std::memory_order_acquire); }

  bool is_locked() const noexcept { return state_.load(std::memory_order_acquire); }

  atomic_mutex() : state_{} {}

  atomic_mutex(atomic_mutex const &) = delete;
  atomic_mutex & operator=(atomic_mutex const &) = delete;

  ///\brief tries to lock
  ///\param sleep_delay delay between retries
  ///\param tries number of tries
  template<typename Rep = int64_t, typename Period = std::milli>
  bool try_lock(std::chrono::duration<Rep, Period> const & sleep_delay = 0, std::uint32_t tries = 1) noexcept
    {
    bool ret{};
    for(std::uint32_t i = 0; ret == false && i < tries; i++)
      {
      bool expected{};
      if(i != 0)  // do not delay 1st try
        std::this_thread::sleep_for(sleep_delay);
      ret = state_.compare_exchange_strong(expected, true, std::memory_order_seq_cst);
      }
    return ret;
    }

  ///\brief locks mutex
  void lock() noexcept
    {
    do
      {
      bool expected{};
      if(state_.compare_exchange_strong(expected, true, std::memory_order_seq_cst))
        break;
      std::this_thread::yield();
      } while(true);
    }

  void unlock() noexcept
    {
    bool expected{true};
    state_.compare_exchange_strong(expected, false, std::memory_order_seq_cst);
    }
  };
  }  // namespace ip
