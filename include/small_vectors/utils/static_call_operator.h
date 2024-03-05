#pragma once
#include <small_vectors/version.h>
#include <cstdint>

#if defined(__cpp_static_call_operator)
#define small_vector_static_call_operator static
#define small_vector_static_call_operator_const
#else
#define small_vector_static_call_operator
#define small_vector_static_call_operator_const const
#endif

