#pragma once
#include <small_vectors/concepts/concepts.h>
#include <small_vectors/detail/uninitialized_constexpr.h>
#include <small_vectors/detail/vector_storage.h>
#include <small_vectors/utils/enum_support.h>
#include <memory>
#include <stdexcept>

namespace small_vectors::inline v3_0::detail
  {

enum struct vector_outcome_e : uint8_t
  {
  no_error,
  out_of_storage,
  invalid_source_range
  };

//-------------------------------------------------------------------------------------------------------------------
// excaption handling for classes
inline constexpr void handle_error(vector_outcome_e error)
  {
  using enum vector_outcome_e;
  switch(error)
    {
    [[likely]] case no_error:
      return;
    [[unlikely]] case out_of_storage:
      throw std::bad_alloc{};
    [[unlikely]] case invalid_source_range:
      throw std::runtime_error{"invalid_source_range"};
    }
  }
//-------------------------------------------------------------------------------------------------------------------
enum struct vector_tune_e : uint8_t
  {
  unchecked,
  checked
  };
//-------------------------------------------------------------------------------------------------------------------
template<typename vector_type>
concept has_size_member_function = requires(vector_type const & vec) {
  typename vector_type::size_type;
    {
    vec.size()
    } noexcept -> std::same_as<typename vector_type::size_type>;
};

//-------------------------------------------------------------------------------------------------------------------
[[nodiscard]]
inline constexpr auto size(has_size_member_function auto const & vec) noexcept
  {
  return vec.size();
  }

//-------------------------------------------------------------------------------------------------------------------
[[nodiscard]]
inline constexpr bool empty(has_size_member_function auto const & vec) noexcept
  {
  return vec.size() == 0;
  }

//-------------------------------------------------------------------------------------------------------------------
template<typename T, typename Expected>
concept value_type_or_lv_refrence = std::same_as<Expected, std::remove_cvref_t<T>>;
//-------------------------------------------------------------------------------------------------------------------
template<typename T, typename Expected>
concept const_or_nonconst_pointer = std::same_as<T, Expected *> or std::same_as<T, Expected const *>;
//-------------------------------------------------------------------------------------------------------------------
template<typename vector_type, typename size_type>
concept has_index_operator = requires(vector_type const & vec, size_type index) {
  requires has_size_member_function<vector_type>;
  requires concepts::unsigned_arithmetic_integral<size_type>;
  typename vector_type::value_type;
    {
    vec[index]
    } noexcept -> value_type_or_lv_refrence<typename vector_type::value_type>;
};
//-------------------------------------------------------------------------------------------------------------------
template<concepts::unsigned_arithmetic_integral size_type>
using growth_size_type_select_t =
  typename size_type_selector_t<std::min<unsigned>(8, sizeof(size_type) * 2)>::size_type;

///\brief small_vector size growth factor function
///\details f(size) = floor(size * 1.5) + 1
template<typename size_type>
constexpr size_type growth(size_type old_size, size_type new_elements) noexcept
  {
  using calc_size_type = growth_size_type_select_t<size_type>;
  constexpr calc_size_type max_size = static_cast<calc_size_type>(std::numeric_limits<size_type>::max());
  calc_size_type minimal_size{nic_sum(static_cast<calc_size_type>(old_size), static_cast<calc_size_type>(new_elements))
  };
  if(minimal_size <= max_size)
    {
    calc_size_type new_size{nic_sum(minimal_size, calc_size_type(minimal_size >> 1u), calc_size_type{1u})};
    return static_cast<size_type>(std::min(max_size, new_size));
    }
  return 0u;
  }

//-------------------------------------------------------------------------------------------------------------------
template<typename vector_type, concepts::unsigned_arithmetic_integral size_type>
  requires has_index_operator<vector_type, size_type>
[[nodiscard]]
inline constexpr auto & at(vector_type & vec, size_type index) noexcept
  {
  assert(index < detail::size(vec));
  return vec[index];
  }

//-------------------------------------------------------------------------------------------------------------------
template<typename vector_type>
concept has_capacity_member_fn = requires(vector_type const & vec) {
  typename vector_type::size_type;
    {
    vec.capacity()
    } -> std::same_as<typename vector_type::size_type>;
};

[[nodiscard]]
inline constexpr auto capacity(has_capacity_member_fn auto const & vec) noexcept
  {
  return vec.capacity();
  }

//-------------------------------------------------------------------------------------------------------------------
template<typename vector_type>
concept has_max_size_member_fn = requires(vector_type const & vec) {
  typename vector_type::size_type;
    {
    vec.max_size()
    } -> std::same_as<typename vector_type::size_type>;
};

[[nodiscard]]
constexpr auto max_size(has_max_size_member_fn auto const & vec) noexcept
  {
  return vec.max_size();
  }

//-------------------------------------------------------------------------------------------------------------------
///\brief returns number of elements that can be inserted without reallocation (if possible)
[[nodiscard]]
inline constexpr auto free_space(auto const & vec) noexcept
  {
  return nic_sub(detail::capacity(vec), detail::size(vec));
  }

//-------------------------------------------------------------------------------------------------------------------
template<typename vector_type>
concept has_data_member_function = requires(vector_type & vec) {
  typename vector_type::value_type;
    {
    vec.data()
    } noexcept -> const_or_nonconst_pointer<typename vector_type::value_type>;
};

template<typename vector_type>
concept has_data_and_size_member_function
  = has_data_member_function<vector_type> && has_size_member_function<vector_type>;

//-------------------------------------------------------------------------------------------------------------------
[[nodiscard]]
inline constexpr auto data(has_data_member_function auto & vec) noexcept
  {
  return vec.data();
  }

//-------------------------------------------------------------------------------------------------------------------
template<typename vector_type>
struct internal_data_context_t
  {
  using value_type = typename vector_type::value_type;
  using size_type = typename vector_type::size_type;
  using iterator = typename vector_type::iterator;
  using const_iterator = typename vector_type::const_iterator;

  storage_context_t<value_type, size_type> ctx;
  size_type size_;

  inline constexpr value_type * data() const noexcept { return ctx.data; }

  inline constexpr value_type const * cdata() const noexcept { return data(); }

  inline constexpr iterator begin() const noexcept { return iterator{data()}; }

  inline constexpr const_iterator cbegin() const noexcept { return const_iterator{data()}; }

  inline constexpr iterator end() const noexcept { return unext(begin(), size_); }

  inline constexpr const_iterator cend() const noexcept { return end(); }

  inline constexpr size_type size() const noexcept { return size_; }

  inline constexpr size_type capacity() const noexcept { return ctx.capacity; }

  inline constexpr size_type free_space() const noexcept { return nic_sub(capacity(), size()); }

  inline explicit constexpr internal_data_context_t(vector_type & vec) noexcept :
      ctx{vec.storage_.context()},
      size_{vec.size()}
    {
    }
  };

template<typename vector_type>
internal_data_context_t(vector_type & vec) -> internal_data_context_t<vector_type>;

//-------------------------------------------------------------------------------------------------------------------
[[nodiscard]]
inline constexpr auto begin(has_data_member_function auto & vec) noexcept
  {
  return detail::data(vec);
  }

//-------------------------------------------------------------------------------------------------------------------
[[nodiscard]]
inline constexpr auto cbegin(has_data_member_function auto const & vec) noexcept
  {
  return detail::begin(vec);
  }

//-------------------------------------------------------------------------------------------------------------------
[[nodiscard]]
inline constexpr auto end(has_data_and_size_member_function auto & vec) noexcept
  {
  return unext(detail::begin(vec), detail::size(vec));
  }

//-------------------------------------------------------------------------------------------------------------------
inline constexpr auto & front(has_data_member_function auto & vec) noexcept { return *detail::data(vec); }

//-------------------------------------------------------------------------------------------------------------------
///\brief Returns a reference to the last element in the container.
///\warning Calling back on an empty container causes undefined behavior.
inline constexpr auto & back(has_data_and_size_member_function auto & vec) noexcept
  {
  return detail::at(vec, nic_sub(detail::size(vec), 1u));
  }

//-------------------------------------------------------------------------------------------------------------------

template<typename vector_type, typename size_type>
concept set_size_constraints = requires(vector_type & vec, size_type pos_ix) {
  typename vector_type::value_type;
  typename vector_type::size_type;
  requires std::same_as<typename vector_type::size_type, size_type>;
  vec.set_size_priv_(pos_ix);
};

template<typename vector_type, typename size_type>
  requires set_size_constraints<vector_type, size_type>
inline constexpr void
  erase_at_end_impl(vector_type & vec, internal_data_context_t<vector_type> const & my, size_type pos_ix) noexcept
  {
  using value_type = typename vector_type::value_type;
  if constexpr(!std::is_trivially_destructible_v<value_type>)
    destroy_range(my.data(), pos_ix, my.size());
  vec.set_size_priv_(pos_ix);
  }

template<typename vector_type, typename size_type>
  requires set_size_constraints<vector_type, size_type>
inline constexpr void erase_at_end_impl(vector_type & vec, size_type pos_ix) noexcept
  {
  detail::erase_at_end_impl(vec, internal_data_context_t{vec}, pos_ix);
  }

//-------------------------------------------------------------------------------------------------------------------
///\brief erases all elements starting at \param pos till end of vector
///       The iterator \ref pos must be valid and dereferenceable.
///\return Iterator following the last removed element equal end of vector
template<typename vector_type>
  requires requires {
    typename vector_type::const_iterator;
    typename vector_type::size_type;
  }
inline constexpr auto erase_at_end(
  vector_type & vec, internal_data_context_t<vector_type> const & my, typename vector_type::const_iterator pos
) noexcept -> typename vector_type::iterator
  {
  using size_type = typename vector_type::size_type;

  size_type pos_ix = udistance<size_type>(my.cbegin(), pos);
  // unsigned cast prevent negative from invalid pos to buffer overflow
  if(pos_ix < my.size())
    {
    detail::erase_at_end_impl(vec, my, pos_ix);
    return unext(my.begin(), pos_ix);
    }
  return my.end();
  }

template<typename vector_type>
  requires requires {
    typename vector_type::const_iterator;
    typename vector_type::size_type;
  }
inline constexpr auto erase_at_end(vector_type & vec, typename vector_type::const_iterator pos) noexcept ->
  typename vector_type::iterator
  {
  internal_data_context_t const my{vec};
  return detail::erase_at_end(vec, my, pos);
  }

///\brief Erases all elements from the container. After this call, size() returns zero.
///       Invalidates any references, pointers, or iterators referring to contained elements.
///       Any past-the-end iterators are also invalidated.
///       Leaves the capacity() of the vector unchanged
template<typename vector_type>
  requires requires { typename vector_type::size_type; }
inline constexpr void clear(vector_type & vec) noexcept
  {
  using size_type = typename vector_type::size_type;
  detail::erase_at_end_impl(vec, size_type(0u));
  }

//-------------------------------------------------------------------------------------------------------------------
template<typename vector_type>
concept vector_with_size_and_value = requires(vector_type & vec) {
  typename vector_type::size_type;
  typename vector_type::value_type;
};

template<typename vector_type>
concept vector_value_move_assignable = requires(vector_type & vec) {
  typename vector_type::const_iterator;
  typename vector_type::iterator;
  requires vector_with_size_and_value<vector_type>;
  requires(true == std::is_move_assignable_v<typename vector_type::value_type>);
  vec.set_size_priv_(typename vector_type::size_type{});
};

///\brief erases element at pos \ref pos
///\returns Iterator following the removed element.
///         If pos refers to the last element, then the end() iterator is returned
template<vector_value_move_assignable vector_type>
inline constexpr auto erase(
  vector_type & vec, typename vector_type::const_iterator pos
) noexcept(std::is_nothrow_move_assignable_v<typename vector_type::value_type>) -> typename vector_type::iterator
  {
  using value_type = typename vector_type::value_type;
  using size_type = typename vector_type::size_type;
  internal_data_context_t const my{vec};

  size_type const pos_ix = udistance<size_type>(my.cbegin(), pos);
  if(pos_ix != my.size())
    {
    size_type const last_ix = nic_sum(pos_ix, 1u);
    if(last_ix != my.size())
      std::move(unext(my.begin(), last_ix), my.end(), unext(my.begin(), pos_ix));
    if constexpr(!std::is_trivially_destructible_v<value_type>)
      std::destroy_at(unext(my.data(), nic_sub(my.size(), 1u)));

    vec.set_size_priv_(nic_sub(my.size(), 1u));
    return unext(my.begin(), pos_ix);
    }
  return my.end();
  }

//-------------------------------------------------------------------------------------------------------------------
///\brief erases elements from range [\ref first \ref last)
///\returns Iterator following the last removed element.
///         If last==end() prior to removal, then the updated end() iterator is returned.
///         If [first, last) is an empty range, then last is returned.
template<vector_value_move_assignable vector_type>
inline constexpr auto erase(
  vector_type & vec, typename vector_type::const_iterator first, typename vector_type::const_iterator last
) noexcept(std::is_nothrow_move_assignable_v<typename vector_type::value_type>) -> typename vector_type::iterator
  {
  using size_type = typename vector_type::size_type;

  internal_data_context_t const my{vec};

  size_type const first_ix = udistance<size_type>(my.cbegin(), first);
  if(first_ix != my.size())
    {
    size_type const last_ix = udistance<size_type>(my.cbegin(), last);
    if(first_ix != last_ix)
      {
      if(last_ix != my.size())
        std::move(unext(my.begin(), last_ix), my.end(), unext(my.begin(), first_ix));
      detail::erase_at_end_impl(vec, nic_sum(nic_sub(my.size(), last_ix), first_ix));
      }
    return unext(my.begin(), first_ix);
    }
  return my.end();
  }

///\brief Removes the last element of the container.
///\warning Calling pop_back on an empty container results in undefined behavior.
template<typename vector_type>
inline constexpr void pop_back(vector_type & vec) noexcept
  {
  internal_data_context_t const my{vec};
  detail::erase_at_end(vec, my, std::prev(my.end(), 1));
  }

//-------------------------------------------------------------------------------------------------------------------

template<concepts::allocate_constraint value_type, typename size_type>
[[nodiscard, gnu::always_inline]]
inline constexpr storage_context_t<value_type, size_type> sv_allocate(size_type capacity) noexcept
  {
  using storage_type = storage_context_t<value_type, size_type>;
  if(std::is_constant_evaluated())
    return storage_type{std::allocator<value_type>{}.allocate(capacity), capacity};
  else
    {
    std::align_val_t alignment{alignof(value_type)};
    size_t alloc_size{capacity * sizeof(value_type)};
    return storage_type
      {
      static_cast<value_type *>(
// https://clang.llvm.org/docs/LanguageExtensions.html#builtin-operator-new-and-builtin-operator-delete
#if defined(__has_builtin) && __has_builtin(__builtin_operator_new) >= 201802L
        __builtin_operator_new(alloc_size, alignment, std::nothrow_t{})
      ),
#else
        ::operator new(alloc_size, alignment, std::nothrow_t{})
      ),
#endif
        capacity
      // clang-format off
      };
    // clang-format on
    }
  }

