// NOTE:
//  This header is such a mess, it will need to be revisited in the future and probably refactored.
//  Maybe add multithreading support as well. But at this stage, let it be as it is.
// TODO:
//  Revisit it.
#pragma once

#include <flux/foundation/utility/detail/strip_path.hpp>
#include <flux/foundation/utility/source_location.hpp>

#include <cstddef>
#include <string_view>

namespace fast_io {

struct flux_source_location_scatter {
    basic_io_scatter_t<char> file_name;
    basic_io_scatter_t<char> function_name;
    std::uint_least32_t      line;
    std::uint_least32_t      column;
};

namespace details {

inline constexpr std::size_t
print_reserve_size_source_location_impl(flux_source_location_scatter location) noexcept {
    constexpr auto reserve_size = print_reserve_size(io_reserve_type<char, std::uint_least32_t>);
    constexpr auto total_size   = (reserve_size * 2 + 3);
    return intrinsics::add_or_overflow_die_chain(location.file_name.len, location.function_name.len,
                                                 total_size);
}

inline constexpr char*
print_reserve_define_source_location_impl(char*                        it,
                                          flux_source_location_scatter location) noexcept {
    constexpr auto io_reserve = io_reserve_type<char, std::uint_least32_t>;
    *(it = non_overlapped_copy_n(location.file_name.base, location.file_name.len, it)) = ':';
    *(it = print_reserve_define(io_reserve, ++it, location.line))                      = ':';
    *(it = print_reserve_define(io_reserve, ++it, location.column))                    = ':';
    return non_overlapped_copy_n(location.function_name.base, location.function_name.len, ++it);
}

inline constexpr flux_source_location_scatter
print_alias_define_source_location_impl(flux::fou::source_location location) noexcept {
    using flux::fou::detail::strip_path;
    return {{strip_path(location.file_name()), cstr_len(strip_path(location.file_name()))},
            {location.function_name(), cstr_len(location.function_name())},
            location.line(),
            location.column()};
}

} // namespace details

inline constexpr std::size_t
print_reserve_size(io_reserve_type_t<char, flux_source_location_scatter>,
                   flux_source_location_scatter location) noexcept {
    return details::print_reserve_size_source_location_impl(location);
}

inline constexpr char* print_reserve_define(io_reserve_type_t<char, flux_source_location_scatter>,
                                            char*                        iter,
                                            flux_source_location_scatter location) noexcept {
    return details::print_reserve_define_source_location_impl(iter, location);
}

inline constexpr flux_source_location_scatter
print_alias_define(io_alias_t, flux::fou::source_location location) noexcept {
    return details::print_alias_define_source_location_impl(location);
}

namespace manipulators {
inline constexpr auto
cur_src_loc(flux::fou::source_location location = flux::fou::source_location::current()) noexcept {
    return location;
}
} // namespace manipulators
} // namespace fast_io

namespace flux::fou {

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
    return local(fast_io::posix_clock_gettime(fast_io::posix_clock_id::realtime));
}

#define FLUX_PRINT_LOGGER_HEADER_TO(output_file)                                                   \
    io::println(output_file, left("+", 36, '-'), left("+", 63, '-'), left("+", 13, '-'),           \
                left("+", 32, '-'));                                                               \
    io::println(output_file, "| ", left("Date / Time", 33), " | ", left("Location", 60), " | ",    \
                left("Level", 10), " | Message");                                                  \
    io::println(output_file, left("+", 36, '-'), left("+", 63, '-'), left("+", 13, '-'),           \
                left("+", 32, '-'));

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
                    println(color, output_file_, "  ", left(get_local_time(), 36),
                            left(location, 63), left(prefix, 13), std::forward<Args>(args)...);
                },
                tuple);
    }

    template <typename Output>
    friend dummy_logger<Output>& flux::fou::logger() noexcept;

private:
    constexpr dummy_logger() noexcept : output_file_{"log.ansi", io::open_mode::app} {
        using namespace io;
        FLUX_PRINT_LOGGER_HEADER_TO(output_file_)
    }
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
                    println(color, out(), left(get_local_time(), 34), location, " ", prefix, " ",
                            std::forward<Args>(args)...);
                },
                tuple);
    }

    template <typename Output>
    friend dummy_logger<Output>& flux::fou::logger() noexcept;

private:
    constexpr dummy_logger() noexcept = default;
    constexpr ~dummy_logger()         = default;
};
// clang-format on

#undef FLUX_PRINT_LOGGER_HEADER_TO

} // namespace detail
} // namespace flux::fou