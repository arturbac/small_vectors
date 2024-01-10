#pragma once
#include <type_traits>
#include <concepts>
#include <atomic>
#include <iterator>
#include <ranges>
#include <algorithm>
#include <bit>

namespace ip
{
enum struct push_status : uint8_t
  {
  error_stack_changed,
  error_not_enough_space, // temporary out of space
  logic_error_not_enough_space, // block being pushed exceeds possible queue size
  succeed
  };
  
enum struct pop_status : bool
  {
  failed_stack_changed,
  succeed_range_valid
  };
namespace ranges = std::ranges;

struct stack_index_t
  {
  std::size_t index;
  constexpr std::size_t pack() const noexcept  { return index; }
  constexpr bool operator ==( stack_index_t const & r ) const noexcept = default;
  };
  
using atomic_index = std::atomic<stack_index_t>;
struct constexpr_index
  {
  using enum std::memory_order;
  stack_index_t value{};
  
  constexpr stack_index_t load(std::memory_order m = seq_cst) const noexcept { return value; }
  constexpr void store(stack_index_t v, std::memory_order m = seq_cst ) noexcept { value = v; }
  constexpr bool compare_exchange_strong(stack_index_t & e, stack_index_t i, std::memory_order m = seq_cst) noexcept
    {
    if(e == value) [[likely]]
      {
      value = i;
      return true;
      }
    else
      return false;
    }
  };
  
struct footer_t
  {
  std::size_t size;
  };
struct footer_data_t
  {
  std::array<uint8_t,sizeof(footer_t)> data_;
  };

///\brief experimental interprocess stack buffer supports single writer and single reader
template<std::size_t buffer_size, typename IndexType = atomic_index>
struct stack_buffer_t
  {
  using index_type = IndexType;
  using buffer_type = std::array<uint8_t,buffer_size+sizeof(footer_data_t)>;
  using buffer_const_iterator = buffer_type::const_iterator;
  
  buffer_type buffer_{};
  index_type stack_index_{};
  
  constexpr explicit stack_buffer_t() noexcept = default;
  };
  
  using enum std::memory_order;
  struct capacity_t
    {
    template<std::size_t buffer_size, typename index_type>
    [[nodiscard]]
    static constexpr auto operator()( stack_buffer_t<buffer_size,index_type> const & ) noexcept
      { return buffer_size; }
    };
  
  inline constexpr capacity_t capacity;
  
  struct empty_t
    {
    template<std::size_t buffer_size, typename index_type>
    [[nodiscard]]
    static constexpr auto operator()(stack_buffer_t<buffer_size,index_type> const & buff ) noexcept 
      {
      auto const b = buff.stack_index_.load(acquire);
      return b.index == 0;
      }
    };
    
  inline constexpr empty_t empty;
  namespace detail
    {
    template<std::size_t buffer_size, typename index_type>
    constexpr auto footer_from_index(stack_buffer_t<buffer_size,index_type> & buff, stack_index_t six ) noexcept
      {
      auto it_top_footer{ranges::next(ranges::begin(buff.buffer_), static_cast<ptrdiff_t>(six.index - sizeof(footer_data_t)))};
      footer_data_t footer_data;
      ranges::copy_n( it_top_footer, sizeof(footer_data_t), ranges::begin(footer_data.data_) );
      return std::make_pair(std::bit_cast<footer_t>(footer_data),it_top_footer);
      }
    }
    
  ///\brief push operation is lock free
  struct push_t
    {
    template<std::size_t buffer_size, typename index_type,
             std::forward_iterator iterator>
    [[nodiscard]]
    static constexpr auto operator()(stack_buffer_t<buffer_size,index_type> & buff,
                    iterator data_beg, iterator data_end ) noexcept
        -> push_status
      {
      // calculate space required
      stack_index_t stack_index{buff.stack_index_.load(acquire)};
      
      //emplace data even if read is in progress
      std::size_t const data_size { static_cast<std::size_t>(ranges::distance(data_beg, data_end))};
      std::size_t const free_space{ capacity(buff) - stack_index.index};
      std::size_t const block_size{ sizeof(footer_data_t) + data_size };
      
      if( block_size > free_space) [[unlikely]]
        {
        if( data_size > capacity(buff) ) [[unlikely]]
          return push_status::logic_error_not_enough_space;
        else
          return push_status::error_not_enough_space;
        }
      
      stack_index_t next_index{ stack_index.index + block_size };
      if(buff.stack_index_.compare_exchange_strong(stack_index, next_index, release )) [[likely]]
        {
        //there was no pop during data write
        auto ix_iter {ranges::copy(data_beg, data_end, ranges::next(ranges::begin(buff.buffer_), static_cast<ptrdiff_t>(stack_index.index))).out };
        auto index_info{std::bit_cast<footer_data_t>(data_size)};
        ranges::copy(ranges::begin(index_info.data_), ranges::end(index_info.data_), ix_iter);
        buff.stack_index_.store( next_index, release );
        return push_status::succeed;
        }
      else
        return push_status::error_stack_changed;
      }
    };
  inline constexpr push_t push;
  
  ///\brief top operation is lock free and is confirmed by pop
  struct top_t
    {
    template<std::size_t buffer_size, typename index_type>
    [[nodiscard]]
    static constexpr std::pair<std::span<uint8_t const>,stack_index_t> operator()(stack_buffer_t<buffer_size,index_type> & buff) noexcept
      {
      stack_index_t stack_during_copy_top{ buff.stack_index_.load(acquire) };
      if(stack_during_copy_top.index != 0 ) [[likely]]
        {
        auto [footer,it_top_footer]{ detail::footer_from_index(buff, stack_during_copy_top)};
        return std::make_pair(std::span<uint8_t const>{ ranges::prev(it_top_footer, static_cast<ptrdiff_t>(footer.size)),it_top_footer}, stack_during_copy_top);
        }
      return {};
      }
    };
  inline constexpr top_t top;
  
  struct pop_t
    {
    template<std::size_t buffer_size, typename index_type>
    [[nodiscard]]
    static constexpr pop_status operator()(stack_buffer_t<buffer_size,index_type> & buff,
                                stack_index_t stack_during_copy_top) noexcept
      {
      auto [footer,_]{ detail::footer_from_index(buff, stack_during_copy_top)};
      stack_index_t next_index { stack_during_copy_top.index - footer.size - sizeof(footer_data_t) };
      if(buff.stack_index_.compare_exchange_strong(stack_during_copy_top, next_index, release )) [[likely]]
        return pop_status::succeed_range_valid;
      else
        return pop_status::failed_stack_changed;
      }
    };
  inline constexpr pop_t pop;
}