template<typename value_type, typename size_type>
inline constexpr void sv_deallocate(storage_context_t<value_type, size_type> storage) noexcept
  {
  if(std::is_constant_evaluated())
    std::allocator<value_type>{}.deallocate(storage.data, storage.capacity);
  else
    {
    std::align_val_t alignment{alignof(value_type)};
#if defined(__has_builtin) && __has_builtin(__builtin_operator_new) >= 201802L
    __builtin_operator_delete(storage.data, /*storage.capacity * sizeof(value_type),*/ alignment, std::nothrow_t{});
#else
    ::operator delete(storage.data, /*storage.capacity * sizeof(value_type),*/ alignment, std::nothrow_t{});
#endif
    }
  }

//-------------------------------------------------------------------------------------------------------------------
template<typename vector_type, typename... Args>
inline constexpr void emplace_back_unchecked(
  vector_type & vec, internal_data_context_t<vector_type> const & my, Args &&... args
) noexcept(std::is_nothrow_constructible_v<typename vector_type::value_type, Args...>)
  {
  std::construct_at(std::addressof(*my.end()), std::forward<Args>(args)...);
  vec.set_size_priv_(nic_sum(my.size(), 1u));
  }

template<typename vector_type, typename... Args>
  requires requires(vector_type & vec) {
    requires vector_with_size_and_value<vector_type>;
    requires std::constructible_from<typename vector_type::value_type, Args...>;
    vec.set_size_priv_(typename vector_type::size_type{});
  }
