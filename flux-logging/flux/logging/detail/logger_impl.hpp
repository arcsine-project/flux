// NOTE:
//  This header is such a mess, it will need to be revisited in the future and probably refactored.
//  Maybe add multithreading support as well. But at this stage, let it be as it is.
// TODO:
//  Revisit it.
#pragma once
#include <flux/logging/detail/source_location.hpp>

#include <string_view>

namespace flux::log {

struct [[nodiscard]] to_file    final {};
struct [[nodiscard]] to_console final {};

struct [[nodiscard]] ansi_color final {
    std::uint8_t r = 255;
    std::uint8_t g = 255;
    std::uint8_t b = 255;
    std::uint8_t a = 255; // << Unused, exists only for alignment.

    struct [[nodiscard]] escape_code final {
        static constexpr std::string_view begin() noexcept {
            return "\033[38;2;";
        }
        static constexpr std::string_view end() noexcept {
            return "\033[0;00m";
        }
    };
};

constexpr auto print_reserve_size(fast_io::io_reserve_type_t<char, ansi_color>) noexcept {
    using namespace fast_io;
    constexpr auto reserve_size = print_reserve_size(io_reserve_type<char, std::uint8_t>);
    constexpr auto total_size   = reserve_size * 3;
    return total_size;
}

constexpr auto print_reserve_define(fast_io::io_reserve_type_t<char, ansi_color>, char* it,
                                    ansi_color color) noexcept {
    using namespace fast_io;
    // clang-format off
    *(it = print_reserve_define(io_reserve_type<char, std::uint8_t>,   it, color.r)) = ';';
    *(it = print_reserve_define(io_reserve_type<char, std::uint8_t>, ++it, color.g)) = ';';
    *(it = print_reserve_define(io_reserve_type<char, std::uint8_t>, ++it, color.b)) = 'm';
    // clang-format on
    return ++it;
}
static_assert(fast_io::reserve_printable<char, ansi_color>);

template <typename T, typename... Args>
constexpr void println(ansi_color color, T&& device, Args&&... args) noexcept {
    io::println(std::forward<T>(device), ansi_color::escape_code::begin(), color,
                std::forward<Args>(args)..., ansi_color::escape_code::end());
}

namespace detail {

// clang-format off
template <typename Output>
struct [[maybe_unused]] dummy_logger;
// clang-format on

} // namespace detail

// clang-format off
template <typename Output>
detail::dummy_logger<Output>& logger() noexcept;
// clang-format on

namespace detail {

inline auto get_local_time() noexcept {
    [[maybe_unused]] static bool once = ([] { fast_io::posix_tzset(); }(), true);
    auto timestamp       = local(fast_io::posix_clock_gettime(fast_io::posix_clock_id::realtime));
    timestamp.subseconds = 0;
    return timestamp;
}

// clang-format off
template <>
struct [[maybe_unused]] dummy_logger<to_file> final {
    constexpr dummy_logger(dummy_logger const&)            = delete;
    constexpr dummy_logger(dummy_logger&&)                 = delete;
    constexpr dummy_logger& operator=(dummy_logger const&) = delete;
    constexpr dummy_logger& operator=(dummy_logger&&)      = delete;

    template <typename... Args>
    constexpr void log(ansi_color color, std::string_view prefix, meta::tuple<Args&&...> tuple,
                       source_location location) noexcept {
        using namespace io;
        [[maybe_unused]] io_flush_guard guard{output_file_};
        meta::apply(
                [&](auto&&... args) {
                    println(color, output_file_, get_local_time(), " ", location, " ", prefix, " ",
                            std::forward<Args>(args)...);
                },
                tuple);
    }

    template <typename Output>
    friend dummy_logger<Output>& flux::log::logger() noexcept;

private:
    constexpr dummy_logger() noexcept : output_file_{"log.ansi", io::open_mode::app} {}
    constexpr ~dummy_logger() = default;

    io::obuf_file output_file_;
};

template <>
struct [[maybe_unused]] dummy_logger<to_console> final {
    constexpr dummy_logger(dummy_logger const&)            = delete;
    constexpr dummy_logger(dummy_logger&&)                 = delete;
    constexpr dummy_logger& operator=(dummy_logger const&) = delete;
    constexpr dummy_logger& operator=(dummy_logger&&)      = delete;

    template <typename... Args>
    constexpr void log(ansi_color color, std::string_view prefix, meta::tuple<Args&&...> tuple,
                       source_location location) noexcept {
        using namespace io;
        meta::apply(
                [&](auto&&... args) noexcept {
                    println(color, out(), get_local_time(), " ", location, " ", prefix, " ",
                            std::forward<Args>(args)...);
                },
                tuple);
    }

    template <typename Output>
    friend dummy_logger<Output>& flux::log::logger() noexcept;

private:
    constexpr dummy_logger() noexcept = default;
    constexpr ~dummy_logger()         = default;
};
// clang-format on

#undef FLUX_PRINT_LOGGER_HEADER_TO

} // namespace detail
} // namespace flux::log