#pragma once
#include <flux/config.hpp>

#include <cstddef>
#include <cstdint>

namespace flux::linux {

extern void* kernel_malloc(::std::size_t, int) noexcept __asm__("__kmalloc");

extern void* kernel_realloc(void const*, ::std::size_t, int) noexcept __asm__("krealloc");

extern void kernel_free(void const*) noexcept __asm__("kfree");

inline constexpr auto gfp_kernel      = 0x400 | 0x800 | 0x40 | 0x80;
inline constexpr auto gfp_kernel_zero = gfp_kernel | 0x100;

} // namespace flux::linux