inline constexpr void emplace_back_unchecked(
  vector_type & vec, Args &&... args
) noexcept(std::is_nothrow_constructible_v<typename vector_type::value_type, Args...>)
  {
  internal_data_context_t const my{vec};
  detail::emplace_back_unchecked(vec, my, std::forward<Args>(args)...);
  }

//-------------------------------------------------------------------------------------------------------------------
///\brief Appends a new element to the end of the container
///       meets strong exception guarantee
template<typename vector_type, typename... Args>
  requires requires(vector_type & vec) {
    requires vector_with_size_and_value<vector_type>;
      {
      vector_type::support_reallocation()
      } -> std::same_as<bool>;
      requires std::constructible_from<typename vector_type::value_type, Args...>;
  }
inline constexpr vector_outcome_e emplace_back(
  vector_type & vec, Args &&... args
) noexcept(concepts::is_nothrow_move_constr_and_constr_v<typename vector_type::value_type, Args...>)
  {
  constexpr bool use_nothrow = concepts::is_nothrow_move_constr_and_constr_v<typename vector_type::value_type, Args...>;
  internal_data_context_t const my{vec};

  if(my.size() < my.capacity())
    {
    detail::emplace_back_unchecked(vec, my, std::forward<Args>(args)...);
    return vector_outcome_e::no_error;
    }
  else
    {
    if constexpr(vector_type::support_reallocation())
      {
      using size_type = typename vector_type::size_type;
      size_type const new_capacity{growth(my.size(), size_type(1u))};
      if(new_capacity != 0u)
        {
        using value_type = typename vector_type::value_type;
        // alocate new space with growth factor, reclaim space in case of throwing at !use_nothrow
        typename noexcept_if<use_nothrow>::cond_except_holder new_space{sv_allocate<value_type>(new_capacity)};
        if(new_space)
          {
          // relocate elements
          if constexpr(use_nothrow)
            // remains only for purprose of better data access order
            {
            uninitialized_relocate_n(my.data(), my.size(), new_space.data());
            // construct new element
            std::construct_at(unext(new_space.data(), my.size()), std::forward<Args>(args)...);
            }
          else
            {
            // construct new element, if the second part can be noexcept relocated then this doesn't need raii unwind
            typename noexcept_if<std::is_nothrow_move_constructible_v<value_type>>::cond_destroy_at el{
              std::construct_at(unext(new_space.data(), my.size()), std::forward<Args>(args)...)
            };
            // if only constructor throw use relocate for elements
            if constexpr(std::is_nothrow_move_constructible_v<value_type>)
              uninitialized_relocate_n(my.data(), my.size(), new_space.data());
            else
              uninitialized_relocate_with_copy_n(my.data(), my.size(), new_space.data());
            // dont destroy if no exception is thrown with uninitialized_relocate_with_copy_n
            // for std::is_nothrow_move_constructible_v<value_type> is no op
            el.release();
            }
          // deallocate old space
          storage_context_t old_storage{vec.exchange_priv_(new_space.release(), nic_sum(my.size(), 1u))};
          if(old_storage.data != nullptr)
            sv_deallocate(old_storage);
          return vector_outcome_e::no_error;
          }
        }
      }
    return vector_outcome_e::out_of_storage;
    }
  }

