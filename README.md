# small_vectors 
![MIT](https://img.shields.io/badge/license-MIT-blue.svg) ![CMake](https://github.com/arturbac/fixed_math/workflows/CMake/badge.svg)
![language](https://img.shields.io/badge/language-C%2B%2B20-blue.svg)![language](https://img.shields.io/badge/language-C%2B%2B23-red.svg) 

C++20,23 utilities library

## features
* static vectors with in class storage
* small vectors with in class storage and/or dynamic allocated memory with custom size type for small_vector and adjusted minimal size type for static_vector depending on number of elements
* static vector fully constant evaluated for trivial element types
* basic_string - with in class buffers fully constant evaluated, with dynamic and static storage ( static storage variant is address independant and may be used in interprocess data exchange)
* basic_fixed_string - for manipulating constant evaluated string literals
* coll::utf - fully constant evaluated utf8,16,32 string manupulation

## minor utility features

* meta_packed_struct - allows bit packing data with strong typing
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

#### coll:utf convertion namespace

* utf_input_view_t - utf view over range
* length - number of code points in range
* capacity_t<char_type> - number of bytes required to encode range into given char type, ie char8_t, char16_t, char32_t, wchar_t ...
* convert - convert range into output iterator with other utf encoding
* to_string_t<char_type> - convert utf range into coll::basic_string<> or std::basic_string<> with other utf encoding
* verify - verification of utf range
```C++
// any range (string string_view, array, vector ...) to output iterator with tpe deduction
constexpr auto view( auto const & a ) noexcept
  { return std::basic_string_view{ std::begin(a), std::end(a)}; }

constexpr std::basic_string_view u8test{u8"𐃆𐃇𐃈𐃉𐃊𐃋𐃌𐃍𐃎𐃏𐃐𐃑𐃒𐃓𐃔"};
constexpr std::basic_string_view u16test{u16"𐃆𐃇𐃈𐃉𐃊𐃋𐃌𐃍𐃎𐃏𐃐𐃑𐃒𐃓𐃔"};
std::array<char16_t, u16test.size()> storage;
utf::convert( u8test, std::begin(storage));
constexpr_test( view(storage) == u16test );

//any range to own string
constexpr_test( utf::to_u16string(u8test) == u16test );

//any range to std string
std::u16string str2 = utf::stl::to_u16string(u8test);
constexpr_test( str2 == u16test );
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
