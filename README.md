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

### tested compilers

there are predefined cmake workflows to test
* cmake --workflow --preset="clang-16-release"
* cmake --workflow --preset="clang-16-libc++release"
* cmake --workflow --preset="clang-15-release"
* cmake --workflow --preset="clang-15-libc++release"
* cmake --workflow --preset="gcc-13-release"
* cmake --workflow --preset="gcc-12-release"
* msvc support is planned
