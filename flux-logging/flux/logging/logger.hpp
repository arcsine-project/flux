#pragma once
#include <flux/logging/detail/logger_impl.hpp>

namespace flux::log {

namespace out {

inline constexpr to_file    file    = {};
inline constexpr to_console console = {};

} // namespace out

namespace color {

// clang-format off
inline constexpr ansi_color cyan    = {0  , 200, 240, 255};
inline constexpr ansi_color gray    = {204, 204, 204, 255};
inline constexpr ansi_color yellow  = {231, 231, 31 , 255};
inline constexpr ansi_color red     = {209, 0  , 0  , 255};
inline constexpr ansi_color green   = {181, 231, 31 , 255};
inline constexpr ansi_color purple  = {119, 89 , 181, 255};
inline constexpr ansi_color crimson = {220, 20 , 60 , 255};
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
        logger<to_console>().log(color::purple, "[DEBUG]", std::forward_as_tuple(std::move(args)...),
                                 location);
    }

    // Explicitly writes the debug log to the file.
    constexpr explicit debug(
            [[maybe_unused]] to_file         output,
            [[maybe_unused]] Args&&...       args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept {
        logger<to_file>().log(color::purple, "[DEBUG]", std::forward_as_tuple(std::move(args)...),
                              location);
    }

    // Writes the debug log to the console by default.
    constexpr explicit debug(
            [[maybe_unused]] Args&&... args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept
            : debug(out::console, std::forward<Args>(args)..., location) {}
};

template <typename... Args> struct [[maybe_unused]] info final {
    // Explicitly writes the info log to the console.
    constexpr explicit info(
            [[maybe_unused]] to_console      output,
            [[maybe_unused]] Args&&...       args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept {
        logger<to_console>().log(color::gray, "[INFO]", std::forward_as_tuple(std::move(args)...),
                                 location);
    }

    // Explicitly writes the info log to the file.
    constexpr explicit info(
            [[maybe_unused]] to_file         output,
            [[maybe_unused]] Args&&...       args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept {
        logger<to_file>().log(color::gray, "[INFO]", std::forward_as_tuple(std::move(args)...),
                              location);
    }

    // Writes the info log to the console by default.
    constexpr explicit info(
            [[maybe_unused]] Args&&... args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept
            : info(out::console, std::forward<Args>(args)..., location) {}
};

template <typename... Args> struct [[maybe_unused]] warn final {
    // Explicitly writes the warning log to the console.
    constexpr explicit warn(
            [[maybe_unused]] to_console      output,
            [[maybe_unused]] Args&&...       args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept {
        logger<to_console>().log(color::yellow, "[WARNING]", std::forward_as_tuple(std::move(args)...),
                                 location);
    }

    // Explicitly writes the warning log to the file.
    constexpr explicit warn(
            [[maybe_unused]] to_file         output,
            [[maybe_unused]] Args&&...       args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept {
        logger<to_file>().log(color::yellow, "[WARNING]", std::forward_as_tuple(std::move(args)...),
                              location);
    }

    // Writes the warning log to the console by default.
    constexpr explicit warn(
            [[maybe_unused]] Args&&... args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept
            : warn(out::console, std::forward<Args>(args)..., location) {}
};

template <typename... Args> struct [[maybe_unused]] error final {
    // Explicitly writes the error log to the console.
    constexpr explicit error(
            [[maybe_unused]] to_console      output,
            [[maybe_unused]] Args&&...       args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept {
        logger<to_console>().log(color::red, "[ERROR]", std::forward_as_tuple(std::move(args)...),
                                 location);
    }

    // Explicitly writes the error log to the file.
    constexpr explicit error(
            [[maybe_unused]] to_file         output,
            [[maybe_unused]] Args&&...       args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept {
        logger<to_file>().log(color::red, "[ERROR]", std::forward_as_tuple(std::move(args)...),
                              location);
    }

    // Writes the error log to the console by default.
    constexpr explicit error(
            [[maybe_unused]] Args&&... args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept
            : error(out::console, std::forward<Args>(args)..., location) {}
};

template <typename... Args> struct [[maybe_unused]] fatal final {
    // Explicitly writes the error log to the console.
    constexpr explicit fatal(
            [[maybe_unused]] to_console      output,
            [[maybe_unused]] Args&&...       args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept {
        logger<to_console>().log(color::crimson, "[FATAL]", std::forward_as_tuple(std::move(args)...),
                                 location);
        fou::fast_terminate();
    }

    // Explicitly writes the error log to the file.
    constexpr explicit fatal(
            [[maybe_unused]] to_file         output,
            [[maybe_unused]] Args&&...       args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept {
        logger<to_file>().log(color::crimson, "[FATAL]", std::forward_as_tuple(std::move(args)...),
                              location);
        fou::fast_terminate();
    }

    // Writes the error log to the console by default.
    constexpr explicit fatal(
            [[maybe_unused]] Args&&... args,
            [[maybe_unused]] source_location location = source_location::current()) noexcept
            : fatal(out::console, std::forward<Args>(args)..., location) {}
};
// clang-format on

// clang-format off
template <typename... Args> trace(to_console, Args&&...) -> trace<Args...>;
template <typename... Args> debug(to_console, Args&&...) -> debug<Args...>;
template <typename... Args> info (to_console, Args&&...) -> info <Args...>;
template <typename... Args> warn (to_console, Args&&...) -> warn <Args...>;
template <typename... Args> error(to_console, Args&&...) -> error<Args...>;
template <typename... Args> fatal(to_console, Args&&...) -> fatal<Args...>;

template <typename... Args> trace(to_file, Args&&...) -> trace<Args...>;
template <typename... Args> debug(to_file, Args&&...) -> debug<Args...>;
template <typename... Args> info (to_file, Args&&...) -> info <Args...>;
template <typename... Args> warn (to_file, Args&&...) -> warn <Args...>;
template <typename... Args> error(to_file, Args&&...) -> error<Args...>;
template <typename... Args> fatal(to_file, Args&&...) -> fatal<Args...>;

template <typename... Args> trace(Args&&...) -> trace<Args...>;
template <typename... Args> debug(Args&&...) -> debug<Args...>;
template <typename... Args> info (Args&&...) -> info <Args...>;
template <typename... Args> warn (Args&&...) -> warn <Args...>;
template <typename... Args> error(Args&&...) -> error<Args...>;
template <typename... Args> fatal(Args&&...) -> fatal<Args...>;
// clang-format on

} // namespace flux::log