# small_vectors 
![MIT](https://img.shields.io/badge/license-MIT-blue.svg) ![CMake](https://github.com/arturbac/fixed_math/workflows/CMake/badge.svg)
![language](https://img.shields.io/badge/language-C%2B%2B20-blue.svg)![language](https://img.shields.io/badge/language-C%2B%2B23-red.svg) 

C++20,23 utilities library

## Features
- **Static Vector**: Trivially copyable for types that are trivially copyable, enabling compiler optimizations such as `memcpy` for copying operations (since v3.0.3-devel).
- **Address Independence**: Both static and small vectors offer in-class storage, making them address-independent and suitable for interprocess data exchange.
- **Dynamic and Custom Sized Storage**: Small vectors support dynamic memory allocation with customizable size types. Static vectors adjust the minimal size type based on the number of elements.
- **Constant Evaluation**: Static vectors can be fully evaluated at compile time for trivial element types.
- **Basic String with Dual Storage**: Provides a basic string implementation with both dynamic and static in-class storage options. The static storage variant is address-independent.
- **Basic Fixed String**: Enables manipulation of constant evaluated string literals.
- **Expected/Unexpected Implementation**: Offers a C++23 standard `expected/unexpected` implementation with monadic operations for C++20 and up.

## Minor Utility Features
- **Meta Packed Struct**: Supports bit-packing data with strong typing. Version 2.3.0 introduced signed type support.
- **Strong Type Wrapping**: Allows for the strong type wrapping of primitive types.
- **Unaligned Access**: Functions for memory unaligned access are fully capable of being executed at compile time starting with v2.4.2.

## Interprocess Features
- **Fork Wrapper**: Simplifies process spawning with an interface similar to `std::async`.
- **Shared Memory Utilities**: Facilitates the construction and access of data in shared interprocess memory with automated memory access indexing to prevent errors.

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
#### memutil::unaligned
```C++
consteval auto make_version_data(string_view sub_ver, string_view data_ver, uint16_t ver_minor, uint16_t comp_minor)
  {
  constexpr auto converter = [](char c) noexcept -> std::byte { return static_cast<std::byte>(c); };
  std::array<std::byte, map_version_t::map_version_raw_size> res{};
  auto it{ranges::transform(sub_ver, res.begin(), converter).out};
  *it = std::byte(' ');  // make space
  ++it;
  it = ranges::transform(data_ver, it, converter).out;
  *it = std::byte{};
  it = ranges::next(res.begin(), map_version_t::map_version_name_chars);
  it = memutil::unaligned_store<uint16_t>(it, expected_version_major);
  it = memutil::unaligned_store<uint16_t>(it, expected_version_minor);
  it = memutil::unaligned_store<uint16_t>(it, expected_version_major);
  memutil::unaligned_store<uint16_t>(it, expected_cmp_minor);
  return res;
  }
static constexpr std::array<std::byte, map_version_t::map_version_raw_size> 
  polska_6_1451_6_18{ make_version_data("Polska", "2403", 1451, 18)};
```
#### shared mem utils
example using static vector, basic_static_string between processes with memory offset table declaration
```C++
//used types between processes
struct foo
  {
  int a,a_;
  double b;
  int64_t c;
  };

using message = coll::static_u8string<512>;
using vector_type = coll::static_vector<uint32_t,128u>;

// memory offset table
using foo_obj_decl = ip::shared_type_decl<foo>;
using shared_vector_decl = ip::shared_type_decl<vector_type,foo_obj_decl>;
using ref_obj_decl = ip::shared_type_decl<int,shared_vector_decl>;
using message_decl = ip::shared_type_decl<message,ref_obj_decl>;

bip::mapped_region region ( shm, bip::read_write );

// construct objects in main process
foo & foo_obj{*ip::construct_at<foo_obj_decl>(region, foo{.a=1,.a_=0,.b=0.5, .c=0xffffffff })};
auto & ref_obj{*ip::construct_at<ref_obj_decl>(region, 2u)};
auto & ref_string { *ip::construct_at<message_decl>(region, u8"message hello world"sv) };
vector_type & vector_obj{ *ip::construct_at<shared_vector_decl>(region) };
resize(vector_obj,1);
front(vector_obj) = 2;

//alter data at forked process
auto child = ip::fork([](std::string_view shared_mem_name )
  {
  bip::shared_memory_object shm_obj{ bip::open_only, shared_mem_name.data() , bip::read_write };
  bip::mapped_region cregion{ shm_obj, bip::read_write };
  
  //reference shared objects
  foo & cfoo_obj{ ip::ref<foo_obj_decl>(cregion) };
  vector_type & vector { ip::ref<shared_vector_decl>(cregion) };
  auto & cref_string { ip::ref<message_decl>(cregion) };
  auto & cref_obj{ip::ref<ref_obj_decl>(cregion)};

  //read write data
  ut::expect( cfoo_obj.a == 1 );
  ut::expect( cfoo_obj.b == 0.5 );
  ut::expect( cfoo_obj.c == 0xffffffff );
  cfoo_obj.a = 2;
  cfoo_obj.b = 1.5;
  cfoo_obj.c = -0x1ffffffff;
  
  ut::expect(size(vector) == 1u );
  ut::expect(front(vector) == 2u );
  ut::expect(resize(vector,128) == coll::vector_outcome_e::no_error ) >> ut::fatal;
  pop_back(vector);
  std::iota(begin(vector), end(vector), 2);
  
  ut::expect( cref_string.view() == u8"message hello world"sv );
  cref_string = u8"hello world from child"sv;
  cref_obj += 2;

  return true;
  },
  shmem_name );

// check modified data at forked process
ut::expect(child->join()) >> ut::fatal;
ut::expect( foo_obj.a == 2 );
ut::expect( foo_obj.b == 1.5 );
ut::expect( foo_obj.c == -0x1ffffffff );

ut::expect( ref_string.view() == u8"hello world from child"sv );

ut::expect(ref_obj == 4 );

ut::expect(size(vector_obj) == 127u );
ut::expect(front(vector_obj) == 2 );
ut::expect(back(vector_obj) == 128 );
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
## Tested Compilers (as of v2.4.2)

### Make Workflows Tested
- `cmake --workflow --preset="clang-16-libc++release"`
- `cmake --workflow --preset="clang-17-release"` using GNU libstdc++ on Linux
- `cmake --workflow --preset="gcc-13-release"`
- `cmake --workflow --preset="gcc-12-release"`

### MSVC
- Tested intermittently