//-------------------------------------------------------------------------------------------------------------------
template<typename vector_type, typename T>
  requires requires {
    typename vector_type::value_type;
    requires std::constructible_from<typename vector_type::value_type, T>;
  }
inline constexpr auto
  push_back(vector_type & vec, T && value) noexcept(noexcept(emplace_back(vec, std::forward<T>(value))))
  {
  return detail::emplace_back(vec, std::forward<T>(value));
  }

//-------------------------------------------------------------------------------------------------------------------
///\brief Inserts elements at the specified location in the container.
///       for efficienty of determining the count of inserting objects
///       input range is limited to random access iterator
///       for not nothrow move constructible elements if insert is done at end() meets strong exception guarantee,
///       otherwise meets basic exception guarantee
///\warning if there is no space left for inserting all elements it returns \ref
/// static_vector_outcome_e::out_of_storage
template<typename vector_type, concepts::random_access_iterator source_iterator>
  requires requires(vector_type & vec) {
    requires vector_with_size_and_value<vector_type>;
      {
      vector_type::support_reallocation()
      } -> std::same_as<bool>;
    // value must be constructible from *iterator
    requires std::constructible_from<typename vector_type::value_type, detail::iterator_value_type_t<source_iterator>>;
    vec.set_size_priv_(typename vector_type::size_type{});
  }
