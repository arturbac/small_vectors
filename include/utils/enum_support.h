#pragma once

#include <concepts>
#include <utils/utility_cxx20.h>

#include <cstdint>
#include <type_traits>
#include <string_view>

#define LT_( a ) L ## a
#define decl_enum_name_char( a ) \
         case a : \
         {static char const * name_##a = #a; \
         return name_##a;} 
         
#define decl_enum_name_sv( a ) \
         case a : \
         { return std::string_view{ #a }; } 
         
#define decl_enum_name_custom( a, b ) \
         case a : \
         {static char const * name_##a = b; \
         return name_##a;} 
         
#define decl_enum_name( a ) \
			case a : \
			{static wchar_t const * a##_name = LT_( #a ); \
			return a##_name;} 
			
			
#define decl_enum_name_ns( en, a ) \
      case en::a : \
      {static char const * a##_name = #a ; \
      return a##_name;} 
      
#define decl_enum_name_ns_sv( en, a ) \
      case en::a : \
      { return std::string_view{ #a };}
      
#define decl_enum_name_custom_ns_sv( en, a, b ) \
      case en::a : \
      { return std::string_view{ b };}
      
#define decl_enum_cmp( a, value_len ) \
			if( 0 == wcscmp( value, LT_( #a ) ) ) \
				return a

#define decl_enum_cmp_custom( src, ret ) \
         if( 0 == wcscmp( value, LT_( #src ) ) ) \
            return ret

#define decl_enum_cmp_char( a, value_len ) \
      if( 0 == strcmp( value, #a ) ) \
        return a

#define decl_enum_cmp_v( a ) \
      if( value == std::string_view{ #a }  ) \
        return a
        
#define decl_enum_cmp_ns_v( en, a ) \
      if( value == std::string_view{ #a }  ) \
        return en::a
        
namespace internal
{
[[nodiscard]]
constexpr bool enum_name_view_compare( std::string_view a, std::string_view b ) noexcept
  {
  return a.size() == b.size() && a == b;
  }
}
#define decl_enum_cmp_char_view( a, value_len ) \
      if( internal::enum_name_view_compare( value, #a ) ) \
        return a
        
#define decl_enum_cmp_char_ns( en, a ) \
      if( 0 == strcmp( value, #a ) ) \
        return en::a
        
#define decl_enum_cmp_custom_char( src, ret ) \
         if( 0 == strcmp( value, #src ) ) \
            return ret

#define decl_enum_cmp_custom_ns_v( en, a, b ) \
      if( value == std::string_view{ #a }  ) \
        return en::b
        
#define decl_enum_index(a,ix) \
	case a: return ix;

#define decl_enum_index_ns(en,a,ix) \
  case en::a: return ix;
namespace detail
{
  template<typename type>
  concept enum_concept = std::is_enum<type>::value;
}
///\brief tests if \ref value has set flag \ref test_flag
template<detail::enum_concept enum_type>
[[nodiscard]]
inline constexpr bool enum_test_flag( enum_type value, enum_type test_flag ) noexcept
  {
  return 0 != (cxx23::to_underlying( value ) & cxx23::to_underlying( test_flag ));
  }

///\returns conditionaly set flag \param cond_e depending on \param cond otherwise returns {}
template<detail::enum_concept enum_type>
[[nodiscard]]
inline constexpr enum_type enum_cond_flag(bool cond, enum_type cond_e) noexcept
  {
  if( cond )
    return cond_e;
  else
    return {};
  }
