#pragma once
#include <flux/foundation/memory/threading.hpp>

namespace flux::fou {

namespace detail {

// clang-format off
template <typename AllocatorReference>
concept any_reference = meta::same_as<AllocatorReference, any_allocator_reference>;
// clang-format on

} // namespace detail

template <typename Derived, typename Base>
concept not_derived_from = not meta::derived_from<Derived, Base>;

template <typename T, raw_allocator RawAllocator>
class [[nodiscard]] std_allocator_adapter : protected allocator_reference<RawAllocator> {
    using allocator_reference = allocator_reference<RawAllocator>;
    using allocator_traits    = allocator_traits<RawAllocator>;
    using propagation_traits  = propagation_traits<RawAllocator>;

public:
    using allocator_type  = typename allocator_reference::allocator_type;
    using value_type      = T;
    using size_type       = ::std::size_t;
    using difference_type = ::std::ptrdiff_t;

    // clang-format off
    using propagate_on_container_swap            =
            typename propagation_traits::propagate_on_container_swap;
    using propagate_on_container_move_assignment =
            typename propagation_traits::propagate_on_container_move_assignment;
    using propagate_on_container_copy_assignment =
            typename propagation_traits::propagate_on_container_move_assignment;

    constexpr std_allocator_adapter() noexcept
        requires(not is_stateful_allocator<RawAllocator>::value)
            : allocator_reference{allocator_type{}} {}

    constexpr explicit(false) std_allocator_adapter(allocator_reference const& allocator) noexcept
            : allocator_reference{allocator} {}

    template <typename Allocator>
        requires not_derived_from<Allocator, std_allocator_adapter>
    constexpr explicit(false) std_allocator_adapter(Allocator& allocator) noexcept
            : allocator_reference{allocator} {}

    template <typename Allocator>
        requires not_derived_from<Allocator, std_allocator_adapter>
    constexpr explicit(false) std_allocator_adapter(Allocator const& allocator) noexcept
            : allocator_reference{allocator} {}

    template <typename U>
    constexpr std_allocator_adapter(std_allocator_adapter<U, RawAllocator>& allocator) noexcept
            : allocator_reference{allocator} {}

    template <typename U>
    constexpr std_allocator_adapter(std_allocator_adapter<U, RawAllocator> const& allocator) noexcept
            : allocator_reference{allocator} {}
    // clang-format on

    // Implicit conversion from any other `allocator_storage` is forbidden to prevent accidentally
    // wrapping another `allocator_storage` inside the `allocator_reference`.
    template <typename Storage, typename Mutex>
    constexpr std_allocator_adapter(allocator_storage<Storage, Mutex>&) = delete;

    [[nodiscard]] constexpr T* allocate(size_type n) noexcept {
        if consteval {
            return static_cast<T*>(::operator new(n * sizeof(T)));
        } else {
            if constexpr (detail::any_reference<allocator_reference>) {
                return static_cast<T*>(any_allocate_impl(n));
            } else {
                return static_cast<T*>(allocate_impl(n));
            }
        }
    }

    [[nodiscard]] constexpr allocation_result<T*> allocate_at_least(size_type n) noexcept {
        return {allocate(n), n};
    }

    constexpr void deallocate(T* ptr, size_type n) noexcept {
        if consteval {
            ::operator delete(ptr);
        } else {
            if constexpr (detail::any_reference<allocator_reference>) {
                any_deallocate_impl(ptr, n);
            } else {
                deallocate_impl(ptr, n);
            }
        }
    }

    constexpr auto select_on_container_copy_construction() const noexcept {
        return propagation_traits::select_on_container_copy_construction(*this);
    }

    constexpr decltype(auto) allocator() noexcept {
        return allocator_reference::allocator();
    }
    constexpr decltype(auto) allocator() const noexcept {
        return allocator_reference::allocator();
    }

    template <typename T1, typename T2, typename Allocator>
    friend constexpr bool operator==(std_allocator_adapter<T1, Allocator> const&,
                                     std_allocator_adapter<T2, Allocator> const&) noexcept;

private:
#if __has_cpp_attribute(__gnu__::__always_inline__)
    [[__gnu__::__always_inline__]]
#endif
    constexpr void* allocate_impl(size_type n) noexcept {
        if (1u == n) {
            return allocator_reference::allocate_node(sizeof(T), alignof(T));
        }
        return allocator_reference::allocate_array(n, sizeof(T), alignof(T));
    }

#if __has_cpp_attribute(__gnu__::__always_inline__)
    [[__gnu__::__always_inline__]]
#endif
    constexpr void deallocate_impl(void* ptr, size_type n) noexcept {
        if (1u == n) {
            return allocator_reference::deallocate_node(ptr, sizeof(T), alignof(T));
        }
        return allocator_reference::deallocate_array(ptr, n, sizeof(T), alignof(T));
    }

#if __has_cpp_attribute(__gnu__::__always_inline__)
    [[__gnu__::__always_inline__]]
#endif
    constexpr void* any_allocate_impl(size_type n) noexcept {
        if (1u == n) {
            return allocator().allocate_node(sizeof(T), alignof(T));
        }
        return allocator().allocate_array(n, sizeof(T), alignof(T));
    }

#if __has_cpp_attribute(__gnu__::__always_inline__)
    [[__gnu__::__always_inline__]]
#endif
    constexpr void any_deallocate_impl(void* ptr, size_type n) noexcept {
        if (1u == n) {
            return allocator().deallocate_node(ptr, sizeof(T), alignof(T));
        }
        return allocator().deallocate_array(ptr, n, sizeof(T), alignof(T));
    }

