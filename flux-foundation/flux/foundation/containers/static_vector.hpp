#pragma once

namespace flux::fou {

template <meta::object T, ::std::size_t Capacity>
class [[nodiscard, clang::trivial_abi]] static_vector final {
    using uninitialized_storage = optional_uninitialized_storage<T>;
    using uninitialized_array   = ::std::array<uninitialized_storage, Capacity>;

    static_assert(meta::equal<sizeof(uninitialized_storage), sizeof(T)>);
    static_assert(meta::non_cv<T>, "T must not be cv-qualified");

    struct [[nodiscard]] mapper final {
        constexpr T& operator()(uninitialized_storage& storage) const noexcept {
            return get<T&>(storage);
        }
        constexpr T const& operator()(uninitialized_storage const& storage) const noexcept {
            return get<T const&>(storage);
        }
    };

    // clang-format off
    template <typename Iterator>
    using iterator_adapter = iterator_adapter<Iterator, mapper>;
    // clang-format on

public:
    using value_type             = T;
    using size_type              = ::std::size_t;
    using difference_type        = ::std::ptrdiff_t;
    using pointer                = T*;
    using const_pointer          = T const*;
    using reference              = T&;
    using const_reference        = T const&;
    using iterator               = iterator_adapter<typename uninitialized_array::iterator>;
    using const_iterator         = iterator_adapter<typename uninitialized_array::const_iterator>;
    using reverse_iterator       = ::std::reverse_iterator<iterator>;
    using const_reverse_iterator = ::std::reverse_iterator<const_iterator>;

    // Public so this type is a structural type and can thus be used in template parameters.
    FLUX_NO_UNIQUE_ADDRESS size_type           size_;
    FLUX_NO_UNIQUE_ADDRESS uninitialized_array storage_;

    constexpr static_vector() noexcept : size_{} {
        // Objects with a trivial lifetime will not be wrapped by `uninitialized_storage`
        // because we want to preserve their trivial properties
        if constexpr (meta::has_trivial_lifetime<T>) {
            // ...and at constant evaluation time initialization is a requirement.
            if consteval {
                value_construct_at(get<pointer>(storage_));
            }
        }
    }

    constexpr explicit static_vector(size_type count) noexcept
            : static_vector(count, value_type{}) {}

    constexpr static_vector(size_type count, T const& value) noexcept : static_vector() {
        FLUX_ASSERT(count <= capacity());
        ranges::uninitialized_construct_n(begin(), difference_type(count), value);
        size_ = count;
    }

    template <meta::input_iterator InputIterator>
    constexpr static_vector(InputIterator first, InputIterator last) noexcept : static_vector() {
        FLUX_ASSERT(ranges::sized_distance(first, last) <= capacity());
        insert(cend(), first, last);
    }

    template <meta::container_compatible_range<T> Range>
    constexpr static_vector(from_range_t, Range&& range) noexcept : static_vector() {
        FLUX_ASSERT(ranges::sized_distance(range) <= capacity());
        insert(cend(), ::std::ranges::begin(range), ::std::ranges::end(range));
    }

    constexpr explicit static_vector(::std::initializer_list<T> list) noexcept
            : static_vector(list.begin(), list.end()) {}

    constexpr static_vector(static_vector const& other) noexcept
        requires meta::trivially_copy_constructible<T>
    = default;
    constexpr static_vector(static_vector const& other) noexcept : static_vector() {
        ranges::uninitialized_copy_no_overlap(other.begin(), other.end(), begin());
        size_ = other.size();
    }

    constexpr static_vector(static_vector&& other) noexcept
        requires meta::trivially_move_constructible<T>
    = default;
    constexpr static_vector(static_vector&& other) noexcept : static_vector() {
        if constexpr (meta::relocatable<T>) {
            ranges::uninitialized_relocate_no_overlap(other.begin(), other.end(), begin());
            size_ = ::std::exchange(other.size_, 0u);
        } else {
            ranges::uninitialized_move(other.begin(), other.end(), begin());
            size_ = other.size_;
        }
    }

    constexpr static_vector& operator=(static_vector const& other) noexcept
        requires meta::trivially_copy_assignable<T>
    = default;
    constexpr static_vector& operator=(static_vector const& other) noexcept {
        clear();
        ranges::uninitialized_copy_no_overlap(other.begin(), other.end(), begin());
        size_ = other.size();
        return *this;
    }

