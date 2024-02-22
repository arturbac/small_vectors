#pragma once
#include <functional>
#include <type_traits>

namespace small_vector::utils
  {
template<typename Tag, typename X>
struct strong_function;  // undefined

template<typename Tag, typename ret_type, typename... args_type>
struct strong_function<Tag, ret_type(args_type...)>
  {
  std::function<ret_type(args_type...)> callable_;
  strong_function() = default;

  template<typename Cb, std::enable_if_t<std::is_invocable_r_v<ret_type, Cb, args_type...>, bool> = true>
  explicit strong_function(Cb callable) : callable_{callable}
    {
    }

  template<typename... fargs_type>
  auto operator()(fargs_type &&... args) const noexcept(noexcept(callable_(std::forward<fargs_type>(args)...)))
    {
    return callable_(std::forward<fargs_type>(args)...);
    }

  constexpr explicit operator bool() const noexcept { return static_cast<bool>(callable_); }
  };

template<typename Tag, class ret_type, class... args_type>
strong_function(Tag, ret_type (*)(args_type...)) -> strong_function<Tag, ret_type(args_type...)>;

  }  // namespace small_vector::utils