inline constexpr vector_outcome_e insert(
  vector_type & vec, typename vector_type::const_iterator citpos, source_iterator itbeg, source_iterator itend
) noexcept(concepts::
             is_nothrow_move_constr_and_constr_v<
               typename vector_type::value_type,
               detail::iterator_value_type_t<source_iterator>>)
  {
  constexpr bool use_nothrow = concepts::is_nothrow_move_constr_and_constr_v<
    typename vector_type::value_type,
    detail::iterator_value_type_t<source_iterator>>;

  using size_type = typename vector_type::size_type;
  auto const new_el_count{std::distance(itbeg, itend)};

  if(new_el_count == 0) [[unlikely]]
    return vector_outcome_e::no_error;

  if(new_el_count < 0) [[unlikely]]
    {
    if(!std::is_constant_evaluated())
      assert(false);
    return vector_outcome_e::invalid_source_range;
    }

  size_type const u_new_el_count{static_cast<size_type>(new_el_count)};

  internal_data_context_t const my{vec};
  auto itpos{unext(my.begin(), udistance<size_type>(my.cbegin(), citpos))};
  // comparing unsigned cast of new_el_count prevents working with negative range in itbeg, itend
  if(u_new_el_count <= my.free_space())
    {
    size_type const old_el_count{udistance<size_type>(itpos, my.end())};
    if(u_new_el_count >= old_el_count)
      {
      // move elements to uninitialized space
      //  item1 item1
      //  item2 item2
      //>item3 nel1
      //  item4 nel2
      //  item5 nel3
      //  free  nel4
      //  free >item3
      //  free  item4
      //  free  item5
      auto it_part = unext(itbeg, old_el_count);
      if(u_new_el_count > old_el_count)
        {
        // remaining part in uninitialized space
        size_type const low_ext_part{udistance<size_type>(it_part, itend)};
        uninitialized_copy_n(it_part, low_ext_part, unext(itpos, old_el_count));
        vec.set_size_priv_(nic_sum(my.size(), low_ext_part));
        }
      if(old_el_count != 0)
        {
        uninitialized_move_n(itpos, old_el_count, unext(itpos, u_new_el_count));
        vec.set_size_priv_(nic_sum(my.size(), u_new_el_count));
        // move part in initialized space
        std::copy(itbeg, it_part, itpos);
        }
      else
        vec.set_size_priv_(nic_sum(my.size(), u_new_el_count));
      }
    else
      {
      size_type const uninit_move_pos = nic_sub(old_el_count, u_new_el_count);
      // item1  item1
      // item2  item2
      //>item3  nel1
      // item4  nel2
      // item5 >item3
      // item6  item4
      // free   item5
      // free   item6
      // free   free
      uninitialized_move_n(unext(itpos, uninit_move_pos), u_new_el_count, unext(itpos, old_el_count));
      vec.set_size_priv_(nic_sum(my.size(), u_new_el_count));
      std::move_backward(itpos, unext(itpos, uninit_move_pos), unext(itpos, nic_sum(u_new_el_count, uninit_move_pos)));
      std::copy(itbeg, itend, itpos);
      }
    return vector_outcome_e::no_error;
    }
  else if(new_el_count > 0)
    {
    if constexpr(vector_type::support_reallocation())
      {
      size_type const new_capacity{growth(my.size(), u_new_el_count)};
      if(new_capacity != 0u)
        {
        using value_type = typename vector_type::value_type;
        // alocate new space with growth factor, reclaim space in case of throwing at !use_nothrow
        typename noexcept_if<use_nothrow>::cond_except_holder new_space{sv_allocate<value_type>(new_capacity)};
        if(new_space)
          {
          size_type lower_el_count{udistance<size_type>(my.begin(), itpos)};

          // insert new elements
          uninitialized_copy_n(itbeg, u_new_el_count, unext(new_space.data(), lower_el_count));
          typename noexcept_if<std::is_nothrow_move_constructible_v<value_type>>::cond_destroy_range ext_range_unwind{
            new_space.data(), lower_el_count, nic_sum(lower_el_count, u_new_el_count)
          };

          if constexpr(std::is_nothrow_move_constructible_v<value_type>)
            {
            // relocate lower part
            uninitialized_relocate_n(my.begin(), lower_el_count, new_space.data());
            // relocate upper part, last loop unwinds if throws by itself
            uninitialized_relocate_n(
              unext(my.begin(), lower_el_count),
              nic_sub(my.size(), lower_el_count),
              unext(new_space.data(), nic_sum(lower_el_count, u_new_el_count))
            );
            }
          else
            {
            uninitialized_copy_n(my.begin(), lower_el_count, new_space.data());
            typename noexcept_if<std::is_nothrow_move_constructible_v<value_type>>::cond_destroy_range
              lower_part_unwind{new_space.data(), size_type{0u}, lower_el_count};
            // cond_destroy_range_at
            // relocate upper part, last loop unwinds if throws by itself
            uninitialized_relocate_with_copy_n(
              unext(my.begin(), lower_el_count),
              nic_sub(my.size(), lower_el_count),
              unext(new_space.data(), nic_sum(lower_el_count, u_new_el_count))
            );
            // finish moving lower part
            destroy_range(my.begin(), size_type{0u}, lower_el_count);
            lower_part_unwind.release();
            }
          ext_range_unwind.release();

          // deallocate old space
          storage_context_t old_storage{vec.exchange_priv_(new_space.release(), nic_sum(my.size(), u_new_el_count))};
          if(old_storage.data != nullptr)
            sv_deallocate(old_storage);
          return vector_outcome_e::no_error;
          }
        }
      }
    return vector_outcome_e::out_of_storage;
    }
  else
    return vector_outcome_e::invalid_source_range;
  }