    constexpr static_vector& operator=(static_vector&& other) noexcept
        requires meta::trivially_move_assignable<T>
    = default;
    constexpr static_vector& operator=(static_vector&& other) noexcept {
        clear();
        if constexpr (meta::relocatable<T>) {
            ranges::uninitialized_relocate_no_overlap(other.begin(), other.end(), begin());
            size_ = ::std::exchange(other.size_, 0u);
        } else {
            ranges::uninitialized_move(other.begin(), other.end(), begin());
            size_ = other.size_;
        }
        return *this;
    }

    // clang-format off
    constexpr ~static_vector() requires meta::trivially_destructible<T> = default;
    constexpr ~static_vector() { clear(); }
    // clang-format on

    constexpr void assign(size_type count, value_type const& value) noexcept {
        FLUX_ASSERT(count <= capacity());
        clear();
        resize(count, value);
    }
    constexpr void assign(::std::initializer_list<T> list) noexcept {
        clear();
        insert(cend(), list);
    }

    template <meta::input_iterator InputIterator>
    constexpr void assign(InputIterator first, InputIterator last) noexcept {
        clear();
        insert(cend(), first, last);
    }

    template <meta::container_compatible_range<T> Range>
    constexpr void assign_range(Range&& range) noexcept {
        clear();
        insert(cend(), ::std::ranges::begin(range), ::std::ranges::end(range));
    }

    // Capacity
    [[nodiscard]] static constexpr size_type max_size() noexcept {
        return Capacity;
    }
    [[nodiscard]] constexpr size_type size() const noexcept {
        return size_;
    }
    [[nodiscard]] constexpr size_type capacity() const noexcept {
        return Capacity;
    }
    [[nodiscard]] constexpr bool empty() const noexcept {
        return size() == 0;
    }
    constexpr void reserve([[maybe_unused]] size_type new_capacity) noexcept {
        FLUX_ASSERT(new_capacity <= capacity());
    }

    // Modifiers
    constexpr void clear() noexcept
        requires meta::trivially_destructible<T>
    {
        size_ = 0;
    }
    constexpr void clear() noexcept
        requires meta::not_trivially_destructible<T>
    {
        destroy_range(begin(), end());
        size_ = 0;
    }

    constexpr iterator insert(const_iterator position, value_type const& value) noexcept {
        auto const insert_position = move_elements(position, 1);
        construct_in_place(insert_position, value);
        return insert_position;
    }
    constexpr iterator insert(const_iterator position, value_type&& value) noexcept {
        auto const insert_position = move_elements(position, 1);
        construct_in_place(insert_position, value);
        return insert_position;
    }
    constexpr iterator insert(const_iterator position, ::std::initializer_list<T> list) noexcept {
        auto const count           = list.size();
        auto const insert_position = move_elements(position, count);
        ranges::uninitialized_copy_no_overlap(list.begin(), list.end(), insert_position);
        return insert_position;
    }

    // clang-format off
    template <meta::input_iterator InputIterator>
    constexpr iterator insert(const_iterator position, InputIterator first, InputIterator last) noexcept {
        if constexpr (meta::forward_iterator<InputIterator>) {
            auto const count           = ranges::sized_distance(first, last);
            auto const insert_position = move_elements(position, count);
            ranges::uninitialized_copy_no_overlap(first, last, insert_position);
            return insert_position;
        } else {
            auto current_position = const_to_mutable_it(position);
            auto  middle_position = end();

            // Place everything at the end.
            for (; first != last && size() < capacity(); ++first) {
                emplace_one_at_back(*first);
            }

            // Reached capacity.
            if (first != last) {
                [[maybe_unused]] size_type excess_element_count = 0;
                for (; first != last; ++first) {
                    ++excess_element_count;
                }

                FLUX_ASSERT(max_size() + excess_element_count <= capacity());
            }

            // Rotate into the correct places.
            ::std::rotate(current_position, middle_position, /* last_position: */ end());
            return current_position;
        }
    }
    // clang-format on

    template <meta::container_compatible_range<T> Range>
    constexpr iterator insert_range(const_iterator position, Range&& range) noexcept {
        return insert(position, ::std::ranges::begin(range), ::std::ranges::end(range));
    }

    template <meta::container_compatible_range<T> Range>
    constexpr void append_range(Range&& range) noexcept {
        insert(cend(), ::std::ranges::begin(range), ::std::ranges::end(range));
    }

