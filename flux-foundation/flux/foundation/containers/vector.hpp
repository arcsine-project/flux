#pragma once

// TODO:
//  * Implement wrap_iter for vector;
//  * Implement rebind for std_allocator_adapter, so it can be fully compatible;
//  * Get rid of explicit Allocator wrapping by std_allocator_adapter inside the vector;
//  ? Maybe switch to some kind of compressed_pair instead of vector_impl;
//  * ...

namespace flux::fou {

namespace detail {

// clang-format off
// template <typename T>
// struct vector_impl final {
//     T* begin   = nullptr;
//     T* end     = nullptr;
//     T* end_cap = nullptr;
// };

template <::std::size_t Times>
constexpr auto calculate_growth(::std::size_t size) noexcept {
    ::std::size_t result = 0;

    if (size) [[likely]] {
#if __has_builtin(__builtin_mul_overflow)
        if (__builtin_mul_overflow(size, Times, addressof(result))) [[unlikely]] {
            fast_terminate();
        }
#else
        constexpr ::std::size_t max_size = SIZE_MAX / Times;
        if (size > max_size) [[unlikely]] {
            fast_terminate();
        }
        result = size << 1zu;
#endif
    }
    return result;
}

constexpr auto grow_twice(::std::size_t size) noexcept {
    return calculate_growth</* times: */ 2zu>(size);
}
// clang-format on

} // namespace detail

template <meta::object T, typename Allocator = std_allocator_adapter<T, default_allocator>>
class [[nodiscard, clang::trivial_abi]] vector final {
    using default_allocator_type = std_allocator_adapter<T, default_allocator>;
    using allocator_traits       = allocator_traits<Allocator>;

public:
    using value_type             = T;
    using allocator_type         = default_allocator_type;
    using size_type              = typename allocator_traits::size_type;
    using difference_type        = typename allocator_traits::difference_type;
    using pointer                = T*;
    using const_pointer          = T const*;
    using reference              = T&;
    using const_reference        = T const&;
    using iterator               = wrap_iter<pointer>;
    using const_iterator         = wrap_iter<const_pointer>;
    using reverse_iterator       = ::std::reverse_iterator<iterator>;
    using const_reverse_iterator = ::std::reverse_iterator<const_iterator>;

    static_assert(meta::non_cv<T>, "T must not be cv-qualified");
    static_assert(not is_stateful_allocator<allocator_type>::value,
                  "Allocator must not be stateful");
    static_assert(meta::same_as<typename allocator_type::value_type, value_type>,
                  "Allocator::value_type must be same type as T");

    FLUX_NO_UNIQUE_ADDRESS pointer        begin_     = {};
    FLUX_NO_UNIQUE_ADDRESS pointer        end_       = {};
    FLUX_NO_UNIQUE_ADDRESS pointer        end_cap_   = {};
    FLUX_NO_UNIQUE_ADDRESS allocator_type allocator_ = {};

    constexpr vector() noexcept {}

    constexpr explicit vector(size_type count) noexcept {
        if (count > 0) {
            vallocate(count);
            end_ = ranges::uninitialized_default_construct_n(begin_, difference_type(count));
        }
    }

    constexpr vector(size_type count, T const& value) noexcept {
        if (count > 0) {
            vallocate(count);
            end_ = ranges::uninitialized_fill_n(begin_, difference_type(count), value);
        }
    }

    template <meta::input_iterator InputIterator>
    constexpr vector(InputIterator first, InputIterator last) noexcept {
        init_with_sentinel(first, last);
    }

    template <meta::container_compatible_range<T> Range>
    constexpr vector(from_range_t, Range&& range) noexcept {
        init_with_sentinel(::std::ranges::begin(range), ::std::ranges::end(range));
    }

    constexpr explicit vector(::std::initializer_list<T> list) noexcept
            : vector(list.begin(), list.end()) {}

    // clang-format off
    constexpr vector(vector const& other) noexcept
        requires meta::copyable<T>
            : begin_    { /* nullptr default init */ },
              end_      { /* nullptr default init */ },
              end_cap_  { /* nullptr default init */ },
              allocator_{allocator_traits::select_on_container_copy_construction(other.allocator_)}
    {
        auto const other_size = other.size();
        if (0 == other_size) [[unlikely]] {
            return;
        }

        vallocate(other_size);
        end_ = ranges::uninitialized_copy_no_overlap(other.begin_, other.end_, begin_);
    }
    constexpr vector(vector const& other) noexcept = delete;

    constexpr vector(vector&& other) noexcept
            : begin_    {::std::exchange(other.begin_  , nullptr)},
              end_      {::std::exchange(other.end_    , nullptr)},
              end_cap_  {::std::exchange(other.end_cap_, nullptr)},
              allocator_{::std::move(other.allocator_)           } {}
    // clang-format on

