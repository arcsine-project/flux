#pragma once

namespace flux::fou {

// clang-format n
// `It` - Any iterator (at least `InputIterator`);
// `Fn` - Unary functor (`F: It -> T&`).
template <meta::input_iterator It, meta::unary_functor<It> Fn>
class iterator_adapter final {
    using functor_type  = Fn;
    using iter_ref_type = meta::deref_t<It>;

public:
    using reference         = meta::deduce_t<functor_type(iter_ref_type)>;
    using value_type        = meta::remove_ref_t<reference>;
    using pointer           = meta::add_pointer_t<value_type>;
    using difference_type   = meta::iter_diff_t<It>;
    using iterator_type     = It;
    using iterator_category = meta::random_access_iterator_tag;
    using iterator_concept  = meta::contiguous_iterator_tag;

    FLUX_NO_UNIQUE_ADDRESS iterator_type it_;
    FLUX_NO_UNIQUE_ADDRESS functor_type  functor_;

    constexpr iterator_adapter() noexcept : it_{}, functor_{} {}

    constexpr iterator_adapter(iterator_type const& it, functor_type adapter = {}) noexcept
            : it_{it}, functor_{adapter} {}

    template <meta::convertible_to<iterator_type> I, typename F>
    constexpr iterator_adapter(iterator_adapter<I, F> const& other) noexcept
            : it_{other.it_}, functor_{other.functor_} {}

    [[nodiscard]] constexpr reference operator*() const noexcept {
        return *operator->();
    }

    [[nodiscard]] constexpr pointer operator->() const noexcept {
        return addressof(functor_(*it_));
    }

    [[nodiscard]] constexpr reference operator[](difference_type index) const noexcept {
        return functor_(it_[index]);
    }

    constexpr iterator_adapter& operator++() noexcept {
        ++it_;
        return *this;
    }

    constexpr iterator_adapter operator++(int) noexcept {
        return {it_++, functor_};
    }

    constexpr iterator_adapter& operator--() noexcept {
        --it_;
        return *this;
    }

    constexpr iterator_adapter operator--(int) noexcept {
        return {it_--, functor_};
    }

    constexpr iterator_adapter& operator+=(difference_type n) noexcept {
        it_ += n;
        return *this;
    }

    constexpr iterator_adapter& operator-=(difference_type n) noexcept {
        it_ -= n;
        return *this;
    }

    constexpr iterator_adapter operator+(difference_type n) const noexcept {
        return {it_ + n, functor_};
    }

    constexpr iterator_adapter operator-(difference_type n) const noexcept {
        return {it_ - n, functor_};
    }

    [[nodiscard]] constexpr difference_type
    operator-(iterator_adapter const& other) const noexcept {
        return it_ - other.it_;
    }

    [[nodiscard]] constexpr std::strong_ordering
    operator<=>(iterator_adapter const& other) const noexcept {
        return it_ <=> other.it_;
    }

    [[nodiscard]] constexpr bool operator==(iterator_adapter const& other) const noexcept {
        return it_ == other.it_;
    }

    friend constexpr auto operator+(difference_type n, iterator_adapter const& a) noexcept {
        return iterator_adapter{a.it_ + n, a.functor_};
    }
};
// clang-format on

} // namespace flux::fou