    // clang-format off
    template <typename U, raw_allocator OtherRawAllocator>
    friend class std_allocator_adapter;
    // clang-format on
};

template <typename T, typename Allocator>
class [[nodiscard]] std_allocator_adapter<T, detail::low_level_allocator_adapter<Allocator>>
        : detail::low_level_allocator_adapter<Allocator> {
    using low_level_allocator = detail::low_level_allocator_adapter<Allocator>;
    using propagation_traits  = propagation_traits<low_level_allocator>;

public:
    using value_type      = T;
    using size_type       = ::std::size_t;
    using difference_type = ::std::ptrdiff_t;

    using propagate_on_container_swap            = meta::false_type;
    using propagate_on_container_move_assignment = meta::true_type;
    using propagate_on_container_copy_assignment = meta::false_type;

    constexpr std_allocator_adapter() noexcept = default;

    template <typename U>
    constexpr std_allocator_adapter(std_allocator_adapter<U, Allocator> const&) noexcept {}

    constexpr std_allocator_adapter(std_allocator_adapter const& allocator) noexcept  = default;
    constexpr std_allocator_adapter& operator=(std_allocator_adapter const&) noexcept = default;

    constexpr ~std_allocator_adapter() = default;

    [[nodiscard]] constexpr T* allocate(size_type n) noexcept {
        if consteval {
            return static_cast<T*>(::operator new(n * sizeof(T)));
        } else {
            return static_cast<T*>(low_level_allocator::allocate(n * sizeof(T)));
        }
    }

    [[nodiscard]] constexpr allocation_result<T*> allocate_at_least(size_type n) noexcept {
        if consteval {
            return {allocate(n), n};
        } else {
            auto [ptr, count] = low_level_allocator::allocate_at_least(n * sizeof(T));
            return {static_cast<T*>(ptr), count / sizeof(T)};
        }
    }

    constexpr void deallocate(T* ptr, size_type n) noexcept {
        if consteval {
            ::operator delete(ptr);
        } else {
            low_level_allocator::deallocate((void*)ptr, n * sizeof(T));
        }
    }

    constexpr auto select_on_container_copy_construction() const noexcept {
        return propagation_traits::select_on_container_copy_construction(*this);
    }

    constexpr std_allocator_adapter& allocator() noexcept {
        return *this;
    }
    constexpr std_allocator_adapter const& allocator() const noexcept {
        return *this;
    }
};

template <typename T, typename U, typename Allocator>
constexpr bool operator==(std_allocator_adapter<T, Allocator> const& lhs,
                          std_allocator_adapter<U, Allocator> const& rhs) noexcept {
    if constexpr (is_shared_allocator<Allocator>::value)
        return lhs.allocator() == rhs.allocator();
    else if constexpr (is_stateful_allocator<Allocator>::value)
        return &lhs.allocator() == &rhs.allocator();
    else
        return true;
}

// clang-format off
template <typename T>
using std_any_allocator = std_allocator_adapter<T, any_allocator>;
// clang-format on

template <typename T, typename Allocator>
struct [[nodiscard]] allocator_traits<std_allocator_adapter<T, Allocator>> final {
    using allocator_type  = std_allocator_adapter<T, Allocator>;
    using size_type       = typename allocator_type::size_type;
    using difference_type = typename allocator_type::difference_type;

    // clang-format off
    using propagate_on_container_swap            =
            typename allocator_type::propagate_on_container_swap;
    using propagate_on_container_move_assignment =
            typename allocator_type::propagate_on_container_move_assignment;
    using propagate_on_container_copy_assignment =
            typename allocator_type::propagate_on_container_copy_assignment;
    // clang-format on

    static constexpr T* allocate(allocator_type& allocator, size_type n) noexcept {
        return allocator.allocate(n);
    }

    static constexpr allocation_result<T*> allocate_at_least(allocator_type& allocator,
                                                             size_type       n) noexcept {
        if constexpr (requires { allocator.allocate_at_least(n); }) {
            return allocator.allocate_at_least(n);
        } else {
            return {allocator.allocate(n), n};
        }
    }

    static constexpr void deallocate(allocator_type& allocator, T* ptr, size_type n) noexcept {
        return allocator.deallocate(ptr, n);
    }

    static constexpr size_type max_size(allocator_type const& allocator) noexcept {
        if constexpr (requires { allocator.max_size(); }) {
            return allocator.max_size();
        } else {
            return ULLONG_MAX / sizeof(T);
        }
    }

    static constexpr allocator_type
    select_on_container_copy_construction(allocator_type const& allocator) noexcept {
        if constexpr (detail::has_select_on_container_copy_construction<allocator_type>) {
            return allocator.select_on_container_copy_construction();
        } else {
            return allocator;
        }
    }
};

} // namespace flux::fou