    constexpr vector& operator=(vector const& other) noexcept
        requires meta::copyable<T>
    {
        if (addressof(other) != this) {
            copy_assign_alloc(other, allocator_traits::propagate_on_container_copy_assignment());
            assign(other.begin(), other.end());
        }
        return *this;
    }
    constexpr vector& operator=(vector const& other) noexcept = delete;

    constexpr vector& operator=(vector&& other) noexcept {
        move_assign_alloc(other, allocator_traits::propagate_on_container_move_assignment());
        return *this;
    }

    constexpr ~vector() {
        vdeallocate();
    }

    constexpr void assign(size_type count, value_type const& value) noexcept {
        clear();
        if (capacity() < count) {
            vreallocate_assign(count, value);
        } else {
            end_ = ranges::uninitialized_fill_n(end_, difference_type(count), value);
        }
    }
    // constexpr void assign(::std::initializer_list<T> list) noexcept {
    //     clear();
    //     insert(cend(), list);
    // }

    // template <meta::input_iterator InputIterator>
    // constexpr void assign(InputIterator first, InputIterator last) noexcept {
    //     clear();
    //     insert(cend(), first, last);
    // }

    // template <meta::container_compatible_range<T> Range>
    // constexpr void assign_range(Range&& range) noexcept {
    //     clear();
    //     insert(cend(), ::std::ranges::begin(range), ::std::ranges::end(range));
    // }

    constexpr void resize(size_type count, value_type const& value) noexcept {
        // Reinitialize the new members if we are enlarging.
        if (size() < count) {
            if (capacity() < count) {
                vreallocate(detail::grow_twice(count));
            }

            auto const n = static_cast<difference_type>(count - size());
            end_         = ranges::uninitialized_fill_n(end_, n, value);
        }
        // Destroy extras if we are making it smaller.
        while (size() > count) {
            --end_;
            destroy_in_place(end());
        }
    }
    constexpr void resize(size_type count) noexcept
        requires meta::default_constructible<T>
    {
        resize(count, value_type{});
    }

    // Capacity
    [[nodiscard]] static constexpr size_type max_size() noexcept {
        return static_cast<size_type>(::std::numeric_limits<difference_type>::max());
    }
    [[nodiscard]] constexpr size_type size() const noexcept {
        return static_cast<size_type>(end_ - begin_);
    }
    [[nodiscard]] constexpr size_type capacity() const noexcept {
        return static_cast<size_type>(end_cap_ - begin_);
    }
    [[nodiscard]] constexpr bool empty() const noexcept {
        return begin_ == end_;
    }
    constexpr void reserve([[maybe_unused]] size_type new_capacity) noexcept {}

    // Modifiers
    constexpr void clear() noexcept
        requires meta::trivially_destructible<T>
    {
        end_ = begin_;
    }
    constexpr void clear() noexcept
        requires meta::not_trivially_destructible<T>
    {
        destroy_range(begin(), end());
        end_ = begin_;
    }

    // Element access
    FLUX_ALWAYS_INLINE
    [[nodiscard]] constexpr pointer data() noexcept {
        return detail::to_address(begin_);
    }
    FLUX_ALWAYS_INLINE
    [[nodiscard]] constexpr const_pointer data() const noexcept {
        return detail::to_address(begin_);
    }

    FLUX_ALWAYS_INLINE
    [[nodiscard]] constexpr reference front() noexcept {
        FLUX_ASSERT(!empty(), "front() called on an empty vector");
        return *begin_;
    }
    FLUX_ALWAYS_INLINE
    [[nodiscard]] constexpr const_reference front() const noexcept {
        FLUX_ASSERT(!empty(), "front() called on an empty vector");
        return *begin_;
    }

    FLUX_ALWAYS_INLINE
    [[nodiscard]] constexpr reference back() noexcept {
        FLUX_ASSERT(!empty(), "back() called on an empty vector");
        return *(end_ - 1);
    }
    FLUX_ALWAYS_INLINE
    [[nodiscard]] constexpr const_reference back() const noexcept {
        FLUX_ASSERT(!empty(), "back() called on an empty vector");
        return *(end_ - 1);
    }

    FLUX_ALWAYS_INLINE
    [[nodiscard]] constexpr reference operator[](size_type index) noexcept {
        FLUX_ASSERT(index < size(), "vector[] index out of bounds");
        return begin_[index];
    }
    FLUX_ALWAYS_INLINE
    [[nodiscard]] constexpr const_reference operator[](size_type index) const noexcept {
        FLUX_ASSERT(index < size(), "vector[] index out of bounds");
        return begin_[index];
    }

