#pragma once
#include <small_vectors/concepts/concepts.h>
#include <small_vectors/utils/utility_cxx20.h>
#include <utility>
#include <array>

namespace small_vectors::inline v3_3::detail
  {

template<typename value_type, std::unsigned_integral size_type>
struct storage_context_t;

template<typename value_type, std::unsigned_integral size_type>
using dynamic_storage_t = storage_context_t<value_type, size_type>;
template<typename value_type, std::unsigned_integral size_type>
storage_context_t(value_type *, size_type) -> storage_context_t<value_type, size_type>;

template<typename value_type, typename size_type>
inline constexpr void sv_deallocate(storage_context_t<value_type, size_type> storage) noexcept;

template<concepts::allocate_constraint value_type, typename size_type>
inline constexpr storage_context_t<value_type, size_type> sv_allocate(size_type count) noexcept;

template<typename size_type>
constexpr auto growth(size_type old_size, size_type new_elements) noexcept -> size_type;

consteval auto storage_size(std::size_t max_value) noexcept -> std::size_t
  {
  if(max_value <= 0xffu)
    return 1;
  else if(max_value <= 0xffffu)
    return 2;
  else if(max_value <= 0xffff'ffffu)
    return 4;
  else
    return 8;
  }

template<std::size_t bytes>
struct size_type_selector_t
  {
  };

template<>
struct size_type_selector_t<1>
  {
  using size_type = uint8_t;
  };

template<>
struct size_type_selector_t<2>
  {
  using size_type = uint16_t;
  };

template<>
struct size_type_selector_t<4>
  {
  using size_type = uint32_t;
  };

template<>
struct size_type_selector_t<8>
  {
  using size_type = uint64_t;
  };

template<std::size_t elements>
using size_type_select_t = typename size_type_selector_t<storage_size(elements)>::size_type;

template<concepts::vector_constraints value_type, uint64_t capacity>
  requires(capacity > 0u)
struct aligned_storage_for_no_trivial
  {
#if 1
  // aligned_storage become deprecated propably
  // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1413r2.pdf
  alignas(value_type) std::byte data_[sizeof(value_type) * capacity];

  [[nodiscard]]
  constexpr auto data() noexcept -> value_type *
    {
    return cxx20::assume_aligned<alignof(value_type)>(std::launder(reinterpret_cast<value_type *>(&data_)));
    }

  [[nodiscard]]
  constexpr auto data() const noexcept -> value_type const *
    {
    return cxx20::assume_aligned<alignof(value_type)>(std::launder(reinterpret_cast<value_type const *>(&data_)));
    }
#else
    // union workaround for constexpr nontrivial types doesn't work with arrays inside union with clang while with gcc
    // does but gcc is wrong
    union _storage {
    constexpr _storage() : _init{} {}

    constexpr ~_storage() {}

    value_type object[capacity];
    char _init = '\0';
    } u;

  [[nodiscard]]
  constexpr value_type * data() noexcept
    {
    return &(u.object[0]);
    }

  [[nodiscard]]
  constexpr value_type const * data() const noexcept
    {
    return &(u.object[0]);
    }
#endif
  };

namespace detail_concepts
  {
  template<typename T>
  concept vector_storage = requires(T & obj) {
    typename T::value_type;
    typename T::size_type;
    typename T::storage_type;
    typename T::storage_context_type;
      // clang-format off
    { obj.size_ } -> std::same_as<typename T::size_type &>;
    { obj.capacity() } -> std::same_as<typename T::size_type>;
    { T::supports_reallocation } -> std::same_as<bool const &>;
    { obj.data() } -> std::same_as<typename T::value_type *>;
    { obj.context() } -> std::same_as<typename T::storage_context_type>;
              // clang-format on
  };
  }  // namespace detail_concepts

template<concepts::vector_constraints V, uint64_t N>
struct static_vector_storage
  {
  using value_type = V;
  using size_type = size_type_select_t<N>;
  using storage_context_type = storage_context_t<value_type, size_type>;

  static constexpr auto capacity() noexcept -> size_type { return N; }

  static constexpr size_type buffered_capacity = N;
  static constexpr bool value_type_has_trivial_liftime
    = std::is_trivially_default_constructible_v<value_type> && std::is_trivially_destructible_v<value_type>;

  static constexpr bool supports_reallocation{false};

  using storage_type = std::conditional_t<
    value_type_has_trivial_liftime,
    std::array<value_type, buffered_capacity>,
    aligned_storage_for_no_trivial<value_type, buffered_capacity>>;

  // If the member is not empty, any tail padding in it may be also reused to store other data members.
#if !defined(WIN32)
  [[no_unique_address]]
#endif
  storage_type data_;
#if !defined(WIN32)
  [[no_unique_address]]
#endif
  size_type size_;

  constexpr static_vector_storage() noexcept : size_{} {}

  [[nodiscard]]
  constexpr auto data() const noexcept -> value_type const *
    {
    return data_.data();
    }

  [[nodiscard]]
  constexpr auto data() noexcept -> value_type *
    {
    return data_.data();
    }

  [[nodiscard]]
  inline constexpr auto context() noexcept -> storage_context_type
    {
    return storage_context_type{data_.data(), buffered_capacity};
    }

  inline constexpr void
    construct_move(static_vector_storage && rh) noexcept(std::is_nothrow_move_constructible_v<value_type>)
    requires std::move_constructible<value_type>
    {
    constexpr bool use_nothrow
      = concepts::is_trivially_relocatable<value_type> or std::is_nothrow_move_constructible_v<value_type>;
    if constexpr(use_nothrow)
      uninitialized_relocate_n(rh.data(), rh.size_, data());
    else
      uninitialized_relocate_with_copy_n(rh.data(), rh.size_, data());

    size_ = std::exchange(rh.size_, 0);
    }

  inline constexpr void assign_move(static_vector_storage && rh) noexcept(std::is_nothrow_move_assignable_v<value_type>)
    requires std::movable<value_type>
    {
    constexpr bool use_nothrow
      = concepts::is_trivially_relocatable<value_type> or std::is_nothrow_move_assignable_v<value_type>;

    if constexpr(not std::is_trivially_destructible_v<value_type>)
      {
      size_type const old_size = size_;
      if(old_size != 0u)
        destroy_range(data(), size_type{0}, old_size);
      }

    if constexpr(use_nothrow)
      uninitialized_relocate_n(rh.data(), rh.size_, data());
    else
      uninitialized_relocate_with_copy_n(rh.data(), rh.size_, data());

    size_ = std::exchange(rh.size_, 0);
    }

  template<uint64_t M>
  constexpr void construct_copy(static_vector_storage<value_type, M> const & rh)
    requires(M <= buffered_capacity && std::copy_constructible<value_type>)
    {
    size_type my_size = rh.size_;
    uninitialized_copy_n(rh.data(), my_size, data());
    size_ = my_size;
    }

  template<uint64_t M>
  constexpr void assign_copy(static_vector_storage<V, M> const & rh)
    requires(M <= buffered_capacity && std::copy_constructible<value_type>)
    {
    if constexpr(!std::is_trivially_destructible_v<value_type>)
      destroy_range(data(), size_type(0u), size_);

    size_ = 0u;
    size_type my_size = rh.size_;
    uninitialized_copy_n(rh.data(), my_size, data());
    size_ = my_size;
    }

  constexpr void swap(static_vector_storage & rh) noexcept(std::is_nothrow_copy_assignable_v<value_type>)
    requires std::copyable<value_type>
    {
    uninitialized_uneven_range_swap(data(), size_, rh.data(), rh.size_);
    std::swap(size_, rh.size_);
    }
  };

// union
//   V[N+M] //m from padding to pointer size + size_type and padding of struct
//   { dynamic pointer, size_type capacity }
// type : byte,
// size : size_type
///\returns aligned size of entire storage struct for small_vector
template<std::unsigned_integral size_type>
consteval auto struct_min_byte_size() noexcept -> size_type
  {
  auto size_req = sizeof(void *) + 3 * sizeof(size_type);
  auto align_size = std::max(alignof(void *), alignof(size_type));
  return static_cast<size_type>((size_req / align_size) * align_size + (size_req % align_size != 0u ? align_size : 0));
  }

///\brief counts minimal number of elements taking into account union alignment,
///       size_type for storing size and capacity at dynamic
///       so there is minimal number of elements that always will be available due to entire struct packing
///       see explanation bellow
///\details putting size as last element we align it to pointer boudnary thus
///          allowing efficient union padding reuse for active tag and maximaly extend values in buffer
///         example struct packings
///         space occupied by s = size_type, v = value_type, p = pointer do dynamic data, a - tag of active storage
///         S == sizeof(size_type), V = sizeof(value_type),
///         between| space of union |, and : alignment boundary, _ empty unsued pading
///
///         S=1 V=1, sizeof(small_vector) = 16, union_min_number_of_elements = 14
///           :01234567:01234 567:01234567:01234567:
///           |vvvvvvvv:vvvvvv|  :
///           |pppppppp:s_____|as:
///
///         S=2 V=1, sizeof(small_vector) = 16, union_min_number_of_elements = 13
///           :01234567:01234 567:01234567:01234567:
///           |vvvvvvvv:vvvvv|   :
///           |pppppppp:ss___|ass:
///
///         S=4 V=1, sizeof(small_vector) = 24, union_min_number_of_elements = 19
///           :01234567:01234567:012 34567:01234567:
///           |vvvvvvvv:vvvvvvvv:vvv|     :
///           |pppppppp:ssss____:___|assss:
///
///         S=8 V=1, sizeof(small_vector) = 32, union_min_number_of_elements = 23
///           :01234567:01234567:0123456 7:01234567:
///           |vvvvvvvv:vvvvvvvv:vvvvvvv| :        :
///           |pppppppp:ssssssss:_______|a:ssssssss:
///
///         S=1 V=2, sizeof(small_vector) = 16, union_min_number_of_elements = 7
///           :01234567:012345 67:01234567:01234567:
///           |vvvvvvvv:vvvvvv|  :
///           |pppppppp:s_____|as:
///
///         S=2 V=2, sizeof(small_vector) = 16, union_min_number_of_elements = 6
///           :01234567:0123 4567:01234567:01234567:
///           |vvvvvvvv:vvvv|    :
///           |pppppppp:ss__|_ass: //designed
///           |pppppppp:ss__|a_ss: //real
///
///         S=4 V=2, sizeof(small_vector) = 24, union_min_number_of_elements = 9
///           :01234567:01234567:01 234567:01234567:
///           |vvvvvvvv:vvvvvvvv:vv|      :
///           |pppppppp:ssss____:__|_assss: //designed
///           |pppppppp:ssss____:__|a_ssss: //real
///
///         S=8 V=2, sizeof(small_vector) = 32, union_min_number_of_elements = 11
///           :01234567:01234567:012345 67:01234567:
///           |vvvvvvvv:vvvvvvvv:vvvvvv|  :        :
///           |pppppppp:ssssssss:______|_a:ssssssss: // designed
///           |pppppppp:ssssssss:______|a_:ssssssss: // real
///
///         S=1 V=4, sizeof(small_vector) = 16, union_min_number_of_elements = 3
///           :01234567:0123 4567:01234567:01234567:
///           |vvvvvvvv:vvvv|    :
///           |pppppppp:s___|__as:
///      or   |pppppppp:s___|as__:
///
///         S=2 V=4, sizeof(small_vector) = 16, union_min_number_of_elements = 3
///           :01234567:0123 4567:01234567:01234567:
///           |vvvvvvvv:vvvv|    :
///           |pppppppp:ss__|_ass: //designed
///           |pppppppp:ss__|a_ss: //real
///
///         S=4 V=4, sizeof(small_vector) = 24, union_min_number_of_elements = 4
///           :01234567:01234567:01234567:01234567:
///           |vvvvvvvv:vvvvvvvv|        :
///           |pppppppp:ssss____|___assss:  //designed
///           |pppppppp:ssss____|a___ssss:  //real
///
///         S=8 V=4, sizeof(small_vector) = 32, union_min_number_of_elements = 5
///           :01234567:01234567:0123 4567:01234567:
///           |vvvvvvvv:vvvvvvvv:vvvv|    :        :
///           |pppppppp:ssssssss:____|___a:ssssssss: //designed
///           |pppppppp:ssssssss:____|a___:ssssssss: //real
///
///         S=1 V=8, sizeof(small_vector) = 16, union_min_number_of_elements = 1
///           :01234567:0 1234567:01234567:01234567:
///           |vvvvvvvv: |       :
///           |pppppppp:s|_____as:  //designed
///           |pppppppp:s|as_____:  //real
///
///         S=2 V=8, sizeof(small_vector) = 16, union_min_number_of_elements = 1
///           :01234567:01 234567:01234567:01234567:
///           |vvvvvvvv   |      :
///           |pppppppp:ss|___ass:  //designed
///           |pppppppp:ss|a_ss__:  //real
///
///         S=4 V=8, sizeof(small_vector) = 24, union_min_number_of_elements = 2
///           :01234567:01234567:01234567:01234567:
///           |vvvvvvvv:vvvvvvvv|        :
///           |pppppppp:ssss____|___assss: //designed
///           |pppppppp:ssss____|a___ssss: //real
///
///         S=8 V=8, sizeof(small_vector) = 32, union_min_number_of_elements = 2
///           :01234567:01234567:01234567:01234567:
///           |vvvvvvvv:vvvvvvvv|        :        :
///           |pppppppp:ssssssss|_______a:ssssssss: //designed
///           |pppppppp:ssssssss|a_______:ssssssss: //real

template<typename value_type, std::unsigned_integral size_type>
constexpr auto union_min_number_of_elements() noexcept -> size_type
  {
  auto struct_size = struct_min_byte_size<size_type>();
  return (struct_size - (1 + sizeof(size_type))) / sizeof(value_type);
  }

template<typename value_type, std::unsigned_integral size_type>
constexpr auto at_least(size_type min_required) noexcept -> size_type
  {
  return std::max(union_min_number_of_elements<value_type, size_type>(), min_required);
  }

enum struct small_vector_storage_type : uint8_t
  {
  buffered,
  dynamic
  };

template<typename V, std::unsigned_integral S>
struct storage_context_t
  {
  using value_type = V;
  using size_type = S;
#if !defined(WIN32)
  [[no_unique_address]]
#endif
  value_type * data;
#if !defined(WIN32)
  [[no_unique_address]]
#endif
  size_type capacity;

  // required to overcome Itanium ABI pod intended for standard layout compatibile causing padding
  constexpr storage_context_t() noexcept = default;

  constexpr storage_context_t(value_type * d, size_type c) noexcept : data{d}, capacity{c} {}
  };

// holder for auto reclamation during unwind for throwing types during copy
template<bool is_noexcept>
struct noexcept_if
  {
  static constexpr bool use_noexcept = is_noexcept;

  template<typename value_type, std::unsigned_integral size_type>
  struct cond_except_holder
    {
    using context = storage_context_t<value_type, size_type>;

    context ctx_;

    inline constexpr explicit operator bool() const noexcept { return ctx_.data != nullptr; }

    inline constexpr auto data() const noexcept { return ctx_.data; }

    inline constexpr cond_except_holder(context && rh) noexcept : ctx_{rh} {}

    inline constexpr context release() noexcept
      {
      if constexpr(use_noexcept)
        return ctx_;
      else
        return std::exchange(ctx_, {});
      }

    inline constexpr ~cond_except_holder()
      {
      if constexpr(!use_noexcept)
        if(ctx_.data != nullptr)
          sv_deallocate(ctx_);
      }
    };

  template<typename context>
    requires requires {
      typename context::value_type;
      typename context::size_type;
    }
  cond_except_holder(context && rh) -> cond_except_holder<typename context::value_type, typename context::size_type>;

  template<typename vector_type>
  struct cond_except_holder_revert
    {
    using value_type = typename vector_type::value_type;
    using size_type = typename vector_type::size_type;
    using context = storage_context_t<value_type, size_type>;
    vector_type & vec_;
    context ctx_;
    size_type size_;

    inline constexpr explicit operator bool() const noexcept { return ctx_.data != nullptr; }

    inline constexpr auto data() const noexcept { return ctx_.data; }

    inline constexpr cond_except_holder_revert(vector_type & vec, size_type size, context && rh) noexcept :
        vec_{vec},
        ctx_{rh},
        size_{size}
      {
      }

    inline constexpr context release() noexcept
      {
      if constexpr(use_noexcept)
        return ctx_;
      else
        return std::exchange(ctx_, {});
      }

    inline constexpr ~cond_except_holder_revert()
      {
      if constexpr(!use_noexcept)
        if(ctx_.data != nullptr)
          vec_.exchange_priv_(ctx_, size_);
      }
    };

  template<typename vector_type, typename context>
  cond_except_holder_revert(vector_type &, typename vector_type::size_type, context &&)
    -> cond_except_holder_revert<vector_type>;

  template<typename value_type>
  struct cond_destroy_at
    {
    value_type * ptr_;

    constexpr cond_destroy_at(value_type * ptr) noexcept : ptr_{ptr} {}

    inline constexpr auto release() noexcept
      {
      if constexpr(use_noexcept)
        return ptr_;
      else
        return std::exchange(ptr_, {});
      }

    inline constexpr ~cond_destroy_at()
      {
      if constexpr(!use_noexcept)
        if(ptr_ != nullptr)
          std::destroy_at(ptr_);
      }
    };

  template<typename value_type>
  cond_destroy_at(value_type * ptr) -> cond_destroy_at<value_type>;

  template<typename value_type, std::unsigned_integral size_type>
  struct cond_destroy_range
    {
    value_type * ptr_;
    size_type first_, last_;

    inline constexpr cond_destroy_range(value_type * ptr, size_type first, size_type last) noexcept :
        ptr_{ptr},
        first_{first},
        last_{last}
      {
      }

    inline constexpr void release() noexcept
      {
      if constexpr(!use_noexcept)
        ptr_ = nullptr;
      }

    inline constexpr ~cond_destroy_range()
      {
      if constexpr(!use_noexcept)
        if(ptr_ != nullptr)
          destroy_range(ptr_, first_, last_);
      }
    };

  template<typename value_type, std::unsigned_integral size_type>
  cond_destroy_range(value_type *, size_type, size_type) -> cond_destroy_range<value_type, size_type>;
  };

///\brief storage for small_vector
///       There is no sense making N smaller than counted from \ref union_min_number_of_elements
template<concepts::vector_constraints V, std::unsigned_integral S, uint64_t N = union_min_number_of_elements<V, S>()>
// requires(N >= union_min_number_of_elements<V, S>() or N == 0)
struct small_vector_storage
  {
  using value_type = V;
  using size_type = S;
  using enum small_vector_storage_type;

  static constexpr size_type buffered_capacity = N;

  // moved from requires for class as it was preventing storage_type selection in basic_string_t for
  // static_vector_storage with buffers with smaller number of elements
  static_assert(N >= union_min_number_of_elements<V, S>() or N == 0);

  static constexpr bool value_type_has_trivial_liftime
    = std::is_trivially_default_constructible_v<value_type> && std::is_trivially_destructible_v<value_type>;

  static constexpr bool supports_reallocation{true};

  using buffered_storage_type = std::conditional_t<
    value_type_has_trivial_liftime,
    std::array<value_type, buffered_capacity>,
    aligned_storage_for_no_trivial<value_type, buffered_capacity>>;

  using dynamic_storage_type = storage_context_t<value_type, size_type>;
  using storage_context_type = storage_context_t<value_type, size_type>;

  // https://eel.is/c++draft/class.union#5
#if !defined(WIN32)
    [[no_unique_address]]
#endif
    union storage_type {
    buffered_storage_type buffered;
#if !defined(WIN32)
    [[no_unique_address]]
#endif
    dynamic_storage_type dynamic;

    constexpr storage_type() noexcept : buffered() {}

    constexpr storage_type(dynamic_storage_type ds) noexcept : dynamic{ds} {}
    } data_;

#if !defined(WIN32)
  [[no_unique_address]]
#endif
  small_vector_storage_type active_;

#if !defined(WIN32)
  [[no_unique_address]]
#endif
  size_type size_;

  inline constexpr small_vector_storage_type active_storage() const noexcept { return active_; }

  [[nodiscard]]
  inline constexpr auto data() const noexcept -> value_type const *
    {
    if(active_ == buffered)
      return data_.buffered.data();
    else
      return data_.dynamic.data;
    }

  [[nodiscard]]
  inline constexpr auto data() noexcept -> value_type *
    {
    if(active_ == buffered)
      return data_.buffered.data();
    else
      return data_.dynamic.data;
    }

  [[nodiscard]]
  inline constexpr auto capacity() const noexcept -> size_type
    {
    if(active_ == buffered)
      return buffered_capacity;
    else
      return data_.dynamic.capacity;
    }

  [[nodiscard]]
  inline constexpr auto dynamic_storage() const noexcept -> dynamic_storage_type
    {
    return data_.dynamic;
    }

  [[nodiscard]]
  inline constexpr auto context() noexcept -> storage_context_type
    {
    if(active_ == buffered)
      return storage_context_type{data_.buffered.data(), buffered_capacity};
    else
      return data_.dynamic;
    }

  inline constexpr small_vector_storage(dynamic_storage_type ds, size_type sz) noexcept :
      data_{ds},
      active_{dynamic},
      size_{sz}
    {
    }

  inline constexpr small_vector_storage() noexcept : active_{buffered}, size_{} {}

  inline constexpr void
    construct_move(small_vector_storage && rh) noexcept(std::is_nothrow_move_constructible_v<value_type>)
    requires std::move_constructible<value_type>
    {
    constexpr bool use_nothrow
      = concepts::is_trivially_relocatable<value_type> or std::is_nothrow_move_constructible_v<value_type>;

    if(rh.active_ == buffered)
      if constexpr(use_nothrow)
        uninitialized_relocate_n(rh.data_.buffered.data(), rh.size_, data_.buffered.data());
      else
        uninitialized_relocate_with_copy_n(rh.data_.buffered.data(), rh.size_, data_.buffered.data());
    else
      {
      data_ = rh.data_.dynamic;
      rh.data_ = {};
      }
    size_ = std::exchange(rh.size_, 0u);
    active_ = std::exchange(rh.active_, buffered);
    }

  constexpr void assign_move(small_vector_storage && rh) noexcept(std::is_nothrow_move_assignable_v<value_type>)
    requires std::movable<value_type>
    {
    constexpr bool use_nothrow
      = concepts::is_trivially_relocatable<value_type> or std::is_nothrow_move_assignable_v<value_type>;

    if constexpr(not std::is_trivially_destructible_v<value_type>)
      {
      size_type const old_size = size_;
      if(old_size != 0u)
        destroy_range(data(), size_type{0}, old_size);
      }
    // design decision if right is buffered then free space and left become buffered
    // on the other hand left may stay dynamic but elements will be copied too so there is no reason to hold memory
    if(active_ == dynamic)
      detail::sv_deallocate(dynamic_storage() /*data(), capacity()*/);

    if(rh.active_ == buffered)
      {
      // activate buffered in union
      if(active_ == dynamic)
        {
        data_ = {};
        active_ = buffered;
        }
      if constexpr(use_nothrow)
        uninitialized_relocate_n(rh.data_.buffered.data(), rh.size_, data_.buffered.data());
      else
        uninitialized_relocate_with_copy_n(rh.data_.buffered.data(), rh.size_, data_.buffered.data());
      }
    else
      {
      data_ = rh.data_.dynamic;
      active_ = dynamic;
      rh.data_ = {};
      rh.active_ = buffered;
      }
    size_ = std::exchange(rh.size_, 0);
    }

  constexpr void swap(small_vector_storage & rh) noexcept(std::is_nothrow_copy_assignable_v<value_type>)
    requires std::copyable<value_type>
    {
    int32_t const swap_case = ((active_ == dynamic) << 1) | (rh.active_ == dynamic);
    switch(swap_case)
      {
      case 0:  // both static
          {
          uninitialized_uneven_range_swap(data_.buffered.data(), size_, rh.data_.buffered.data(), rh.size_);
          break;
          }
      case 1:  // left static right dynamic
          {
          auto dyn = rh.data_.dynamic;
          rh.data_ = {};
          rh.active_ = buffered;
          uninitialized_copy_n(data_.buffered.data(), size_, rh.data_.buffered.data());
          data_ = dyn;
          active_ = dynamic;
          break;
          }
      case 2:  // left dynamic right static
          {
          auto dyn = data_.dynamic;
          data_ = {};
          active_ = buffered;
          uninitialized_copy_n(rh.data_.buffered.data(), rh.size_, data_.buffered.data());
          rh.data_ = dyn;
          rh.active_ = dynamic;
          break;
          }
      case 3:  // both dynamic
        std::swap(data_.dynamic, rh.data_.dynamic);
        break;
      }
    std::swap(size_, rh.size_);
    }

  template<uint64_t M>
  ///\warning copy constructor may throw always, for dynamic as there is no other way to signalize allocation error
  constexpr void construct_copy(small_vector_storage<V, S, M> const & rh)
    requires std::copy_constructible<value_type>
    {
    size_type my_size = rh.size_;
    if(my_size > buffered_capacity)
      {
      // design decision dont overallocate
      // size_type new_capacity{ detail::growth(my_size, size_type(0u) ) };
      size_type const new_capacity{my_size};
      typename noexcept_if<std::is_nothrow_copy_constructible_v<value_type>>::cond_except_holder new_space{
        detail::sv_allocate<value_type>(new_capacity)
      };
      if(new_space.data())
        {
        uninitialized_copy_n(rh.data(), my_size, new_space.data());
        data_ = new_space.release();
        active_ = dynamic;
        size_ = my_size;
        }
      else
        throw std::bad_alloc{};
      }
    else
      {
      active_ = buffered;
      uninitialized_copy_n(rh.data(), my_size, data_.buffered.data());
      size_ = my_size;
      }
    }

  template<uint64_t M>
  constexpr void assign_copy(small_vector_storage<V, S, M> const & rh)
    requires std::copyable<value_type>
    {
    if constexpr(!std::is_trivially_destructible_v<value_type>)
      destroy_range(data(), size_type(0u), size_);

    size_ = 0u;

    size_type my_size = rh.size_;

    // for buffered lh.capacity_ can never be less than rh.size_
    if(capacity() < my_size)
      {
      if(active_ == dynamic)
        detail::sv_deallocate(dynamic_storage());

      // design decision dont overallocate
      // size_type new_capacity{ detail::growth(my_size, size_type(0u) ) };
      size_type const new_capacity{my_size};

      typename noexcept_if<std::is_nothrow_copy_constructible_v<value_type>>::cond_except_holder new_space{
        detail::sv_allocate<value_type>(new_capacity)
      };
      if(new_space)
        {
        uninitialized_copy_n(rh.data(), my_size, new_space.data());
        data_ = new_space.release();
        size_ = my_size;
        active_ = dynamic;
        }
      else
        throw std::bad_alloc{};
      }
    else
      {
      if(active_ == dynamic && my_size <= buffered_capacity)
        {
        // design decision if right is buffered then free space and left become buffered
        // no matter what we have to copy data
        detail::sv_deallocate(data_.dynamic);
        data_ = {};
        active_ = buffered;
        }

      uninitialized_copy_n(rh.data(), my_size, data());
      size_ = my_size;
      }
    }

  constexpr void destroy() noexcept
    {
    if constexpr(!std::is_trivially_destructible_v<value_type>)
      destroy_range(data(), size_type{0}, size_);
    if(active_ == dynamic)
      detail::sv_deallocate(dynamic_storage());
    }

  inline constexpr auto switch_static_priv_() noexcept -> storage_context_type
    {
    storage_context_type result{data_.dynamic};
    data_ = {};
    active_ = buffered;
    size_ = 0u;
    return result;
    }

  inline constexpr auto exchange_priv_(storage_context_type new_storage, size_type size) noexcept
    -> storage_context_type
    {
    if(active_ == dynamic)
      {
      size_ = size;
      return std::exchange(data_.dynamic, new_storage);
      }
    else
      {
      // required approach for changin active union member type
      data_ = new_storage;
      size_ = size;
      active_ = dynamic;
      return {};
      }
    }

  inline constexpr void set_size_priv_(size_type pos_ix) noexcept { size_ = pos_ix; }
  };

template<concepts::vector_constraints V, std::unsigned_integral S>
struct small_vector_storage<V, S, 0u>
  {
  using value_type = V;
  using size_type = S;
  using storage_context_type = storage_context_t<value_type, size_type>;
  using dynamic_storage_type = storage_context_t<value_type, size_type>;
  using storage_type = dynamic_storage_type;

  static constexpr size_type buffered_capacity = 0;

#if !defined(WIN32)
  [[no_unique_address]]
#endif
  storage_context_type dynamic;

#if !defined(WIN32)
  [[no_unique_address]]
#endif
  size_type size_;

  static inline constexpr auto active_storage() noexcept -> small_vector_storage_type
    {
    return small_vector_storage_type::dynamic;
    }

  [[nodiscard]]
  inline constexpr auto data() const noexcept -> value_type const *
    {
    return dynamic.data;
    }

  [[nodiscard]]
  inline constexpr auto data() noexcept -> value_type *
    {
    return dynamic.data;
    }

  [[nodiscard]]
  inline constexpr auto capacity() const noexcept -> size_type
    {
    return dynamic.capacity;
    }

  [[nodiscard]]
  inline constexpr auto context() noexcept -> storage_context_type
    {
    return dynamic;
    }

  inline constexpr small_vector_storage(storage_context_type ds, size_type sz) noexcept : dynamic{ds}, size_{sz} {}

  inline constexpr small_vector_storage() noexcept : dynamic{}, size_{} {}

  inline constexpr void construct_move(small_vector_storage && rh) noexcept
    {
    dynamic = std::exchange(rh.dynamic, {});
    size_ = std::exchange(rh.size_, 0u);
    }

  constexpr void assign_move(small_vector_storage && rh) noexcept
    requires std::movable<value_type>
    {
    size_type const old_size = size_;
    if(old_size != 0u)
      {
      if constexpr(not std::is_trivially_destructible_v<value_type>)
        destroy_range(data(), size_type{0}, old_size);

      // design decision if right is buffered then free space and left become buffered
      // on the other hand left may stay dynamic but elements will be copied too so there is no reason to hold memory
      detail::sv_deallocate(dynamic);
      }
    dynamic = std::exchange(rh.dynamic, {});
    size_ = std::exchange(rh.size_, 0);
    }

  constexpr void swap(small_vector_storage & rh) noexcept
    requires std::movable<value_type>
    {
    std::swap(dynamic, rh.dynamic);
    std::swap(size_, rh.size_);
    }

  template<uint64_t M>
  constexpr void construct_copy(small_vector_storage<V, S, M> const & rh)
    requires std::copy_constructible<value_type>
    {
    size_type my_size = rh.size_;

    // design decision dont overallocate
    //  size_type new_capacity{ detail::growth(my_size, size_type(0u) ) };
    size_type new_capacity{my_size};
    typename noexcept_if<std::is_nothrow_copy_constructible_v<value_type>>::cond_except_holder new_space{
      detail::sv_allocate<value_type>(new_capacity)
    };
    if(new_space)
      {
      uninitialized_copy_n(rh.data(), my_size, new_space.data());
      dynamic = new_space.release();
      size_ = my_size;
      }
    else
      throw std::bad_alloc{};
    }

  template<uint64_t M>
  constexpr void assign_copy(small_vector_storage<V, S, M> const & rh)
    requires std::copyable<value_type>
    {
    if constexpr(!std::is_trivially_destructible_v<value_type>)
      if(size_ != 0u)
        destroy_range(data(), size_type(0u), size_);
    size_ = 0u;

    size_type my_size = rh.size_;

    if(capacity() < my_size)
      {
      if(data() != nullptr)
        {
        detail::sv_deallocate(dynamic);
        dynamic = {};
        }
      // design decision dont overallocate
      //         size_type new_capacity{ detail::growth(my_size, size_type(0u) ) };
      size_type new_capacity{my_size};
      typename noexcept_if<std::is_nothrow_copy_constructible_v<value_type>>::cond_except_holder new_space{
        detail::sv_allocate<value_type>(new_capacity)
      };
      if(new_space)
        {
        uninitialized_copy_n(rh.data(), my_size, new_space.data());
        dynamic = new_space.release();
        size_ = my_size;
        }
      else
        throw std::bad_alloc{};
      }
    else
      uninitialized_copy_n(rh.data(), my_size, data());
    size_ = my_size;
    }

  constexpr void destroy() noexcept
    {
    if(data() != nullptr)
      {
      if constexpr(!std::is_trivially_destructible_v<value_type>)
        destroy_range(data(), size_type{0}, size_);
      detail::sv_deallocate(dynamic);
      }
    }

  inline constexpr auto exchange_priv_(storage_context_type new_storage, size_type size) noexcept
    -> storage_context_type
    {
    size_ = size;
    return std::exchange(dynamic, new_storage);
    }

  inline constexpr void set_size_priv_(size_type pos_ix) noexcept { size_ = pos_ix; }
  };

  }  // namespace small_vectors::inline v3_3::detail

