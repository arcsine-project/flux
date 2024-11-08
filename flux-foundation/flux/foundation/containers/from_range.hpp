#pragma once

namespace flux::fou {

#ifdef __cpp_lib_containers_ranges
using from_range_t = ::std::from_range_t;
using ::std::from_range;
#else
struct from_range_t final {
    explicit from_range_t() = default;
};
inline constexpr from_range_t from_range{};
#endif

} // namespace flux::fou