    // Iterators
    constexpr iterator begin() noexcept {
        return iterator{begin_};
    }
    constexpr const_iterator begin() const noexcept {
        return const_iterator{begin_};
    }
    constexpr iterator end() noexcept {
        return iterator{end_};
    }
    constexpr const_iterator end() const noexcept {
        return const_iterator{end_};
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

private:
    constexpr void vallocate(size_type capacity) noexcept {
        // clang-format off
        auto allocation = allocate_at_least(allocator_, capacity);
        end_            = begin_ = allocation.ptr;
        end_cap_        = begin_ + allocation.count;
        // clang-format on
    }

    constexpr void vreallocate(size_type capacity) noexcept {
        // clang-format off
        auto old_begin    = begin_;
        auto old_end      = end_;
        auto old_capacity = static_cast<size_type>(end_cap_ - begin_);

        auto [new_begin, new_capacity] = allocate_at_least(allocator_, capacity);
        if constexpr (meta::relocatable<T>) {
            auto new_end = ranges::uninitialized_relocate_no_overlap(old_begin,
                                                                     old_end,
                                                                     new_begin);
        } else if constexpr (meta::nothrow_move_constructible<T>) {
            auto new_end = ranges::uninitialized_move(old_begin, old_end, new_begin);
            destroy_range(old_begin, old_end);
        } else {
            auto new_end = ranges::uninitialized_copy(old_begin, old_end, new_begin);
            destroy_range(old_begin, old_end);
        }
        allocator_traits::deallocate(allocator_, old_begin, old_capacity);
        begin_   = new_begin;
        end_     = new_end;
        end_cap_ = new_begin + new_capacity;
        // clang-format on
    }

    constexpr void vreallocate_assign(size_type capacity, value_type const& value) noexcept {
        auto old_begin    = begin_;
        auto old_capacity = static_cast<size_type>(end_cap_ - begin_);

        auto [new_begin, new_capacity] = allocate_at_least(allocator_, capacity);
        auto count                     = static_cast<difference_type>(capacity);
        auto new_end                   = ranges::uninitialized_fill_n(new_begin, count, value);
        allocator_traits::deallocate(allocator_, old_begin, old_capacity);
        begin_   = new_begin;
        end_     = new_end;
        end_cap_ = new_begin + new_capacity;
    }

    constexpr void vdeallocate() noexcept {
        if (begin_ != nullptr) {
            clear();
            allocator_traits::deallocate(allocator_, begin_, capacity());
            begin_ = end_ = end_cap_ = nullptr;
        }
    }

    template <meta::input_iterator Iterator, meta::sentinel_for<Iterator> Sentinel>
    constexpr void init_with_sentinel(Iterator first, Sentinel last) noexcept {
        if constexpr (meta::forward_iterator<Iterator>) {
            auto const count = ranges::distance(first, last);
            vallocate(static_cast<size_type>(count));
            end_ = ranges::uninitialized_copy_no_overlap(first, last, begin_);
        } else {
            // InputIterators by definition actually only allow you to iterate through them once.
            // Thus the standard *requires* that we do this (inefficient) implementation.
            // Luckily, InputIterators are in practice almost never used, so this code will likely
            // never get executed.
            for (; first != last; ++first) {
                emplace_back(*first);
            }
        }
    }

    constexpr void move_assign_alloc(vector& other, meta::true_type) noexcept {
        vdeallocate();
        allocator_ = ::std::move(other.allocator_);
        // clang-format off
        begin_     = ::std::exchange(other.begin_  , nullptr);
        end_       = ::std::exchange(other.end_    , nullptr);
        end_cap_   = ::std::exchange(other.end_cap_, nullptr);
        // clang-format on
    }
    constexpr void move_assign_alloc(vector& other, meta::false_type) noexcept {
        // clang-format off
        if (allocator_ != other.allocator_) {
            // The rvalue's allocator cannot be moved and is not equal,
            // so we need to individually move each element.            
            if (auto const other_size = other.size(); capacity() < other_size) {
                vdeallocate();
                vallocate(other_size);
            } else {
                clear();
            }

            if constexpr (meta::relocatable<T>) {
                end_       = ranges::uninitialized_relocate_no_overlap(other.begin_,
                                                                       other.end_,
                                                                       begin_);
                other.end_ = other.begin_; // Clear without destroy.
            } else {
                end_ = ranges::uninitialized_move(other.begin_, other.end_, begin_);
                other.clear();
            }
        } else {
            move_assign_alloc(other, meta::true_type{});
        }
        // clang-format on
    }

    constexpr void copy_assign_alloc(vector& other, meta::true_type) noexcept {
        if (allocator_ != other.allocator_) {
            clear();
            allocator_traits::deallocate(allocator_, begin_, capacity());
            begin_ = end_ = end_cap_ = nullptr;
        }
        allocator_ = other.allocator_;
    }
    constexpr void copy_assign_alloc(vector& other, meta::false_type) noexcept {
        (void)other;
    }
};

} // namespace flux::fou