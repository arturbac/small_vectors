#pragma once
#include <iterator>
#include <concepts>

namespace coll::detail
{
  /// \brief adapter for wrapping use of pointer arithmetic
  template<typename Iterator>
  struct adapter_iterator
    {
    using iterator_type = Iterator;
    using traits_type = std::iterator_traits<iterator_type>;
    
    using iterator_category = typename traits_type::iterator_category;
    using value_type = typename traits_type::value_type;
    using difference_type = typename traits_type::difference_type;
    using reference = typename traits_type::reference;
    using pointer = typename traits_type::pointer;
    using iterator_concept = typename traits_type::iterator_concept;
    
    iterator_type current_{};
    
    inline constexpr adapter_iterator() noexcept = default;
    
    inline constexpr explicit adapter_iterator(iterator_type const & i) noexcept 
      : current_(i) {}
    
    template<std::convertible_to<iterator_type> Iter>
    inline constexpr adapter_iterator( adapter_iterator<Iter> const & i) noexcept  
      : current_{i.base()} { }
      
    // forward
    [[nodiscard]]
    inline constexpr auto 
    operator*() const noexcept -> reference
      { return *current_; }
    
    [[nodiscard]]
    inline constexpr auto 
    operator->() const noexcept -> pointer
      { return current_; }
      
    inline constexpr auto
    operator++() noexcept -> adapter_iterator &
      {
      ++current_;
      return *this;
      }
    
    [[nodiscard]]
    inline constexpr auto
    operator++(int) noexcept -> adapter_iterator
       { return adapter_iterator{current_++}; }
       
    // bidirectional
    inline constexpr auto
    operator--() noexcept -> adapter_iterator &
      {
      --current_;
      return *this;
      }

    [[nodiscard]]
    inline constexpr auto
    operator--(int) noexcept -> adapter_iterator
      { return adapter_iterator{current_--}; }
      
    // random
    [[nodiscard]]
    inline constexpr auto
    operator[](std::size_t index) const noexcept -> reference
      { return current_[index]; }

    inline constexpr auto
    operator+=(difference_type index) noexcept -> adapter_iterator &
      { current_ += index; return *this; }

    [[nodiscard]]
    inline constexpr auto
    operator+(difference_type index) const noexcept -> adapter_iterator
      { return adapter_iterator(current_ + index); }

    inline constexpr auto
    operator-=(difference_type index) noexcept -> adapter_iterator &
      { current_ -= index; return *this; }

    [[nodiscard]]
    inline constexpr auto
    operator-(difference_type index) const noexcept -> adapter_iterator
      { return adapter_iterator(current_ - index); }

    [[nodiscard]]
    inline constexpr auto
    base() const noexcept -> iterator_type const &
      { return current_; }
    };
    
  template<typename iterator_type>
  adapter_iterator(iterator_type const & i) -> adapter_iterator<iterator_type>;
  
  template<typename iter1, std::equality_comparable_with<iter1> iter2>
  [[nodiscard]]
  inline constexpr auto
  operator==( adapter_iterator<iter1> const & l, adapter_iterator<iter2> const & r)
      noexcept(noexcept(l.base() == r.base()))
      -> bool
    { return l.base() == r.base(); }
    
  template<typename iter1, std::totally_ordered_with<iter1> iter2>
  [[nodiscard]]
  inline constexpr auto
  operator<=>(adapter_iterator<iter1> const & l, adapter_iterator<iter2> const & r)
      noexcept(noexcept(l.base() <=> r.base()))
    { return l.base() <=> r.base(); }

  template<typename iter1, typename iter2>
  [[nodiscard]]
  inline constexpr auto
  operator-(adapter_iterator<iter1> const & l, adapter_iterator<iter2> const & r) noexcept
    { return l.base() - r.base(); }

  template<typename iter1>
  [[nodiscard]]
  inline constexpr auto
  operator+(typename adapter_iterator<iter1>::difference_type n, adapter_iterator<iter1> const & i) noexcept
       -> adapter_iterator<iter1>
    { return adapter_iterator<iter1>(i.base() + n); }
}
