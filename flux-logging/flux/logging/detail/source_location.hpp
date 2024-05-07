#pragma once
#include <flux/logging/detail/strip_path.hpp>

namespace flux::log {

// This is a custom implementation of `source_location` specifically for logging.
// If you want standard behavior please use `std::source_location`.
struct [[nodiscard]] source_location final {
    static consteval source_location
    current(std::uint_least32_t line          = __builtin_LINE(),
            std::uint_least32_t column        = __builtin_COLUMN(),
            char const*         file_name     = __builtin_FILE(),
            char const*         function_name = __builtin_FUNCTION()) noexcept {
        source_location location{};
        location.line_          = line;
        location.column_        = column;
        location.file_name_     = file_name;
        location.function_name_ = function_name;

        return location;
    }

    constexpr source_location() noexcept = default;

    constexpr std::uint_least32_t line() const noexcept {
        return line_;
    }

    constexpr std::uint_least32_t column() const noexcept {
        return column_;
    }

    constexpr char const* file_name() const noexcept {
        return detail::strip_path(file_name_);
    }

    constexpr char const* function_name() const noexcept {
        return function_name_;
    }

private:
    std::uint_least32_t line_          = {};
    std::uint_least32_t column_        = {};
    char const*         file_name_     = "";
    char const*         function_name_ = "";
};

} // namespace flux::log

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
print_alias_define_source_location_impl(flux::log::source_location location) noexcept {
    return {{location.file_name(), cstr_len(location.file_name())},
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
print_alias_define(io_alias_t, flux::log::source_location location) noexcept {
    return details::print_alias_define_source_location_impl(location);
}

namespace manipulators {
inline constexpr auto
cur_src_loc(flux::log::source_location location = flux::log::source_location::current()) noexcept {
    return location;
}
} // namespace manipulators
} // namespace fast_io
