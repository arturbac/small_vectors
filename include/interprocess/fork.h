#pragma once
#include <concepts>
#include <optional>
#include <functional>
#include <cstdlib>
#include <sys/unistd.h>
#include <sys/wait.h>

namespace ip
  {
struct fork_child_t
  {
  mutable pid_t pid_{};

  constexpr auto joinable() const noexcept { return pid_ != pid_t{}; }

  auto join() const noexcept -> bool
    {
    if(joinable())
      {
      int status;
      auto res = ::waitpid(pid_, &status, 0);
      pid_ = {};
      if(-1 != res)
        return EXIT_SUCCESS == WEXITSTATUS(status);
      }
    return false;
    }
  };

template<typename function, typename... Args>
  requires std::invocable<function, Args...>
[[nodiscard]]
inline auto fork(function const & fn, Args... args) noexcept -> std::optional<fork_child_t>
  {
  auto pid = ::fork();
  if(pid == 0)
    {
    bool status{};
    if constexpr(std::is_nothrow_invocable_v<function, Args...>)
      status = std::invoke(fn, args...);
    else
      try
        {
        status = std::invoke(fn, args...);
        }
      catch(...)
        {
        }
    std::exit(status ? EXIT_SUCCESS : EXIT_FAILURE);
    }
  else if(pid != -1)
    return fork_child_t{.pid_ = pid};
  else
    return {};
  }
  }  // namespace ip
