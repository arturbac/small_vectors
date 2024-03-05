#pragma once

#include <small_vectors/version.h>
#include <type_traits>
#include <concepts>
#include <atomic>
#include <iterator>
#include <ranges>
#include <algorithm>

namespace small_vectors::inline v3_0::ip
  {
namespace detail
  {

  enum struct push_status : uint8_t
    {
    error_no_enough_space,
    succeed
    };
  namespace ranges = std::ranges;

  using atomic_index = std::atomic_size_t;

  struct constexpr_index
    {
    using enum std::memory_order;
    std::size_t value{};

    constexpr std::size_t load(std::memory_order m = seq_cst) const noexcept { return value; }

    constexpr void store(std::size_t v, std::memory_order m = seq_cst) noexcept { value = v; }
    };

  struct head_t
    {
    std::size_t ix, size;
    };

  struct head_data_t
    {
    std::array<uint8_t, sizeof(head_t)> data_;
    };

  template<std::size_t buffer_size, typename IndexType = atomic_index>
  struct ring_queue_impl_t
    {
    using enum std::memory_order;
    using index_type = IndexType;
    using buffer_type = std::array<uint8_t, buffer_size>;

    buffer_type buffer_{};
    index_type read_index_{0u}, write_index_{0u};

    constexpr explicit ring_queue_impl_t() noexcept = default;

    static constexpr auto capacity() noexcept { return buffer_size - 1; }

    constexpr auto empty() const noexcept
      {
      auto const b = read_index_.load(acquire);
      auto const e = write_index_.load(relaxed);
      return b % capacity() == e;
      }

    template<std::forward_iterator iterator>
    constexpr auto push(iterator data_beg, iterator data_end) noexcept -> push_status
      {
      push_status status{push_status::error_no_enough_space};

      // calculate space required
      std::size_t write_ix{write_index_.load(acquire)};
      std::size_t const read_ix{read_index_.load(relaxed)};
      bool wraps_up = read_ix > write_ix;
      std::size_t data_size{static_cast<std::size_t>(ranges::distance(data_beg, data_end))};
      std::size_t block_size{sizeof(head_t) + data_size};

      std::size_t free_space;
      std::size_t next;

      // store only continuous ranges
      if(wraps_up)
        {
        free_space = read_ix - write_ix;
        next = write_ix;
        }
      else
        {
        std::size_t free_space_low = read_ix;
        std::size_t free_space_hi = capacity() - write_ix;
        if(free_space_hi >= block_size)
          {
          free_space = free_space_hi;
          next = write_ix;
          }
        else
          {
          free_space = free_space_low;
          next = 0u;
          }
        }

      // store data
      if(free_space > block_size) [[likely]]
        {
        ranges::copy_n(
          data_beg,
          static_cast<std::ptrdiff_t>(data_size),
          ranges::next(ranges::begin(buffer_), static_cast<std::ptrdiff_t>(next))
        );
        write_ix = (write_ix + data_size) % capacity();
        write_index_.store(write_ix);
        status = push_status::succeed;
        }
      return status;
      }
    };
  }  // namespace detail
  }  // namespace small_vectors::inline v3_0::ip
