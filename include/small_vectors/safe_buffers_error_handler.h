#pragma once
#include <functional>
#include <string_view>

namespace small_vectors::inline v3_2
  {
auto set_error_report_handler(std::function<void(std::string_view)>) -> void;
  }
