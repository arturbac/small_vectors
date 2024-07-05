#pragma once

#include <source_location>
#include <format>

template<>
struct std::formatter<std::source_location>
  {
  template<typename ParseContext>
  constexpr auto parse(ParseContext & ctx) -> decltype(ctx.begin())
    {
    return ctx.begin();
    }

  template<typename FormatContext>
  auto format(std::source_location const & loc, FormatContext & ctx) const -> decltype(ctx.out())
    {
    return format_to(ctx.out(), "{}:{}", loc.line(), loc.file_name());
    }
  };
