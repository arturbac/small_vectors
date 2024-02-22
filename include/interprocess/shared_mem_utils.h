#pragma once
#include <type_traits>
#include <concepts>
#include <cstdint>

namespace ip
  {
namespace detail
  {
  template<typename type>
  concept trivially_copyable = std::is_trivially_copyable_v<type>;

  template<typename Type, std::size_t offset>
  concept concept_aligned_offset = requires { requires(offset % alignof(Type)) == 0; };

  template<typename type>
  struct end_offset_of
    {
    static constexpr std::size_t value = type::end_offset;
    };

  template<>
  struct end_offset_of<void>
    {
    static constexpr std::size_t value = 0;
    };
  }  // namespace detail

template<typename Type, typename prev_type = void>
  requires detail::concept_aligned_offset<Type, detail::end_offset_of<prev_type>::value>
struct shared_type_decl
  {
  using type = Type;
  static constexpr std::size_t offset = detail::end_offset_of<prev_type>::value;
  static constexpr std::size_t end_offset = offset + sizeof(type);
  };

namespace detail
  {
  template<typename shared_type_decl_type>
  concept concept_type_decl = requires {
    typename shared_type_decl_type::type;
    shared_type_decl_type::offset;
    shared_type_decl_type::end_offset;
  };
  template<typename type>
  concept concept_pointer = std::is_pointer_v<type>;

  template<typename type>
  concept concept_mapped_region = requires(type & region) {
    region.get_address();
      {
      region.get_address()
      } -> concept_pointer;
  };
  }  // namespace detail

template<detail::concept_type_decl shared_type_decl_type, typename... Args>
inline auto construct_at(detail::concept_mapped_region auto & region, Args &&... args) noexcept ->
  typename shared_type_decl_type::type *
  {
  using type = typename shared_type_decl_type::type;
  constexpr auto offset{static_cast<ptrdiff_t>(shared_type_decl_type::offset)};
  auto addr{std::next(reinterpret_cast<uint8_t *>(region.get_address()), offset)};
  return std::construct_at(reinterpret_cast<type *>(addr), args...);
  }

template<detail::concept_type_decl shared_type_decl_type>
[[nodiscard]]
inline auto ref(detail::concept_mapped_region auto & region) noexcept -> typename shared_type_decl_type::type &
  {
  using type = typename shared_type_decl_type::type;
  constexpr auto offset{static_cast<ptrdiff_t>(shared_type_decl_type::offset)};
  auto addr{std::next(reinterpret_cast<uint8_t *>(region.get_address()), offset)};
  return *std::launder(reinterpret_cast<type *>(addr));
  }

  }  // namespace ip
