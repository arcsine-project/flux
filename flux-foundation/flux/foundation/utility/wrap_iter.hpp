#pragma once

namespace flux::fou {

template <typename Iterator> class wrap_iter final {
    using iterator_traits = ::std::iterator_traits<Iterator>;

public:
    using value_type        = iterator_traits::value_type;
    using reference         = iterator_traits::reference;
    using pointer           = iterator_traits::pointer;
    using difference_type   = iterator_traits::difference_type;
    using iterator_type     = Iterator;
    using iterator_category = iterator_traits::iterator_category;
    using iterator_concept  = meta::contiguous_iterator_tag;

    constexpr wrap_iter() noexcept : it_{} {}

    constexpr explicit wrap_iter(iterator_type const& it) noexcept : it_{it} {}

    template <meta::convertible_to<iterator_type> It>
    constexpr wrap_iter(wrap_iter<It> const& other) noexcept : it_{other.base()} {}

    [[nodiscard]] constexpr reference operator*() const noexcept {
        return *operator->();
    }

    [[nodiscard]] constexpr pointer operator->() const noexcept {
        return detail::to_address(it_);
    }

    [[nodiscard]] constexpr reference operator[](difference_type index) const noexcept {
        return it_[index];
    }

    constexpr wrap_iter& operator++() noexcept {
        ++it_;
        return *this;
    }

    constexpr wrap_iter operator++(int) noexcept {
        wrap_iter tmp(*this);
        ++(*this);
        return tmp;
    }

    constexpr wrap_iter& operator--() noexcept {
        --it_;
        return *this;
    }

    constexpr wrap_iter operator--(int) noexcept {
        wrap_iter tmp(*this);
        --(*this);
        return tmp;
    }

    constexpr wrap_iter& operator+=(difference_type n) noexcept {
        it_ += n;
        return *this;
    }

    constexpr wrap_iter& operator-=(difference_type n) noexcept {
        it_ -= n;
        return *this;
    }

    constexpr wrap_iter operator+(difference_type n) const noexcept {
        wrap_iter tmp(*this);
        tmp += n;
        return tmp;
    }

    constexpr wrap_iter operator-(difference_type n) const noexcept {
        return *this + (-n);
    }

    constexpr iterator_type base() const noexcept {
        return it_;
    }

private:
    iterator_type it_;

    template <typename U> friend class wrap_iter;
};

template <typename Iter1>
constexpr bool operator==(wrap_iter<Iter1> const& x, wrap_iter<Iter1> const& y) noexcept {
    return x.base() == y.base();
}

template <typename Iter1, meta::equality_comparable_with<Iter1> Iter2>
constexpr bool operator==(wrap_iter<Iter1> const& x, wrap_iter<Iter2> const& y) noexcept {
    return x.base() == y.base();
}

template <typename Iter1>
constexpr bool operator<=>(wrap_iter<Iter1> const& x, wrap_iter<Iter1> const& y) noexcept {
    return y <=> x;
}

template <typename Iter1, meta::totally_ordered_with<Iter1> Iter2>
constexpr bool operator<=>(wrap_iter<Iter1> const& x, wrap_iter<Iter2> const& y) noexcept {
    return y <=> x;
}

template <typename Iter1, typename Iter2>
constexpr auto operator-(wrap_iter<Iter1> const& x, wrap_iter<Iter2> const& y) noexcept {
    return x.base() - y.base();
}

template <typename Iter1>
constexpr wrap_iter<Iter1> operator+(typename wrap_iter<Iter1>::difference_type n,
                                     wrap_iter<Iter1>                           x) noexcept {
    x += n;
    return x;
}

} // namespace flux::fou