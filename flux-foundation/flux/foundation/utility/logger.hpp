#pragma once
#include <flux/io.hpp>

#include <flux/foundation/utility/detail/logger_impl.hpp>
#include <flux/foundation/utility/terminate.hpp>

namespace flux::fou {

namespace out {

inline constexpr to_file    file    = {};
inline constexpr to_console console = {};

} // namespace out

namespace color {

// clang-format off
inline constexpr ansi_color cyan   = {0  , 200, 240, 255};
inline constexpr ansi_color white  = {255, 255, 255, 255};
inline constexpr ansi_color yellow = {231, 231, 31 , 255};
inline constexpr ansi_color red    = {231, 31 , 31 , 255};
// clang-format on

} // namespace color

// clang-format off
template <typename Output>
detail::dummy_logger<Output>& logger() noexcept {
    static detail::dummy_logger<Output> instance;
    return instance;
}
// clang-format on

// clang-format off
template <typename... Args> struct [[maybe_unused]] trace final {
    // Explicitly writes the trace log to the console.
    constexpr explicit trace(
            [[maybe_unused]] to_console      output,
            [[maybe_unused]] Args&&...       args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept {
        logger<to_console>().log(color::cyan, "[TRACE]", std::forward_as_tuple(std::move(args)...),
                                 location);
    }

    // Explicitly writes the trace log to the file.
    constexpr explicit trace(
            [[maybe_unused]] to_file         output,
            [[maybe_unused]] Args&&...       args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept {
        logger<to_file>().log(color::cyan, "[TRACE]", std::forward_as_tuple(std::move(args)...),
                              location);
    }

    // Writes the trace log to the console by default.
    constexpr explicit trace(
            [[maybe_unused]] Args&&... args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept
            : trace(out::console, std::forward<Args>(args)..., location) {}
};

template <typename... Args> struct [[maybe_unused]] debug final {
    // Explicitly writes the debug log to the console.
    constexpr explicit debug(
            [[maybe_unused]] to_console      output,
            [[maybe_unused]] Args&&...       args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept {
        logger<to_console>().log(color::white, "[DEBUG]", std::forward_as_tuple(std::move(args)...),
                                 location);
    }

    // Explicitly writes the debug log to the file.
    constexpr explicit debug(
            [[maybe_unused]] to_file         output,
            [[maybe_unused]] Args&&...       args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept {
        logger<to_file>().log(color::white, "[DEBUG]", std::forward_as_tuple(std::move(args)...),
                              location);
    }

    // Writes the debug log to the console by default.
    constexpr explicit debug(
            [[maybe_unused]] Args&&... args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept
            : debug(out::console, std::forward<Args>(args)..., location) {}
};

template <typename... Args> struct [[maybe_unused]] warning final {
    // Explicitly writes the warning log to the console.
    constexpr explicit warning(
            [[maybe_unused]] to_console      output,
            [[maybe_unused]] Args&&...       args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept {
        logger<to_console>().log(color::yellow, "[WARNING]", std::forward_as_tuple(std::move(args)...),
                                 location);
    }

    // Explicitly writes the warning log to the file.
    constexpr explicit warning(
            [[maybe_unused]] to_file         output,
            [[maybe_unused]] Args&&...       args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept {
        logger<to_file>().log(color::yellow, "[WARNING]", std::forward_as_tuple(std::move(args)...),
                              location);
    }

    // Writes the warning log to the console by default.
    constexpr explicit warning(
            [[maybe_unused]] Args&&... args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept
            : warning(out::console, std::forward<Args>(args)..., location) {}
};

template <typename... Args> struct [[maybe_unused]] error final {
    // Explicitly writes the error log to the console.
    constexpr explicit error(
            [[maybe_unused]] to_console      output,
            [[maybe_unused]] Args&&...       args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept {
        logger<to_console>().log(color::red, "[ERROR]", std::forward_as_tuple(std::move(args)...),
                                 location);
        fast_terminate();
    }

    // Explicitly writes the error log to the file.
    constexpr explicit error(
            [[maybe_unused]] to_file         output,
            [[maybe_unused]] Args&&...       args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept {
        logger<to_file>().log(color::red, "[ERROR]", std::forward_as_tuple(std::move(args)...),
                              location);
        fast_terminate();
    }

    // Writes the error log to the console by default.
    constexpr explicit error(
            [[maybe_unused]] Args&&... args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept
            : error(out::console, std::forward<Args>(args)..., location) {}
};
// clang-format on

// clang-format off
template <typename... Args> trace  (to_console, Args&&...) -> trace  <Args...>;
template <typename... Args> debug  (to_console, Args&&...) -> debug  <Args...>;
template <typename... Args> warning(to_console, Args&&...) -> warning<Args...>;
template <typename... Args> error  (to_console, Args&&...) -> error  <Args...>;

template <typename... Args> trace  (to_file, Args&&...) -> trace  <Args...>;
template <typename... Args> debug  (to_file, Args&&...) -> debug  <Args...>;
template <typename... Args> warning(to_file, Args&&...) -> warning<Args...>;
template <typename... Args> error  (to_file, Args&&...) -> error  <Args...>;

template <typename... Args> trace  (Args&&...) -> trace  <Args...>;
template <typename... Args> debug  (Args&&...) -> debug  <Args...>;
template <typename... Args> warning(Args&&...) -> warning<Args...>;
template <typename... Args> error  (Args&&...) -> error  <Args...>;
// clang-format on

} // namespace flux::fou

namespace flux::log {

namespace out {
using flux::fou::out::console;
using flux::fou::out::file;
} // namespace out

using flux::fou::debug;
using flux::fou::error;
using flux::fou::trace;
using flux::fou::warning;

} // namespace flux::log