//-------------------------------------------------------------------------------------------------------------------
template<typename vector_type, typename... Args>
concept emplace_constraints = requires(vector_type & vec) {
  requires vector_with_size_and_value<vector_type>;
  requires std::constructible_from<typename vector_type::value_type, Args...>;
  vec.set_size_priv_(typename vector_type::size_type{});
};

//-------------------------------------------------------------------------------------------------------------------
template<typename vector_type, typename... Args>
  requires emplace_constraints<vector_type, Args...>
inline constexpr void emplace_unchecked_impl(
  vector_type & vec,
  internal_data_context_t<vector_type> const & my,
  typename vector_type::const_iterator citpos,
  Args &&... args
) noexcept(concepts::is_nothrow_move_constr_and_constr_v<typename vector_type::value_type, Args...>)
  {
  using value_type = typename vector_type::value_type;
  using size_type = typename vector_type::size_type;
  // item1  item1
  // item2  item2
  //>item3  nel1
  // item4 >item3
  // item5  item4
  // item6  item5
  // free   item6
  // free   free
  // free   free

  auto itpos{unext(my.begin(), udistance<size_type>(my.cbegin(), citpos))};
  size_type const old_el_count{udistance<size_type>(itpos, my.end())};
  if(old_el_count != 0)
    {
    size_type uninit_move_pos = nic_sub(old_el_count, 1u);
    detail::uninitialized_move_if_noexcept_n(unext(itpos, uninit_move_pos), 1u, unext(itpos, old_el_count));
    // change size after new element is moved to uninitialized space
    vec.set_size_priv_(nic_sum(my.size(), 1u));
    std::move_backward(itpos, unext(itpos, uninit_move_pos), unext(itpos, nic_sum(uninit_move_pos, 1u)));
    *itpos = value_type{std::forward<Args>(args)...};
    }
  else
    {
    std::construct_at(std::addressof(*my.end()), std::forward<Args>(args)...);
    vec.set_size_priv_(nic_sum(my.size(), 1u));
    }
  }

template<typename vector_type, typename... Args>
  requires emplace_constraints<vector_type, Args...>
inline constexpr void emplace_unchecked(
  vector_type & vec, typename vector_type::const_iterator citpos, Args &&... args
) noexcept(concepts::is_nothrow_move_constr_and_constr_v<typename vector_type::value_type, Args...>)
  {
  internal_data_context_t const my{vec};
  emplace_unchecked_impl(vec, my, citpos, std::forward<Args>(args)...);
  }

//-------------------------------------------------------------------------------------------------------------------
template<typename vector_type, typename... Args>
  requires emplace_constraints<vector_type, Args...>
