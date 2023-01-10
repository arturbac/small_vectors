#pragma once
#include <type_traits>
#include <concepts>

namespace ip
{
template<typename type>
concept trivially_copyable = std::is_trivially_copyable_v<type>;

template<typename Type, std::size_t offset>
concept concept_aligned_offset = requires
{
 (offset % alignof(Type)) == 0;
};

template<typename Type, std::size_t Offset>
  requires concept_aligned_offset<Type,Offset>
struct shared_type_decl
  {
  using type = Type;
  static constexpr std::size_t offset = Offset;
  };

template<typename shared_type_decl_type>
concept concept_type_decl = requires
{
  typename shared_type_decl_type::type;
  shared_type_decl_type::offset;
  requires std::same_as<shared_type_decl_type,
    shared_type_decl<typename shared_type_decl_type::type,shared_type_decl_type::offset> >;
};
template<typename type>
concept concept_pointer = std::is_pointer_v<type>;

template<typename type>
concept concept_mapped_region = requires( type & region )
{
 region.get_address();
 {region.get_address()} -> concept_pointer;
};

template<concept_type_decl shared_type_decl_type, concept_mapped_region mapped_region, typename ... Args>
inline decltype(auto) construct_ref( mapped_region & region, Args && ... args ) noexcept
  {
  using type = typename shared_type_decl_type::type;
  constexpr auto offset { static_cast<ptrdiff_t>(shared_type_decl_type::offset) };
  auto addr{ std::next(reinterpret_cast<uint8_t *>(region.get_address()),offset) };
  return *std::construct_at( reinterpret_cast<type *>(addr), args ... );
  }
  
template<concept_type_decl shared_type_decl_type, concept_mapped_region mapped_region>
[[nodiscard]]
inline decltype(auto) ref( mapped_region & region ) noexcept
{
  using type = typename shared_type_decl_type::type;
  auto addr{ reinterpret_cast<uint8_t *>(region.get_address()) };
  constexpr auto offset { static_cast<ptrdiff_t>(shared_type_decl_type::offset) };
  return *std::launder(reinterpret_cast<type *>(std::next(addr,offset)));
}

}
