#pragma once

#include <system_error>
#include <format>

template<>
struct std::formatter<std::error_code>
  {
  template<typename ParseContext>
  constexpr auto parse(ParseContext & ctx) -> decltype(ctx.begin())
    {
    return ctx.begin();
    }

  template<typename FormatContext>
  auto format(std::error_code const & ec, FormatContext & ctx) const -> decltype(ctx.out())
    {
    return format_to(ctx.out(), "{}:{} - {}", ec.category().name(), ec.value(), ec.message());
    }
  };