inline constexpr vector_outcome_e emplace(
  vector_type & vec, typename vector_type::const_iterator citpos, Args &&... args
) noexcept(concepts::is_nothrow_move_constr_and_constr_v<typename vector_type::value_type, Args...>)
  {
  constexpr bool use_nothrow = concepts::is_nothrow_move_constr_and_constr_v<typename vector_type::value_type, Args...>;

  using size_type = typename vector_type::size_type;
  internal_data_context_t const my{vec};
  auto itpos{unext(my.begin(), udistance<size_type>(my.cbegin(), citpos))};

  if(my.free_space() != 0u)
    {
    emplace_unchecked_impl(vec, my, itpos, std::forward<Args>(args)...);
    return vector_outcome_e::no_error;
    }
  else if constexpr(vector_type::support_reallocation())
    {
    size_type const new_capacity{growth(my.size(), size_type(1u))};
    if(new_capacity != 0u)
      {
      using value_type = typename vector_type::value_type;
      // alocate new space with growth factor, reclaim space in case of throwing at !use_nothrow
      typename noexcept_if<use_nothrow>::cond_except_holder new_space{sv_allocate<value_type>(new_capacity)};
      if(new_space)
        {
        size_type const lower_el_count{udistance<size_type>(my.begin(), itpos)};
          {
          typename noexcept_if<std::is_nothrow_move_constructible_v<value_type>>::cond_destroy_at el{
            std::construct_at(unext(new_space.data(), lower_el_count), std::forward<Args>(args)...)
          };

          if constexpr(std::is_nothrow_move_constructible_v<value_type>)
            {
            uninitialized_relocate_n(my.begin(), lower_el_count, new_space.data());
            uninitialized_relocate_n(
              unext(my.begin(), lower_el_count),
              nic_sub(my.size(), lower_el_count),
              unext(new_space.data(), nic_sum(lower_el_count, 1u))
            );
            }
          else
            {
            // copy lower part
            uninitialized_copy_n(my.data(), lower_el_count, new_space.data());
            // unwind reclamation of copied instances
            typename noexcept_if<std::is_nothrow_move_constructible_v<value_type>>::cond_destroy_range
              lower_part_unwind{new_space.data(), size_type{0u}, lower_el_count};

            uninitialized_relocate_with_copy_n(
              unext(my.begin(), lower_el_count),
              nic_sub(my.size(), lower_el_count),
              unext(new_space.data(), nic_sum(lower_el_count, 1u))
            );
            // finish moving lower part
            if constexpr(not std::is_trivially_destructible_v<value_type>)
              destroy_range(my.begin(), size_type{0u}, lower_el_count);

            lower_part_unwind.release();
            }
          el.release();
          }
        // deallocate old space
        storage_context_t old_storage{vec.exchange_priv_(new_space.release(), nic_sum(my.size(), 1u))};
        if(old_storage.data != nullptr)
          sv_deallocate(old_storage);
        return vector_outcome_e::no_error;
        }
      }
    }
  return vector_outcome_e::out_of_storage;
  }

//-------------------------------------------------------------------------------------------------------------------
template<typename vector_type>
concept reserve_constraints = requires {
  requires vector_with_size_and_value<vector_type>;
  requires(true == std::is_move_constructible_v<typename vector_type::value_type>);
    {
    vector_type::support_reallocation()
    } -> std::same_as<bool>;
    requires(true == vector_type::support_reallocation());
};

//-------------------------------------------------------------------------------------------------------------------
template<typename vector_type>
  requires reserve_constraints<vector_type>
constexpr vector_outcome_e relocate_elements_dyn(
  vector_type & vec, internal_data_context_t<vector_type> const & my, typename vector_type::size_type new_capacity
) noexcept(std::is_nothrow_move_constructible_v<typename vector_type::value_type>)
  {
  constexpr bool use_nothrow = std::is_nothrow_move_constructible_v<typename vector_type::value_type>;
  using value_type = typename vector_type::value_type;
  // allocate new space with growth factor, reclaim space in case of throwing at !use_nothrow
  typename noexcept_if<use_nothrow>::cond_except_holder new_space{sv_allocate<value_type>(new_capacity)};
  if(new_space)
    {
    if constexpr(use_nothrow)
      uninitialized_relocate_n(my.begin(), my.size(), new_space.data());
    else
      uninitialized_relocate_with_copy_n(my.begin(), my.size(), new_space.data());
    // deallocate old space
    storage_context_t old_storage{vec.exchange_priv_(new_space.release(), my.size())};
    if(old_storage.data != nullptr)
      sv_deallocate(old_storage);
    return vector_outcome_e::no_error;
    }
  return vector_outcome_e::out_of_storage;
  }

//-------------------------------------------------------------------------------------------------------------------
template<typename vector_type>
  requires reserve_constraints<vector_type>
constexpr vector_outcome_e relocate_elements_static(
  vector_type & vec, internal_data_context_t<vector_type> const & my
) noexcept(std::is_nothrow_move_constructible_v<typename vector_type::value_type>)
  {
  constexpr bool use_nothrow = std::is_nothrow_move_constructible_v<typename vector_type::value_type>;
  typename noexcept_if<use_nothrow>::cond_except_holder_revert old_storage{vec, my.size(), vec.switch_static_priv_()};
  if constexpr(use_nothrow)
    uninitialized_relocate_n(my.begin(), my.size(), vec.begin());
  else
    uninitialized_relocate_with_copy_n(my.begin(), my.size(), vec.begin());
  vec.set_size_priv_(my.size());
  sv_deallocate(old_storage.release());
  return vector_outcome_e::no_error;
  }

//-------------------------------------------------------------------------------------------------------------------
///\brief Increase the capacity of the vector to a value that's greater or equal to new_cap.
///       If new_cap is greater than the current capacity(), new storage is allocated, otherwise the function does
///       nothing.
template<typename vector_type>
  requires reserve_constraints<vector_type>
constexpr vector_outcome_e reserve(
  vector_type & vec, typename vector_type::size_type new_capacity
) noexcept(std::is_nothrow_move_constructible_v<typename vector_type::value_type>)
  {
  if(new_capacity < max_size(vec))
    {
    internal_data_context_t const my{vec};
    if(new_capacity > my.capacity())
      return relocate_elements_dyn(vec, my, new_capacity);
    else
      return vector_outcome_e::no_error;
    }
  return vector_outcome_e::out_of_storage;
  }

