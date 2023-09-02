# small_vectors 
![MIT](https://img.shields.io/badge/license-MIT-blue.svg) ![CMake](https://github.com/arturbac/fixed_math/workflows/CMake/badge.svg)
![language](https://img.shields.io/badge/language-C%2B%2B20-blue.svg)![language](https://img.shields.io/badge/language-C%2B%2B23-red.svg) 

C++20,23 utilities library

## features
* static vectors with in class storage ( static storage variant is address independant and may be used in interprocess data exchange)
* small vectors with in class storage and/or dynamic allocated memory with custom size type for small_vector and adjusted minimal size type for static_vector depending on number of elements
* static vector fully constant evaluated for trivial element types
* basic_string - with in class buffers fully constant evaluated, with dynamic and static storage ( static storage variant is address independant and may be used in interprocess data exchange)
* basic_fixed_string - for manipulating constant evaluated string literals
* C++23 standard expected/unexpected implementation with monadic ops for c++20 and up

## minor utility features

* meta_packed_struct - allows bit packing data with strong typing, v2.3.0 added signed type support
* strong_type - strong type wrapping for primitive types
* unaligned_load and unaligned_store - for memory unaligned access

## interprocess features
* fork - wrapper for easy process spawning with std::async like interface
* shared_mem_utils - for construction and access data in shared interprocess memory with auto memory accesss indexing preventing errors
* 
### examples

#### small_vector and static_vector
```C++

#include <coll/static_vector.h>

//static vector with in buffer class memory for 10 elements
coll::static_vector<uint32_t,10u> vec10;

//small vector with in buffer class memory for coll::union_min_number_of_elements<int32_t,uint8_t>

coll::small_vector<int32_t,uint8_t> vec10;

//equivalent for std::vector<int32_t> with size type eq size_t and not in class buffer memory
coll::small_vector<int32_t,size_t,0> vec10;

```
#### expected/unexpected
```C++
using expected_type = expected<value_type,error_type>;
auto f = [](value_type v) noexcept { return expected_type{ in_place, ++v}; };
expected_type ex{ in_place, value_type{2} };
auto res { std::move(ex).and_then(f) };
constexpr_test( std::same_as<decltype(res), expected_type>);
constexpr_test( res == value_type{3});
```
#### meta_packed_struct
```C++
enum struct mbs_fields 
  {
    field_1, field_2, field_3, field_4
  };
enum struct example_enum_value : uint8_t
  { value0 = 0, value1, value2, value3 };

using enum mbs_fields;
// pack bit struct
using mixed_bitfiled_struct3 = 
    meta_packed_struct<
      member<uint8_t,mbs_fields::field_1,4>,
      member<bool,mbs_fields::field_2,1>,
      member<uint64_t ,mbs_fields::field_3,56>,
      member<example_enum_value, mbs_fields::field_4,3>
      >;
mixed_bitfiled_struct3 mbs;
get<field_3>(mbs) = (0x1llu<<56)-1;
constexpr_test(get<field_1>(mbs) == 0 );
constexpr_test(get<field_2>(mbs) == false );
constexpr_test(get<field_3>(mbs) == (0x1llu<<56)-1 );
constexpr_test(get<field_4>(mbs) == example_enum_value{} );

auto packed_value = pack_value<uint64_t>(mbs);
constexpr_test(packed_value == 0b00'11111111111111111111111111111111111111111111111111111111'0'0000 );

// unpack bitstruct
using mixed_bitfiled_struct2 = 
  meta_packed_struct<
    member<uint8_t,mbs_fields::field_1,4>,
    member<bool,mbs_fields::field_2,1>,
    member<uint16_t ,mbs_fields::field_3,16>,
    member<example_enum_value, mbs_fields::field_4,3>
    >;
constexpr auto fcount = filed_count<mixed_bitfiled_struct2>();
constexpr_test(fcount == 4);
constexpr auto s_bit_width = bit_width<mixed_bitfiled_struct2>();
constexpr_test(s_bit_width == 24);
uint32_t packed_value{ 0b011000011111111000010010 };
auto mbs{ unpack_value<mixed_bitfiled_struct2>(packed_value) };

constexpr_test(get<field_1>(mbs) == 0x02 );
constexpr_test(get<field_2>(mbs) == true );
constexpr_test(get<field_3>(mbs) == 0x0ff0 );
constexpr_test(get<field_4>(mbs) == value3 );
```
### tested compilers

there are predefined cmake workflows to test
* cmake --workflow --preset="clang-16-release"
* cmake --workflow --preset="clang-16-libc++release"
* cmake --workflow --preset="clang-15-release"
* cmake --workflow --preset="clang-15-libc++release"
* cmake --workflow --preset="gcc-13-release"
* cmake --workflow --preset="gcc-12-release"
* msvc support is planned
