#pragma once
#include <small_vectors/small_vector.h>
#include <small_vectors/basic_string.h>
#include <small_vectors/utils/static_call_operator.h>
#include <concepts>
#include <numeric>
#include <unordered_map>
#include <boost/container/flat_map.hpp>

/// \brief Burrows–Wheeler transform
namespace small_vectors::inline v3_0::algo::bwt
  {
namespace concepts
  {
  template<typename value_type>
  concept char_type
    = std::same_as<std::remove_const_t<value_type>, char> || std::same_as<std::remove_const_t<value_type>, char8_t>
      || std::same_as<std::remove_const_t<value_type>, char16_t>
      || std::same_as<std::remove_const_t<value_type>, char32_t>
      || std::same_as<std::remove_const_t<value_type>, wchar_t>;
  }

template<char EndMarker>
struct encode_t
  {
  static constexpr char end_marker = EndMarker;

  template<
    std::contiguous_iterator source_iterator,
    std::sentinel_for<source_iterator> sentinel,
    std::weakly_incrementable out_iterator>
    requires std::indirectly_copyable<source_iterator, out_iterator>
  small_vector_static_call_operator constexpr auto
    operator()(source_iterator beg, sentinel end, out_iterator out) small_vector_static_call_operator_const noexcept
    -> out_iterator
    {
    namespace ranges = std::ranges;
    using char_type = std::iter_value_t<source_iterator>;
    using string_type = small_vectors::basic_string<char_type>;
    using size_type = typename string_type::size_type;
    if(beg != end)
      {
      size_type sz{size_type(ranges::distance(beg, end) + 1u)};
      string_type table;
      table.resize_and_overwrite(
        sz + sz,
        [sz, beg, end](char_type * data, size_type /*cap*/)
        {
          auto it{data};
          it = ranges::copy(beg, end, it).out;
          *it = char_type(end_marker);
          ++it;
          it = ranges::copy(beg, end, it).out;
          *it = char_type(end_marker);
          return sz + sz;
        }
      );
      using view_type = std::basic_string_view<char_type>;
      auto const f_view = [sz, &table](uint32_t ix) noexcept
      {
        auto begit{ranges::next(ranges::begin(table), ix)};
        return view_type{begit, ranges::next(begit, sz)};
      };

      small_vectors::small_vector<uint32_t, uint32_t> rotvec;
      rotvec.resize(sz);
      std::iota(ranges::begin(rotvec), ranges::end(rotvec), 0u);
      ranges::sort(
        rotvec, [&f_view](uint32_t lix, uint32_t rix) noexcept { return f_view(lix).compare(f_view(rix)) < 0; }
      );

      out = ranges::transform(
              rotvec,
              out,
              [&table, sz](uint32_t ix) noexcept -> char_type
              {
                char_type c{table[ix + sz - 1u]};
                return c;
              }
      ).out;
      }

    return out;
    }

  template<std::ranges::contiguous_range contiguous_range, std::weakly_incrementable out_iterator>
  small_vector_static_call_operator constexpr auto
    operator()(contiguous_range const & range, out_iterator out) small_vector_static_call_operator_const noexcept
    {
    return operator()(std::ranges::begin(range), std::ranges::end(range), out);
    }
  };

template<char end_char>
inline constexpr encode_t<end_char> encode;

template<char EndMarker>
struct decode_t
  {
  static constexpr char end_marker = EndMarker;

  template<
    std::contiguous_iterator source_iterator,
    std::sentinel_for<source_iterator> sentinel,
    std::weakly_incrementable out_iterator>
    requires std::indirectly_copyable<source_iterator, out_iterator>
  small_vector_static_call_operator constexpr auto
    operator()(source_iterator beg, sentinel end, out_iterator out) small_vector_static_call_operator_const noexcept
    -> out_iterator
    {
    namespace ranges = std::ranges;
    using small_vectors::small_vector;
    using char_type = std::iter_value_t<source_iterator>;
    using string_type = small_vectors::basic_string<char_type>;
    using size_type = typename string_type::size_type;
    using index_list = small_vector<uint32_t, uint32_t>;
    if(beg != end)
      {
      std::span const btw_arr{beg, end};
      auto it_x{ranges::find(btw_arr, char_type(end_marker))};
      if(it_x != ranges::end(btw_arr))
        {
        size_type const sz{size_type(btw_arr.size())};
        small_vector<char_type, uint32_t> sorted_bwt(sz);
        ranges::copy(btw_arr, ranges::begin(sorted_bwt));
        ranges::sort(sorted_bwt);
        index_list l_shift(sz);

        // Array of lists to compute l_shift
        std::unordered_map<char_type, index_list> arr(sz);

        // Adds each character of bwtArr to a linked list
        // and appends to it the new node whose data part
        // contains index at which character occurs in bwtArr
        for(uint32_t i = 0; i != sz; i++)
          arr[btw_arr[i]].emplace_back(i);

        // Adds each character of sortedBwt to a linked list
        // and finds lShift
        for(uint32_t i = 0; i != sz; i++)
          {
          l_shift[i] = arr[sorted_bwt[i]][0u];
          arr[sorted_bwt[i]].erase(ranges::begin(arr[sorted_bwt[i]]));
          }
        // Index at which original string appears
        // in the sorted rotations list
        uint32_t x = static_cast<uint32_t>(ranges::distance(ranges::begin(btw_arr), it_x));
        index_list decoded_ix(sz);
        for(uint32_t i = 0; i != sz; i++)
          {
          x = l_shift[x];
          decoded_ix[sz - 1 - i] = x;
          }
        auto last{ranges::prev(std::make_reverse_iterator(ranges::begin(decoded_ix)))};
        auto first{std::make_reverse_iterator(ranges::end(decoded_ix))};

        out = ranges::transform(first, last, out, [&btw_arr](uint32_t ix) noexcept { return btw_arr[ix]; }).out;
        }
      }
    return out;
    }

  template<std::ranges::contiguous_range contiguous_range, std::weakly_incrementable out_iterator>
  small_vector_static_call_operator constexpr auto
    operator()(contiguous_range const & range, out_iterator out) small_vector_static_call_operator_const noexcept
    {
    return operator()(std::ranges::begin(range), std::ranges::end(range), out);
    }
  };

template<char end_char>
inline constexpr decode_t<end_char> decode;
  }  // namespace small_vectors::inline v3_0::algo::bwt
