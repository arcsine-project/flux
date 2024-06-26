#pragma once
#include <flux/foundation/memory/allocator_traits.hpp>

#include <mutex>

namespace flux::fou {

// clang-format off
template <typename Mutex>
concept lockable = requires(Mutex mutex) {
    { mutex.try_lock() } -> meta::same_as<bool>;
    { mutex.    lock() } -> meta::same_as<void>;
    { mutex.  unlock() } -> meta::same_as<void>;
};

// A dummy `mutex` class that does not lock anything. It is a valid `mutex`
// and can be used to disable locking anywhere a `mutex` is requested.
struct [[nodiscard]] no_mutex {
    constexpr void   lock() noexcept {}
    constexpr void unlock() noexcept {}

    constexpr bool try_lock() noexcept {
        return true;
    }
};
// clang-format on

template <typename RawAllocator>
concept thread_safe_allocator = not is_stateful_allocator<RawAllocator>::value;
template <typename Allocator>
inline constexpr bool is_thread_safe_allocator = thread_safe_allocator<Allocator>;

namespace detail {

// clang-format off
template <typename Mutex>
struct [[nodiscard]] mutex_adapter {
    constexpr mutex_adapter() noexcept = default;

    constexpr mutex_adapter(mutex_adapter const&) noexcept {}

    constexpr mutex_adapter& operator=(mutex_adapter const&) noexcept {
        return *this;
    }

    constexpr void lock() const noexcept {
        mutex_.lock();
    }

    constexpr void unlock() const noexcept {
        mutex_.unlock();
    }

    constexpr bool try_lock() const noexcept {
        return mutex_.try_lock();
    }

protected:
    ~mutex_adapter() = default;

    mutable Mutex mutex_;
};

template <>
struct [[nodiscard]] mutex_adapter<no_mutex> {
    mutex_adapter() noexcept = default;

    constexpr void   lock() const noexcept {}
    constexpr void unlock() const noexcept {}

    constexpr bool try_lock() const noexcept {
        return true;
    }

protected:
    ~mutex_adapter() = default;
};

template <typename Mutex>
struct [[nodiscard]] dummy_lock_guard final {
    using mutex_type = Mutex;

    constexpr explicit dummy_lock_guard(Mutex&) noexcept {}

    constexpr dummy_lock_guard(Mutex&, ::std::adopt_lock_t) noexcept {}

    constexpr ~dummy_lock_guard() {}

private:
    dummy_lock_guard(dummy_lock_guard const&)            = delete;
    dummy_lock_guard& operator=(dummy_lock_guard const&) = delete;
};

template <typename Allocator, typename Mutex>
struct [[nodiscard]] locked_allocator {
    constexpr locked_allocator(Allocator& allocator, Mutex& mutex) noexcept
            : allocator_{&allocator}, mutex_{&mutex} {
        mutex_->lock();
    }

    constexpr ~locked_allocator() {
        if (mutex_)
            mutex_->unlock();
    }

    constexpr locked_allocator(locked_allocator&& other) noexcept
            : allocator_{::std::exchange(other.allocator_, nullptr)},
              mutex_    {::std::exchange(other.mutex_    , nullptr)} {}

    constexpr locked_allocator& operator=(locked_allocator&& other) noexcept = delete;

    constexpr Allocator& operator*() const noexcept {
        FLUX_ASSERT(allocator_);
        return *allocator_;
    }

    constexpr Allocator* operator->() const noexcept {
        FLUX_ASSERT(allocator_);
        return allocator_;
    }

private:
    Allocator* allocator_;
    Mutex*     mutex_;
};

template <typename Allocator, typename Mutex>
constexpr auto lock_allocator(Allocator& allocator, Mutex& mutex) noexcept {
    return locked_allocator<Allocator, Mutex>{allocator, mutex};
}
// clang-format on

} // namespace detail

template <typename Allocator, lockable Mutex>
using mutex_for_t = meta::condition<is_thread_safe_allocator<Allocator>, no_mutex, Mutex>;

template <typename Storage, lockable Mutex>
using mutex_adapter_t = detail::mutex_adapter<mutex_for_t<typename Storage::allocator_type, Mutex>>;

template <typename Mutex>
using lock_guard_t = meta::condition<meta::same_as<Mutex, no_mutex>,
                                     detail::dummy_lock_guard<Mutex>, ::std::lock_guard<Mutex>>;

} // namespace flux::fou