    template <typename... Args>
    constexpr iterator emplace(const_iterator position, Args&&... args) noexcept {
        auto const emplace_position = move_elements(position, 1);
        construct_in_place(emplace_position, ::std::forward<Args>(args)...);
        return emplace_position;
    }

    constexpr iterator erase(const_iterator first, const_iterator last) noexcept {
        FLUX_ASSERT(first <= last, "invalid range first > last");
        FLUX_ASSERT(first >= cbegin() && last <= cend(), "iterators exceed container range");
        auto const elements_to_move   = ranges::distance(last, cend());
        auto const elements_to_remove = ranges::distance(first, last);

        auto move_first  = const_to_mutable_it(last);
        auto move_last   = ranges::next(move_first, elements_to_move);
        auto erase_first = const_to_mutable_it(first);

        if consteval {
            // Do the move.
            auto erase_last = ::std::move(move_first, move_last, erase_first);

            // Clean out the tail.
            destroy_range(erase_last, move_last);
        } else {
            // We can only use this when `!is_constant_evaluated`, since otherwise Clang
            // complains about objects being accessed outside their lifetimes.

            // Clean out the gap.
            destroy_range(erase_first, ranges::next(erase_first, elements_to_remove));

            // Do the relocation.
            ranges::uninitialized_relocate(move_first, move_last, erase_first);
        }

        size_ -= static_cast<size_type>(elements_to_remove);
        return erase_first;
    }

    constexpr iterator erase(const_iterator position) noexcept {
        return erase(position, position + 1);
    }

    // clang-format off
    template <typename... Args>
    constexpr reference emplace_back(Args&&... args) noexcept {
        return emplace_one_at_back(::std::forward<Args>(args)...);
    }
    // clang-format on

    constexpr void push_back(value_type const& value) noexcept {
        emplace_one_at_back(value);
    }
    constexpr void push_back(value_type&& value) noexcept {
        emplace_one_at_back(::std::move(value));
    }

    constexpr void pop_back() noexcept {
        FLUX_ASSERT(!empty());
        --size_;
        destroy_in_place(end());
    }

    constexpr void resize(size_type count, value_type const& value) noexcept {
        FLUX_ASSERT(count <= capacity());

        // Reinitialize the new members if we are enlarging.
        while (size() < count) {
            construct_in_place(end(), value);
            ++size_;
        }
        // Destroy extras if we are making it smaller.
        while (size() > count) {
            --size_;
            destroy_in_place(end());
        }
    }
    constexpr void resize(size_type count) noexcept
        requires meta::default_constructible<T>
    {
        resize(count, value_type{});
    }

    // Element access
    FLUX_ALWAYS_INLINE
    [[nodiscard]] constexpr pointer data() noexcept {
        return get<pointer>(storage_.front());
    }
    FLUX_ALWAYS_INLINE
    [[nodiscard]] constexpr const_pointer data() const noexcept {
        return get<const_pointer>(storage_.front());
    }

    FLUX_ALWAYS_INLINE
    [[nodiscard]] constexpr reference front() noexcept {
        return get<reference>(storage_.front());
    }
    FLUX_ALWAYS_INLINE
    [[nodiscard]] constexpr const_reference front() const noexcept {
        return get<const_reference>(storage_.front());
    }

    FLUX_ALWAYS_INLINE
    [[nodiscard]] constexpr reference back() noexcept {
        return get<reference>(storage_[size() - 1]);
    }
    FLUX_ALWAYS_INLINE
    [[nodiscard]] constexpr const_reference back() const noexcept {
        return get<const_reference>(storage_[size() - 1]);
    }

    FLUX_ALWAYS_INLINE
    [[nodiscard]] constexpr reference operator[](size_type index) noexcept {
        return get<reference>(storage_[index]);
    }
    FLUX_ALWAYS_INLINE
    [[nodiscard]] constexpr const_reference operator[](size_type index) const noexcept {
        return get<const_reference>(storage_[index]);
    }

    // Iterators
    constexpr iterator begin() noexcept {
        return {storage_.begin()};
    }
    constexpr const_iterator begin() const noexcept {
        return {storage_.begin()};
    }
    constexpr iterator end() noexcept {
        return {storage_.begin() + static_cast<difference_type>(size())};
    }
    constexpr const_iterator end() const noexcept {
        return {storage_.begin() + static_cast<difference_type>(size())};
    }

