#pragma once

#include <small_vectors/detail/vector_func.h>
#include <algorithm>
#include <string_view>
#include <iterator>
#include <ranges>
#include <small_vectors/utils/static_call_operator.h>

namespace small_vectors::inline v3_0::detail::string
  {
struct static_string_tag
  {
  };

struct buffered_string_tag
  {
  };

template<typename tag>
concept static_storage_tag = std::same_as<tag, static_string_tag>;

template<typename tag>
concept buffered_storage_tag = std::same_as<tag, buffered_string_tag>;

template<typename storage_tag, typename storage_type>
  requires static_storage_tag<storage_tag>
inline constexpr void storage_cleanup(storage_type &) noexcept
  {
  }

template<typename storage_tag, typename storage_type>
  requires detail::string::buffered_storage_tag<storage_tag>
inline constexpr void storage_cleanup(storage_type & storage) noexcept
  {
  storage.destroy();
  }

inline constexpr bool null_terminate_string = true;
inline constexpr uint8_t null_termination = null_terminate_string ? 1u : 0u;

//-------------------------------------------------------------------------------------------------------------------
template<typename char_type>
struct growth_t
  {
  template<typename size_type>
  [[nodiscard]]
  small_vector_static_call_operator constexpr auto
    operator()(size_type new_elements) small_vector_static_call_operator_const noexcept -> size_type
    {
    auto byte_size = static_cast<size_type>(sizeof(char_type) * new_elements);
    return static_cast<size_type>((((byte_size + 8u) >> 3) << 3) / sizeof(char_type));
    }
  };

template<typename char_type>
inline constexpr growth_t<char_type> growth;

//-------------------------------------------------------------------------------------------------------------------
struct cond_null_terminate_t
  {
  template<typename char_type>
  small_vector_static_call_operator inline constexpr void operator()(char_type * data
  ) small_vector_static_call_operator_const noexcept
    {
    if constexpr(null_terminate_string)
      uninitialized_value_construct(data);
    }

  template<typename char_type, typename size_type>
  small_vector_static_call_operator inline constexpr void
    operator()(char_type * data, size_type new_size) small_vector_static_call_operator_const noexcept
    {
    if constexpr(null_terminate_string)
      uninitialized_value_construct(data + new_size);
    }
  };

inline constexpr cond_null_terminate_t cond_null_terminate;

//-------------------------------------------------------------------------------------------------------------------
struct clear_t
  {
  template<detail_concepts::vector_storage vector_storage>
  small_vector_static_call_operator inline constexpr void operator()(vector_storage & storage
  ) small_vector_static_call_operator_const noexcept
    {
    storage.size_ = 0u;
    cond_null_terminate(storage.data());
    }
  };

inline constexpr clear_t clear;

//-------------------------------------------------------------------------------------------------------------------
struct shrink_to_fit_t
  {
  template<detail_concepts::vector_storage vector_storage>
  small_vector_static_call_operator inline constexpr void operator()(vector_storage & storage
  ) small_vector_static_call_operator_const
    requires(vector_storage::supports_reallocation)
    {
    using char_type = typename vector_storage::value_type;
    using size_type = typename vector_storage::size_type;
    if(storage.capacity() > storage.buffered_capacity)
      {
      size_type const raw_size = storage.size_ + null_termination;
      if(storage.buffered_capacity < raw_size)
        {
        auto optimal_capacity{growth<char_type>(raw_size)};
        if(optimal_capacity != storage.capacity())
          {
          auto old_strage_data{storage.data()};
          storage_context_t new_space{sv_allocate<char_type>(optimal_capacity)};
          uninitialized_copy(old_strage_data, old_strage_data + storage.size_, new_space.data);
          storage_context_t old_storage{storage.exchange_priv_(new_space, storage.size_)};
          assert(old_storage.data != nullptr);
          sv_deallocate(old_storage);
          }
        }
      else
        {
        size_type size{storage.size_};
        storage_context_t old_storage{storage.switch_static_priv_()};
        assert(old_storage.data != nullptr);
        auto data{storage.data()};
        uninitialized_copy(old_storage.data, old_storage.data + size, data);
        cond_null_terminate(data + size);
        storage.size_ = size;
        sv_deallocate(old_storage);
        }
      }
    }
  };

inline constexpr shrink_to_fit_t shrink_to_fit;

//-------------------------------------------------------------------------------------------------------------------
template<detail_concepts::vector_storage vector_storage>
struct uninitialized_construct_t
  {
  template<typename lambda>
  [[nodiscard]]
  small_vector_static_call_operator inline constexpr auto
    operator()(typename vector_storage::size_type sz, lambda const & uninitialized_fn)
      small_vector_static_call_operator_const->vector_storage
    {
    vector_storage storage;

    using char_type = typename vector_storage::value_type;
    using size_type = typename vector_storage::size_type;
    size_type const raw_size = sz + null_termination;

    if(storage.buffered_capacity < raw_size) [[unlikely]]  // optimise for small buffered strings
      {
      if constexpr(vector_storage::supports_reallocation)
        {
        auto new_capacity{growth<char_type>(raw_size)};
        storage_context_t new_space{sv_allocate<char_type>(new_capacity)};
        auto last = uninitialized_fn(new_space.data);
        cond_null_terminate(last);
        storage.data_ = new_space;
        storage.active_ = small_vector_storage_type::dynamic;
        }
      else
        throw std::length_error{"Out of buffer space"};
      }
    else  // if( sz != 0u )
      {
      auto dest_data{storage.data()};
      // in constant evaluated buffer must be initialized even when unused
      if(std::is_constant_evaluated())
        uninitialized_default_construct(dest_data, dest_data + storage.buffered_capacity);

      auto last = uninitialized_fn(dest_data);
      cond_null_terminate(last);

      if constexpr(vector_storage::supports_reallocation)
        storage.active_ = small_vector_storage_type::buffered;
      }
    storage.size_ = sz;
    return storage;
    }
  };

template<detail_concepts::vector_storage vector_storage>
inline constexpr uninitialized_construct_t<vector_storage> uninitialized_construct;

//-------------------------------------------------------------------------------------------------------------------
struct uninitialized_assign_t
  {
  template<detail_concepts::vector_storage vector_storage, typename lambda>
  small_vector_static_call_operator inline constexpr void operator()(
    vector_storage & storage, typename vector_storage::size_type sz, lambda const & uninitialized_fn
  ) small_vector_static_call_operator_const
    {
    using char_type = typename vector_storage::value_type;
    using size_type = typename vector_storage::size_type;
    size_type const raw_size = sz + null_termination;

    if constexpr(vector_storage::supports_reallocation)
      if(raw_size <= vector_storage::buffered_capacity && storage.active_ == small_vector_storage_type::dynamic)
        [[unlikely]]
        {
        storage_context_t old_storage{storage.switch_static_priv_()};
        if(old_storage.data != nullptr)
          sv_deallocate(old_storage);
        }

    if(storage.capacity() < raw_size) [[unlikely]]  // optimise for small buffered strings
      {
      if constexpr(vector_storage::supports_reallocation)
        {
        auto new_capacity{growth<char_type>(raw_size)};
        storage_context_t new_space{sv_allocate<char_type>(new_capacity)};
        auto last = uninitialized_fn(new_space.data);
        cond_null_terminate(last);
        // deallocate old space
        storage_context_t old_storage{storage.exchange_priv_(new_space, sz)};
        if(old_storage.data != nullptr) [[unlikely]]
          sv_deallocate(old_storage);
        }
      else
        throw std::length_error{"Out of buffer space"};
      }
    else
      {
      auto dest_data{storage.data()};
      auto last = uninitialized_fn(dest_data);
      cond_null_terminate(last);
      }
    storage.size_ = sz;
    }
  };

inline constexpr uninitialized_assign_t uninitialized_assign;

//-------------------------------------------------------------------------------------------------------------------
struct swap_t
  {
  template<detail_concepts::vector_storage vector_storage>
  small_vector_static_call_operator inline constexpr void
    operator()(vector_storage & lstorage, vector_storage & rstorage) small_vector_static_call_operator_const noexcept
    {
    lstorage.swap(rstorage);
    cond_null_terminate(lstorage.data() + lstorage.size_);
    cond_null_terminate(rstorage.data() + rstorage.size_);
    }
  };

inline constexpr swap_t swap;

//-------------------------------------------------------------------------------------------------------------------
template<detail_concepts::vector_storage vector_storage>
struct copy_construct_t
  {
  template<concepts::forward_iterator iterator>
  [[nodiscard]]
  small_vector_static_call_operator inline constexpr auto
    operator()(iterator first, iterator last) small_vector_static_call_operator_const->vector_storage
    {
    using size_type = typename vector_storage::size_type;
    return uninitialized_construct<vector_storage>(
      static_cast<size_type>(std::ranges::distance(first, last)),
      [&first, &last](auto * data) noexcept { return uninitialized_copy(first, last, data); }
    );
    }
  };

template<detail_concepts::vector_storage vector_storage>
inline constexpr copy_construct_t<vector_storage> copy_construct;

//-------------------------------------------------------------------------------------------------------------------
struct assign_copy_t
  {
  template<detail_concepts::vector_storage vector_storage, concepts::forward_iterator iterator>
  small_vector_static_call_operator inline constexpr void
    operator()(vector_storage & storage, iterator first, iterator last) small_vector_static_call_operator_const
    {
    using size_type = typename vector_storage::size_type;
    uninitialized_assign(
      storage,
      static_cast<size_type>(std::ranges::distance(first, last)),
      [&first, &last](auto * data) noexcept { return uninitialized_copy(first, last, data); }
    );
    }
  };

inline constexpr assign_copy_t assign_copy;

//-------------------------------------------------------------------------------------------------------------------
template<detail_concepts::vector_storage vector_storage>
struct value_construct_t
  {
  [[nodiscard]]
  small_vector_static_call_operator inline constexpr auto
    operator()(typename vector_storage::size_type sz) small_vector_static_call_operator_const->vector_storage
    {
    return uninitialized_construct<vector_storage>(
      sz, [sz](auto * data) noexcept { return uninitialized_value_construct(data, data + sz); }
    );
    }
  };

template<detail_concepts::vector_storage vector_storage>
inline constexpr value_construct_t<vector_storage> value_construct;

//-------------------------------------------------------------------------------------------------------------------
template<detail_concepts::vector_storage vector_storage>
struct fill_construct_t
  {
  [[nodiscard]]
  small_vector_static_call_operator inline constexpr auto
    operator()(typename vector_storage::size_type sz, typename vector_storage::value_type ch)
      small_vector_static_call_operator_const->vector_storage
    {
    return uninitialized_construct<vector_storage>(
      sz, [sz, ch](auto * data) noexcept { return uninitialized_fill(data, data + sz, ch); }
    );
    }
  };

template<detail_concepts::vector_storage vector_storage>
inline constexpr fill_construct_t<vector_storage> fill_construct;

//-------------------------------------------------------------------------------------------------------------------
struct fill_assign_t
  {
  template<detail_concepts::vector_storage vector_storage>
  small_vector_static_call_operator inline constexpr void operator()(
    vector_storage & storage, typename vector_storage::size_type sz, typename vector_storage::value_type ch
  ) small_vector_static_call_operator_const
    {
    uninitialized_assign(
      storage, sz, [sz, ch](auto * data) noexcept { return uninitialized_fill(data, data + sz, ch); }
    );
    }
  };

inline constexpr fill_assign_t fill_assign;

//-------------------------------------------------------------------------------------------------------------------
struct reserve_t
  {
  template<detail_concepts::vector_storage vector_storage>
  small_vector_static_call_operator inline constexpr void operator()(
    vector_storage & storage, typename vector_storage::size_type req_capacity
  ) small_vector_static_call_operator_const
    {
    using char_type = typename vector_storage::value_type;
    using size_type = typename vector_storage::size_type;
    size_type const raw_size = req_capacity + null_termination;
    if(raw_size > storage.capacity())
      {
      if constexpr(vector_storage::supports_reallocation)
        {
        auto new_capacity{growth<char_type>(raw_size)};
        storage_context_t new_space{sv_allocate<char_type>(new_capacity)};
        auto first{storage.data()};
        auto last{first + storage.size_ + null_termination};
        uninitialized_copy(first, last, new_space.data);  // copies conditionally null termination

        // deallocate old space
        storage_context_t old_storage{storage.exchange_priv_(new_space, storage.size_)};
        if(old_storage.data != nullptr) [[unlikely]]
          sv_deallocate(old_storage);
        }
      else
        throw std::length_error{"Out of buffer space"};
      }
    }
  };

inline constexpr reserve_t reserve;

//-------------------------------------------------------------------------------------------------------------------
struct resize_and_overwrite_t
  {
  template<detail_concepts::vector_storage vector_storage, typename Operation>
  small_vector_static_call_operator inline constexpr void operator()(
    vector_storage & storage, typename vector_storage::size_type sz, Operation op
  ) small_vector_static_call_operator_const
    {
    using char_type = typename vector_storage::value_type;
    using size_type = typename vector_storage::size_type;
    size_type const raw_size = sz + null_termination;

    if(raw_size <= storage.capacity())
      {
      auto data{storage.data()};
      typename vector_storage::size_type new_size{op(data, storage.capacity())};
      cond_null_terminate(data + new_size);
      storage.size_ = new_size;
      }
    else if constexpr(vector_storage::supports_reallocation)
      {
      auto new_capacity{growth<char_type>(raw_size)};
      storage_context_t new_space{sv_allocate<char_type>(new_capacity)};
      auto data{storage.data()};
      uninitialized_copy(data, data + storage.size_, new_space.data);
      if(std::is_constant_evaluated())
        {
        // for constant evaluated mode we have to initialize storage to allow user operator assignment
        // in constant evaluated mode assignment to uninitialized store is not allowed
        uninitialized_value_construct(new_space.data + storage.size_, new_space.data + new_capacity);
        }
      typename vector_storage::size_type new_size{op(new_space.data, new_capacity - null_termination)};
      cond_null_terminate(new_space.data + new_size);

      // deallocate old space
      storage_context_t old_storage{storage.exchange_priv_(new_space, new_size)};
      if(old_storage.data != nullptr) [[unlikely]]
        sv_deallocate(old_storage);
      }
    else
      throw std::length_error{"Out of buffer space"};
    }
  };

inline constexpr resize_and_overwrite_t resize_and_overwrite;

//-------------------------------------------------------------------------------------------------------------------
struct resize_t
  {
  template<detail_concepts::vector_storage vector_storage>
  small_vector_static_call_operator inline constexpr void
    operator()(vector_storage & storage, typename vector_storage::size_type sz) small_vector_static_call_operator_const
    {
    using char_type = typename vector_storage::value_type;
    using size_type = typename vector_storage::size_type;

    resize_and_overwrite(
      storage,
      sz,
      [old_size = storage.size_, sz](char_type * data, size_type /*buff_cap*/) noexcept -> size_type
      {
        if(sz > old_size)
          uninitialized_value_construct(data + old_size, data + sz);
        return sz;
      }
    );
    }

  template<detail_concepts::vector_storage vector_storage>
  small_vector_static_call_operator inline constexpr void operator()(
    vector_storage & storage, typename vector_storage::size_type sz, typename vector_storage::value_type ch
  ) small_vector_static_call_operator_const
    {
    using char_type = typename vector_storage::value_type;
    using size_type = typename vector_storage::size_type;

    resize_and_overwrite(
      storage,
      sz,
      [old_size = storage.size_, sz, ch](char_type * data, size_type /*buff_cap*/) noexcept -> size_type
      {
        if(sz > old_size)
          uninitialized_fill(data + old_size, data + sz, ch);
        return sz;
      }
    );
    }
  };

inline constexpr resize_t resize;

//-------------------------------------------------------------------------------------------------------------------
struct replace_aux_t
  {
  template<detail_concepts::vector_storage vector_storage, typename Operation>
  small_vector_static_call_operator inline constexpr void operator()(
    vector_storage & storage,
    typename vector_storage::size_type pos,
    typename vector_storage::size_type old_size,
    typename vector_storage::size_type new_size,
    Operation op
  ) small_vector_static_call_operator_const
    {
    using char_type = typename vector_storage::value_type;
    using size_type = typename vector_storage::size_type;

    if(pos > storage.size_) [[unlikely]]
      throw std::out_of_range("index is out of range");

    size_type str_new_size = storage.size_ - old_size + new_size;
    size_type const raw_size = str_new_size + null_termination;
    if(raw_size <= storage.capacity())
      {
      // aaaaaRRRRRaaaaaa
      // aaaaaNNNNNaaaaaa no copy
      // aaaaaNNNaaaaaa shrinking
      // aaaaaNNNNNNNaaaaaa extending, case with 0 old size is just insert
      char_type * data{storage.data()};
      if(old_size == new_size) {}
      else if(old_size > new_size)
        uninitialized_copy(data + pos + old_size, data + storage.size_, data + pos + new_size);
      else
        {
        auto ito_end{std::reverse_iterator{data + pos + old_size}};
        auto ito_begin{std::reverse_iterator{data + storage.size_}};
        auto itn_begin{std::reverse_iterator{data + storage.size_ - old_size + new_size}};
        uninitialized_copy(ito_begin, ito_end, itn_begin);
        }
      op(data + pos);
      cond_null_terminate(data + str_new_size);
      storage.size_ = str_new_size;
      }
    else if constexpr(vector_storage::supports_reallocation)
      {
      size_type new_capacity{growth<char_type>(raw_size)};
      storage_context_t new_space{sv_allocate<char_type>(new_capacity)};
      char_type * data{storage.data()};
      uninitialized_copy(data, data + pos, new_space.data);
      op(new_space.data + pos);
      uninitialized_copy(data + pos + old_size, data + storage.size_, new_space.data + pos + new_size);
      cond_null_terminate(new_space.data + str_new_size);
      // deallocate old space
      storage_context_t old_storage{storage.exchange_priv_(new_space, str_new_size)};
      if(old_storage.data != nullptr) [[unlikely]]
        sv_deallocate(old_storage);
      }
    else
      throw std::length_error{"Out of buffer space"};
    }
  };

inline constexpr replace_aux_t replace_aux;

//-------------------------------------------------------------------------------------------------------------------
struct replace_copy_t
  {
  template<detail_concepts::vector_storage vector_storage, concepts::forward_iterator iterator>
  small_vector_static_call_operator inline constexpr void operator()(
    vector_storage & storage,
    typename vector_storage::size_type pos,
    typename vector_storage::size_type count,
    iterator first,
    iterator last
  ) small_vector_static_call_operator_const
    {
    using size_type = typename vector_storage::size_type;
    using char_type = typename vector_storage::value_type;
    size_type new_count{static_cast<size_type>(std::ranges::distance(first, last))};
    replace_aux(
      storage,
      pos,
      count,
      new_count,
      [&first, &last](char_type * data) noexcept { uninitialized_copy(first, last, data); }
    );
    }
  };

inline constexpr replace_copy_t replace_copy;

//-------------------------------------------------------------------------------------------------------------------
struct replace_fill_t
  {
  template<detail_concepts::vector_storage vector_storage>
  small_vector_static_call_operator inline constexpr void operator()(
    vector_storage & storage,
    typename vector_storage::size_type pos,
    typename vector_storage::size_type count,
    typename vector_storage::size_type count2,
    typename vector_storage::value_type ch
  ) small_vector_static_call_operator_const
    {
    using char_type = typename vector_storage::value_type;
    replace_aux(
      storage,
      pos,
      count,
      count2,
      [count2, ch](char_type * data) noexcept { uninitialized_fill(data, data + count2, ch); }
    );
    }
  };

inline constexpr replace_fill_t replace_fill;

//-------------------------------------------------------------------------------------------------------------------
struct insert_fill_t
  {
  template<detail_concepts::vector_storage vector_storage>
  small_vector_static_call_operator inline constexpr void operator()(
    vector_storage & storage,
    typename vector_storage::size_type pos,
    typename vector_storage::size_type count,
    typename vector_storage::value_type ch
  ) small_vector_static_call_operator_const
    {
    using char_type = typename vector_storage::value_type;

    using size_type = typename vector_storage::size_type;
    replace_aux(
      storage,
      pos,
      size_type(0u),
      count,
      [count, ch](char_type * data) noexcept { uninitialized_fill(data, data + count, ch); }
    );
    }
  };

inline constexpr insert_fill_t insert_fill;

//-------------------------------------------------------------------------------------------------------------------
struct insert_copy_t
  {
  template<detail_concepts::vector_storage vector_storage, concepts::forward_iterator iterator>
  small_vector_static_call_operator inline constexpr void operator()(
    vector_storage & storage, typename vector_storage::size_type pos, iterator first, iterator last
  ) small_vector_static_call_operator_const
    {
    using size_type = typename vector_storage::size_type;
    using char_type = typename vector_storage::value_type;
    size_type count{static_cast<size_type>(std::ranges::distance(first, last))};
    replace_aux(
      storage,
      pos,
      size_type(0u),
      count,
      [&first, &last](char_type * data) noexcept { uninitialized_copy(first, last, data); }
    );
    }
  };

inline constexpr insert_copy_t insert_copy;

//-------------------------------------------------------------------------------------------------------------------
struct append_fill_t
  {
  template<detail_concepts::vector_storage vector_storage>
  small_vector_static_call_operator inline constexpr void operator()(
    vector_storage & storage, typename vector_storage::size_type count, typename vector_storage::value_type ch
  ) small_vector_static_call_operator_const
    {
    using char_type = typename vector_storage::value_type;
    using size_type = typename vector_storage::size_type;
    size_type new_size{static_cast<size_type>(storage.size_ + count)};
    resize_and_overwrite(
      storage,
      new_size,
      [old_size = storage.size_, new_size, ch](char_type * data, size_type /*buff_cap*/) noexcept -> size_type
      {
        uninitialized_fill(data + old_size, data + new_size, ch);
        return new_size;
      }
    );
    }
  };

inline constexpr append_fill_t append_fill;

//-------------------------------------------------------------------------------------------------------------------
struct append_copy_t
  {
  template<detail_concepts::vector_storage vector_storage, concepts::forward_iterator iterator>
  small_vector_static_call_operator inline constexpr void
    operator()(vector_storage & storage, iterator first, iterator last) small_vector_static_call_operator_const
    {
    using char_type = typename vector_storage::value_type;
    using size_type = typename vector_storage::size_type;
    size_type count{static_cast<size_type>(std::ranges::distance(first, last))};
    size_type new_size{static_cast<size_type>(storage.size_ + count)};
    resize_and_overwrite(
      storage,
      new_size,
      [old_size = storage.size_, new_size, first, last](char_type * data, size_type /*buff_cap*/) noexcept -> size_type
      {
        uninitialized_copy(first, last, data + old_size);
        return new_size;
      }
    );
    }
  };

inline constexpr append_copy_t append_copy;

//-------------------------------------------------------------------------------------------------------------------

struct push_back_t
  {
  template<detail_concepts::vector_storage vector_storage>
  small_vector_static_call_operator inline constexpr void
    operator()(vector_storage & storage, typename vector_storage::value_type ch) small_vector_static_call_operator_const
    {
    append_fill(storage, 1u, ch);
    }
  };

inline constexpr push_back_t push_back;

//-------------------------------------------------------------------------------------------------------------------
struct erase_t
  {
  template<detail_concepts::vector_storage vector_storage>
  small_vector_static_call_operator inline constexpr void operator()(
    vector_storage & storage, typename vector_storage::size_type index, typename vector_storage::size_type count
  ) small_vector_static_call_operator_const noexcept
    {
    using size_type = typename vector_storage::size_type;
    using char_type = typename vector_storage::value_type;

    count = std::min<size_type>(count, storage.size_ - index);
    size_type rpos = index + count;
    char_type * data{storage.data()};
    std::ranges::copy(data + rpos, data + storage.size_, data + index);
    storage.size_ -= count;
    cond_null_terminate(data + storage.size_);
    }
  };

inline constexpr erase_t erase;

//-------------------------------------------------------------------------------------------------------------------
struct pop_back_t
  {
  template<detail_concepts::vector_storage vector_storage>
  small_vector_static_call_operator inline constexpr void operator()(vector_storage & storage
  ) small_vector_static_call_operator_const noexcept
    {
    using char_type = typename vector_storage::value_type;
    char_type * data{storage.data()};
    storage.size_ -= 1u;
    cond_null_terminate(data + storage.size_);
    }
  };

inline constexpr pop_back_t pop_back;

  }  // namespace small_vectors::inline v3_0::detail::string

