#pragma once

#include <small_vectors/version.h>
#include <string_view>
#include <source_location>

#if !defined(SMALL_VECTORS_CHECK_VALID_ELEMENT_ACCESS)
#define SMALL_VECTORS_CHECK_VALID_ELEMENT_ACCESS true
#endif

namespace small_vectors::inline v3_3::detail
  {
// motivated by to https://libcxx.llvm.org/Hardening.html
// checks that any attempts to access a container element, whether through the container object or through an iterator,
// are valid and do not attempt to go out of bounds or otherwise access a non-existent element.
inline constexpr bool check_valid_element_access{SMALL_VECTORS_CHECK_VALID_ELEMENT_ACCESS};

[[noreturn]]
void report_invalid_element_access(
  std::string_view, std::size_t size, std::size_t index, std::source_location loc = std::source_location::current()
);
  }  // namespace small_vectors::inline v3_3::detail