    constexpr const_iterator cbegin() const noexcept {
        return begin();
    }
    constexpr const_iterator cend() const noexcept {
        return end();
    }

    constexpr reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }
    constexpr const_reverse_iterator rbegin() const noexcept {
        return crbegin();
    }
    constexpr const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(cend());
    }

    constexpr reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }
    constexpr const_reverse_iterator rend() const noexcept {
        return crend();
    }
    constexpr const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(cbegin());
    }

    // Span
    constexpr operator ::std::span<value_type>() noexcept {
        return {data(), size()};
    }
    constexpr operator ::std::span<value_type const>() const noexcept {
        return {data(), size()};
    }

    // Swap
    friend constexpr void swap(static_vector& lhs, static_vector& rhs) noexcept {
        if (addressof(lhs) == addressof(rhs)) {
            return;
        }

        auto swap_impl = [](static_vector& a, static_vector& b) {
            auto b_last = b.end();
            auto a_it   = ::std::swap_ranges(b.begin(), b_last, a.begin());
            ranges::uninitialized_relocate_no_overlap(a_it, a.end(), b_last);
        };
        if (lhs.size() >= rhs.size()) {
            swap_impl(lhs, rhs);
        } else {
            swap_impl(rhs, lhs);
        }
        ::std::swap(lhs.size_, rhs.size_);
    }

    // Comparators
    template <size_type OtherCapacity>
    [[nodiscard]] constexpr bool
    operator==(static_vector<T, OtherCapacity> const& other) const noexcept {
        if (size() != other.size()) {
            return false;
        }
        return ::std::equal(begin(), end(), other.begin());
    }

    template <size_type OtherCapacity>
    [[nodiscard]] constexpr auto
    operator<=>(static_vector<T, OtherCapacity> const& other) const noexcept {
        return ::std::lexicographical_compare_three_way(begin(), end(), other.begin(), other.end());
    }

private:
    constexpr iterator const_to_mutable_it(const_iterator it) noexcept {
        return ranges::next(begin(), ranges::distance(cbegin(), it));
    }

    // clang-format off
    template <typename... Args>
    constexpr reference emplace_one_at_back(Args&&... args) noexcept {
        FLUX_ASSERT(size() + 1 <= capacity());
        construct_in_place(end(), ::std::forward<Args>(args)...);
        ++size_;
        return back();
    }

    constexpr iterator move_elements(const_iterator position, meta::integral auto n) noexcept {
        FLUX_ASSERT(size() + static_cast<size_type>(n) <= capacity());
        auto const elements_to_move = ranges::distance(position, cend());
        size_                      += static_cast<size_type>(n);

        auto first  = const_to_mutable_it(position);
        auto last   = ranges::next(first, elements_to_move);
        auto result = ranges::next(first, static_cast<difference_type>(n) + elements_to_move);
        [[maybe_unused]] auto out = ranges::uninitialized_relocate_backward(first, last, result);
        return first;

        #if 0
        size_type const begin_offset = static_cast<size_type>(position - begin());
        size_type const   end_offset = begin_offset + static_cast<size_type>(n);

        size_type const elements_to_move = size() - begin_offset;

        size_type const from_index = begin_offset + elements_to_move - 1;
        size_type const   to_index =   end_offset + elements_to_move - 1;

        if constexpr (meta::trivially_relocatable<T>) {
            if consteval {
                for (size_type i = 0; i < elements_to_move; ++i) {
                    relocate_at(get<pointer>(storage_[from_index - i]),
                                get<pointer>(storage_[  to_index - i]));
                }
            } else {
                detail::constexpr_memmove(get<pointer>(storage_[  end_offset]),
                                          get<pointer>(storage_[begin_offset]),
                                          elements_to_move);
            }
        } else {
            for (size_type i = 0; i < elements_to_move; ++i) {
                relocate_at(get<pointer>(storage_[from_index - i]),
                            get<pointer>(storage_[  to_index - i]));
            }
        }
        size_ += static_cast<size_type>(n);
        return begin() + difference_type(begin_offset);
        #endif
    }
    // clang-format on
};

template <typename T, typename... Args>
static_vector(T, Args...) -> static_vector<meta::enforce_same_t<T, Args...>, 1 + sizeof...(Args)>;

} // namespace flux::fou