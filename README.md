# small_vectors

C++20 small and static vectors

## features
* static vectors with in class storage
* small vectors with in class storage and/or dynamic allocated memory
* custom size type for small_vector
* adjusted minimal size type for static_vector depending on number of elements
* static vector fully constant evaluated for trivial element types


### examples

```C++

#include <coll/static_vector.h>

//static vector with in buffer class memory for 10 elements
coll::static_vector<uint32_t,10u> vec10;

//small vector with in buffer class memory for coll::union_min_number_of_elements<int32_t,uint8_t>

coll::small_vector<int32_t,uint8_t> vec10;

//equivalent for std::vector<int32_t> with size type eq size_t and not in class buffer memory
coll::small_vector<int32_t,size_t,0> vec10;

```
