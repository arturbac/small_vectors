#pragma once

#include <small_vectors/concepts/concepts.h>
#include <cstdint>
#include <limits>
#include <cassert>
#include <bit>

namespace small_vectors::inline v3_0
  {
template<typename T, concepts::trivial V>
//     requires( alignof(T) > 1 ) //disabled due to not being able use declaration for recurent types
struct composed_pointer_with_data
  {
  using pointer = T *;
  using data_value_type = V;

  static constexpr uintptr_t data_bits = std::bit_width<alignof(T)>;
  static constexpr uintptr_t data_mask = alignof(T) - 1;
  static constexpr uintptr_t pointer_mask = std::numeric_limits<uintptr_t>::max() ^ data_mask;

  std::uintptr_t data_;

  inline constexpr composed_pointer_with_data() noexcept
    requires(alignof(T) > 1)
  = default;

  inline explicit composed_pointer_with_data(pointer ptr) noexcept
    requires(alignof(T) > 1)
      : data_{reinterpret_cast<uintptr_t>(ptr)}  // lower bits are empty
    {
    }

  inline explicit composed_pointer_with_data(pointer ptr, data_value_type value)
    requires(alignof(T) > 1)
      : data_{reinterpret_cast<uintptr_t>(ptr) | static_cast<uintptr_t>(value)}  // lower bits are empty
    {
    }

  inline constexpr void validate(data_value_type value)
    {
    if(std::is_constant_evaluated())
      {
      if(static_cast<uintptr_t>(value) > data_mask)
        throw;
      }
    else
      assert(static_cast<uintptr_t>(value) <= data_mask);
    }

  inline pointer ptr() const noexcept { return reinterpret_cast<pointer>(data_ & pointer_mask); }

  inline void set_ptr(pointer ptr) noexcept { data_ = reinterpret_cast<uintptr_t>(ptr) | (data_ & data_mask); }

  inline pointer operator->() const noexcept { return ptr(); }

  inline constexpr explicit operator bool() const noexcept { return (data_ & pointer_mask) != 0; }

  inline constexpr data_value_type data() const noexcept { return static_cast<data_value_type>(data_ & data_mask); }

  inline constexpr void set_data(data_value_type value)
    {
    validate(value);
    data_ = (data_ & pointer_mask) | static_cast<uintptr_t>(value);
    }
  };
  }  // namespace small_vectors::inline v3_0
