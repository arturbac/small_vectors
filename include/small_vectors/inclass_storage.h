// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT
// SPDX-PackageHomePage: https://github.com/arturbac/small_vectors
#pragma once

#include <cstddef>
#include <new>
#include <utility>
#include <concepts>
#include <memory>
#include <concepts>
#include <type_traits>

namespace small_vectors::inline v3_2
  {
namespace concepts
  {
  template<typename T>
  concept complete_type = requires { sizeof(T); };

  template<typename T>
  concept same_as_inclass_storage = requires {
    // clang-format off
    typename T::value_type;
    { T::storage_size } -> std::convertible_to<std::size_t>;
    { T::alignment } -> std::convertible_to<std::size_t>;
    requires sizeof(typename T::value_type) == sizeof(std::byte[T::storage_size]) && alignof(typename T::value_type) == T::alignment;
      // clang-format on
  };
  }  // namespace concepts

template<typename ValueType, std::size_t StorageSize, std::size_t Alignment>
struct inclass_storage_t
  {
  using value_type = ValueType;
  static constexpr std::size_t storage_size{StorageSize};
  static constexpr std::size_t alignment{Alignment};

  alignas(alignment) std::byte data[storage_size];
  };

namespace inclass_storage
  {
  template<concepts::same_as_inclass_storage storage_type>
  constexpr auto ptr(storage_type & store) noexcept -> typename storage_type::value_type *
    {
    return std::launder(reinterpret_cast<typename storage_type::value_type *>(&store.data[0]));
    }

  template<concepts::same_as_inclass_storage storage_type>
  constexpr auto ptr(storage_type const & store) noexcept -> typename storage_type::value_type const *
    {
    return std::launder(reinterpret_cast<typename storage_type::value_type const *>(&store.data[0]));
    }

  template<concepts::same_as_inclass_storage storage_type>
    requires concepts::complete_type<typename storage_type::value_type>
             && std::default_initializable<typename storage_type::value_type>
  constexpr auto
    default_construct() noexcept(std::is_nothrow_default_constructible_v<typename storage_type::value_type>)
      -> storage_type

    {
    storage_type storage{};
    if constexpr(!std::is_trivially_default_constructible_v<typename storage_type::value_type>)
      std::construct_at(ptr(storage));

    return storage;
    }

  template<concepts::same_as_inclass_storage storage_type>
    requires std::destructible<typename storage_type::value_type>
  constexpr void destroy(storage_type & storage
  ) noexcept(std::is_nothrow_destructible_v<typename storage_type::value_type>)
    {
    if constexpr(!std::is_trivially_constructible_v<typename storage_type::value_type>)
      std::destroy_at(ptr(storage));
    }

  template<concepts::same_as_inclass_storage storage_type, typename... Args>
    requires concepts::complete_type<typename storage_type::value_type>
             && std::constructible_from<typename storage_type::value_type, Args &&...>
  constexpr auto construct_from(Args &&... args) -> storage_type
    {
    storage_type storage{};
    std::construct_at(ptr(storage), std::forward<Args>(args)...);
    return storage;
    }

  template<concepts::same_as_inclass_storage storage_type>
    requires std::copy_constructible<typename storage_type::value_type>
  constexpr auto copy_construct(storage_type const & other
  ) noexcept(std::is_nothrow_copy_constructible_v<typename storage_type::value_type>) -> storage_type
    {
    if constexpr(std::is_trivially_copy_constructible_v<typename storage_type::value_type>)
      return storage_type{other};
    else
      {
      storage_type storage{};
      std::construct_at(ptr(storage), *ptr(other));
      return storage;
      }
    }

  template<concepts::same_as_inclass_storage storage_type>
    requires concepts::complete_type<typename storage_type::value_type>
             && std::move_constructible<typename storage_type::value_type>
  constexpr auto move_construct(storage_type && other
  ) noexcept(std::is_nothrow_move_constructible_v<typename storage_type::value_type>) -> storage_type
    {
    if constexpr(std::is_trivially_move_constructible_v<typename storage_type::value_type>)
      return storage_type{other};
    else
      {
      storage_type storage{};
      std::construct_at(ptr(storage), std::move(*ptr(other)));
      return storage;
      }
    }

  template<concepts::same_as_inclass_storage storage_type>
    requires concepts::complete_type<typename storage_type::value_type>
             && std::copyable<typename storage_type::value_type>
  constexpr void copy_assign(
    storage_type & that, storage_type const & other
  ) noexcept(std::is_nothrow_copy_assignable_v<typename storage_type::value_type>)
    {
    if(&that != &other)
      {
      if constexpr(std::is_trivially_copy_assignable_v<typename storage_type::value_type>)
        that = other;
      else
        *ptr(that) = *ptr(other);
      }
    }

  template<concepts::same_as_inclass_storage storage_type>
    requires concepts::complete_type<typename storage_type::value_type>
             && std::movable<typename storage_type::value_type>
  constexpr void move_assign(
    storage_type & that, storage_type && other
  ) noexcept(std::is_nothrow_move_assignable_v<typename storage_type::value_type>)
    {
    if(&that != &other)
      {
      if constexpr(std::is_trivially_move_assignable_v<typename storage_type::value_type>)
        that = other;
      else
        *ptr(that) = std::move(*ptr(other));
      }
    }

  }  // namespace inclass_storage

template<
  typename ValueType,
  std::size_t StorageSize = sizeof(ValueType),
  std::size_t Alignment = std::alignment_of_v<ValueType>>
struct inclass_store_t
  {
  using value_type = ValueType;
  static constexpr std::size_t storage_size{StorageSize};

  struct store_t
    {
    alignas(Alignment) std::byte data[storage_size];
    };

  store_t store_;

private:
  constexpr auto ptr() noexcept -> value_type *
    {
    return std::launder(reinterpret_cast<value_type *>(&store_.data[0]));
    }

  constexpr auto ptr() const noexcept -> value_type const *
    {
    return std::launder(reinterpret_cast<value_type const *>(&store_.data[0]));
    }

public:
  constexpr inclass_store_t() noexcept(std::is_nothrow_default_constructible_v<value_type>)
    requires concepts::complete_type<value_type> && std::default_initializable<value_type>
    {
    if constexpr(std::is_trivially_default_constructible_v<value_type>)
      store_ = {};
    else
      std::construct_at(ptr());
    }

  constexpr inclass_store_t(inclass_store_t const & other) noexcept(std::is_nothrow_copy_constructible_v<value_type>)
    requires std::copy_constructible<value_type>
    {
    if constexpr(std::is_trivially_copy_constructible_v<value_type>)
      store_ = other.store_;
    else
      std::construct_at(ptr(), *other.ptr());
    }

  constexpr inclass_store_t(inclass_store_t && other) noexcept(std::is_nothrow_move_constructible_v<value_type>)
    requires concepts::complete_type<value_type> && std::move_constructible<value_type>
    {
    if constexpr(std::is_trivially_move_constructible_v<value_type>)
      store_ = other.store_;
    else
      std::construct_at(ptr(), std::move(*other.ptr()));
    }

  constexpr auto operator=(inclass_store_t const & other) noexcept(std::is_nothrow_copy_assignable_v<value_type>)
    -> inclass_store_t &
    requires concepts::complete_type<value_type> && std::copyable<value_type>  // Requires value_type to be copyable
    {
    if(this != &other)
      {
      if constexpr(std::is_trivially_copy_assignable_v<value_type>)
        store_ = other.store_;
      else
        *ptr() = *other.ptr();
      }
    return *this;
    }

  constexpr auto operator=(inclass_store_t && other) noexcept(std::is_nothrow_move_assignable_v<value_type>)
    -> inclass_store_t &
    requires concepts::complete_type<value_type> && std::movable<value_type>  // Requires value_type to be movable
    {
    if(this != &other)
      {
      if constexpr(not std::is_trivially_destructible_v<value_type>)
        std::destroy_at(ptr());
      if constexpr(std::is_trivially_move_assignable_v<value_type>)
        store_ = other.store_;
      else
        std::construct_at(ptr(), std::move(*other.ptr()));
      }
    return *this;
    }

  template<typename... Args>
  constexpr inclass_store_t(Args &&... args)
    requires concepts::complete_type<value_type> && std::constructible_from<value_type, Args &&...>
    {
    std::construct_at(ptr(), std::forward<Args>(args)...);
    }

  constexpr ~inclass_store_t() noexcept(std::is_nothrow_destructible_v<value_type>)
    requires std::destructible<value_type> and (not std::is_trivially_destructible_v<value_type>)
    {
    std::destroy_at(ptr());
    }

  constexpr ~inclass_store_t() noexcept
    requires std::is_trivially_destructible_v<value_type>
  = default;

  constexpr auto operator*() noexcept -> value_type & { return *ptr(); }

  constexpr auto operator*() const noexcept -> value_type const & { return *ptr(); }

  constexpr auto operator->() noexcept -> value_type * { return ptr(); }

  constexpr auto operator->() const noexcept -> value_type const * { return ptr(); }
  };

  }  // namespace small_vectors::inline v3_2
