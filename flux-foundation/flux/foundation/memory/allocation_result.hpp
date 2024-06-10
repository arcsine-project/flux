#pragma once

namespace flux::fou {

#ifdef __cpp_lib_allocate_at_least
template <typename Pointer, typename SizeType = ::std::size_t>
using allocation_result = ::std::allocation_result<Pointer, SizeType>;
#else
// clang-format off
template <typename Pointer, typename SizeType = ::std::size_t>
struct allocation_result final {
    Pointer  ptr;
    SizeType count;
};
// clang-format on
#endif

} // namespace flux::fou