//-------------------------------------------------------------------------------------------------------------------
template<typename vector_type>
concept vector_with_move_and_default_constructible_value_type = requires {
  requires vector_with_size_and_value<vector_type>;
  requires(true == std::is_move_constructible_v<typename vector_type::value_type>);
  requires(true == std::is_default_constructible_v<typename vector_type::value_type>);
    {
    vector_type::support_reallocation()
    } -> std::same_as<bool>;
};

template<vector_with_move_and_default_constructible_value_type vector_type>
inline constexpr vector_outcome_e default_append(
  vector_type & vec, internal_data_context_t<vector_type> const & my, typename vector_type::size_type count
) noexcept(std::is_nothrow_move_constructible_v<typename vector_type::value_type> && std::is_nothrow_default_constructible_v<typename vector_type::value_type>)
  {
  using value_type = typename vector_type::value_type;
  using size_type = typename vector_type::size_type;

  constexpr bool use_nothrow
    = std::is_nothrow_move_constructible_v<value_type> && std::is_nothrow_default_constructible_v<value_type>;

  if(count <= my.free_space())
    {
    uninitialized_value_construct_n(my.end(), count);
    vec.set_size_priv_(nic_sum(my.size(), count));
    return vector_outcome_e::no_error;
    }
  else if constexpr(vector_type::support_reallocation())
    {
    size_type new_capacity{growth(my.size(), count)};

    // allocate new space with growth factor, reclaim space in case of throwing at !use_nothrow
    typename noexcept_if<use_nothrow>::cond_except_holder new_space{sv_allocate<value_type>(new_capacity)};
    if(new_space)
      {
      if constexpr(use_nothrow)
        {
        // exists only of the purpose of better memory access order
        uninitialized_relocate_n(my.begin(), my.size(), new_space.data());
        uninitialized_value_construct_n(unext(new_space.data(), my.size()), count);
        }
      else
        {
        uninitialized_value_construct_n(unext(new_space.data(), my.size()), count);
        // remember new objects constructed for destroy_range if append fails
        typename noexcept_if<std::is_nothrow_move_constructible_v<value_type>>::cond_destroy_range new_elems_unwind{
          new_space.data(), my.size(), nic_sum(my.size(), count)
        };

        if constexpr(std::is_nothrow_move_constructible_v<value_type>)
          uninitialized_relocate_n(my.begin(), my.size(), new_space.data());
        else
          uninitialized_relocate_with_copy_n(my.begin(), my.size(), new_space.data());

        new_elems_unwind.release();
        }
      // deallocate old space
      storage_context_t old_storage{vec.exchange_priv_(new_space.release(), nic_sum(my.size(), count))};
      if(old_storage.data != nullptr)
        sv_deallocate(old_storage);
      return vector_outcome_e::no_error;
      }
    }
  return vector_outcome_e::out_of_storage;
  }

template<vector_with_move_and_default_constructible_value_type vector_type>
inline constexpr vector_outcome_e default_append(
  vector_type & vec, typename vector_type::size_type count
) noexcept(std::is_nothrow_move_constructible_v<typename vector_type::value_type> && std::is_nothrow_default_constructible_v<typename vector_type::value_type>)
  {
  internal_data_context_t my{vec};
  return default_append(vec, my, count);
  }

//-------------------------------------------------------------------------------------------------------------------
template<vector_with_move_and_default_constructible_value_type vector_type>
constexpr vector_outcome_e resize(
  vector_type & vec, typename vector_type::size_type new_size
) noexcept(std::is_nothrow_move_constructible_v<typename vector_type::value_type> && std::is_nothrow_default_constructible_v<typename vector_type::value_type>)
  {
  internal_data_context_t const my{vec};

  if(my.size() != new_size)
    {
    if(new_size <= max_size(vec))
      {
      if(my.size() < new_size)
        return default_append(vec, my, nic_sub(new_size, my.size()));
      else
        {
        detail::erase_at_end(vec, unext(my.begin(), new_size));
        return vector_outcome_e::no_error;
        }
      }
    else
      return vector_outcome_e::out_of_storage;
    }
  else
    return vector_outcome_e::no_error;
  }

//-------------------------------------------------------------------------------------------------------------------
template<typename vector_type>
  requires reserve_constraints<vector_type>
constexpr vector_outcome_e shrink_to_fit(vector_type & vec
) noexcept(std::is_nothrow_move_constructible_v<typename vector_type::value_type>)
  {
  internal_data_context_t const my{vec};
  if constexpr(vector_type::buffered_capacity() != 0)
    {
    if(my.free_space() != 0 && my.capacity() > vector_type::buffered_capacity())
      {
      if(my.size() > vector_type::buffered_capacity())
        return relocate_elements_dyn(vec, my, my.size());
      else
        return relocate_elements_static(vec, my);
      }
    }
  else
    {
    if(my.free_space() != 0)
      return relocate_elements_dyn(vec, my, my.size());
    }
  return vector_outcome_e::no_error;
  }
  }  // namespace small_vectors::inline v3_0::detail

