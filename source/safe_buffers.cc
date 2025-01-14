#include <small_vectors/detail/safe_buffers.h>
#include <small_vectors/formattable/source_location.h>
#include <small_vectors/safe_buffers_error_handler.h>
#include <cstdlib>
#include <print>

namespace small_vectors::inline v3_3
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wglobal-constructors"
static std::function<void(std::string_view)> error_handler = [](std::string_view error) { std::print("{}", error); };
#pragma clang diagnostic pop

auto set_error_report_handler(std::function<void(std::string_view)> handler) -> void
  {
  error_handler = std::move(handler);
  }
  }  // namespace small_vectors::inline v3_3

namespace small_vectors::inline v3_3::detail
  {
void report_invalid_element_access(std::string_view, std::size_t size, std::size_t index, std::source_location loc)
  {
  std::invoke(
    error_handler, std::format("[invalid_element_access] current size:{} index:{} called from {}", size, index, loc)
  );
  std::abort();
  }
  }  // namespace small_vectors::inline v3_3::detail
