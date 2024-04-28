#pragma once
#include <cstdint>

namespace flux::fou {

struct [[nodiscard]] source_location final {
    static consteval source_location
    current(std::uint_least32_t line          = __builtin_LINE(),
            std::uint_least32_t column        = __builtin_COLUMN(),
            char const*         file_name     = __builtin_FILE(),
            char const*         function_name = __builtin_FUNCTION()) noexcept {
        source_location source{};
        source.line_          = line;
        source.column_        = column;
        source.file_name_     = file_name;
        source.function_name_ = function_name;

        return source;
    }

    constexpr source_location() noexcept = default;

    constexpr std::uint_least32_t line() const noexcept {
        return line_;
    }

    constexpr std::uint_least32_t column() const noexcept {
        return column_;
    }

    constexpr char const* file_name() const noexcept {
        return file_name_;
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

} // namespace flux::fou