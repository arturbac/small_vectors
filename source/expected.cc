#include <utils/expected.h>

namespace cxx23
  {
#if !(defined(__cpp_lib_expected) && __cpp_lib_expected >= 202211L)
char const * bad_expected_access<void>::what() const noexcept { return "access to expected value without value"; }
#endif
  }  // namespace cxx23
