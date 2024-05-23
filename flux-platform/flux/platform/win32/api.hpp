#pragma once
#include <flux/config.hpp>

#include <cstddef>
#include <cstdint>

namespace flux::win32 {

// clang-format off
#if (__has_cpp_attribute(__gnu__::__dllimport__) && !defined(__WINE__))
[[__gnu__::__dllimport__]]
#endif
#if (__has_cpp_attribute(__gnu__::__stdcall__) && !defined(__WINE__))
[[__gnu__::__stdcall__]]
#endif
extern void* FLUX_STDCALL GetStdHandle(::std::uint_least32_t) noexcept
#if defined(FLUX_CLANG)
__asm__("GetStdHandle")
#endif
;

#if (__has_cpp_attribute(__gnu__::__dllimport__) && !defined(__WINE__))
[[__gnu__::__dllimport__]]
#endif
#if (__has_cpp_attribute(__gnu__::__stdcall__) && !defined(__WINE__))
[[__gnu__::__stdcall__]]
#endif
extern int FLUX_STDCALL GetConsoleMode(void*, ::std::uint_least32_t*) noexcept
#if defined(FLUX_CLANG) 
__asm__("GetConsoleMode")
#endif
;

#if (__has_cpp_attribute(__gnu__::__dllimport__) && !defined(__WINE__))
[[__gnu__::__dllimport__]]
#endif
#if (__has_cpp_attribute(__gnu__::__stdcall__) && !defined(__WINE__))
[[__gnu__::__stdcall__]]
#endif
extern int FLUX_STDCALL SetConsoleMode(void*, ::std::uint_least32_t) noexcept
#if defined(FLUX_CLANG) 
__asm__("SetConsoleMode")
#endif
;

#if (__has_cpp_attribute(__gnu__::__dllimport__) && !defined(__WINE__))
[[__gnu__::__dllimport__]]
#endif
#if (__has_cpp_attribute(__gnu__::__stdcall__) && !defined(__WINE__))
[[__gnu__::__stdcall__]]
#endif
#if __has_cpp_attribute(__gnu__::__malloc__)
[[__gnu__::__malloc__]]
#endif
extern void* FLUX_STDCALL HeapAlloc(void*, ::std::uint_least32_t, ::std::size_t) noexcept
#if defined(FLUX_CLANG)
__asm__("HeapAlloc")
#endif
;

#if (__has_cpp_attribute(__gnu__::__dllimport__) && !defined(__WINE__))
[[__gnu__::__dllimport__]]
#endif
#if (__has_cpp_attribute(__gnu__::__stdcall__) && !defined(__WINE__))
[[__gnu__::__stdcall__]]
#endif
extern void* FLUX_STDCALL HeapFree(void*, ::std::uint_least32_t, void*) noexcept
#if defined(FLUX_CLANG)
__asm__("HeapFree")
#endif
;

#if (__has_cpp_attribute(__gnu__::__dllimport__) && !defined(__WINE__))
[[__gnu__::__dllimport__]]
#endif
#if (__has_cpp_attribute(__gnu__::__stdcall__) && !defined(__WINE__))
[[__gnu__::__stdcall__]]
#endif
#if __has_cpp_attribute(__gnu__::__const__)
[[__gnu__::__const__]]
#endif
extern void* FLUX_STDCALL GetProcessHeap() noexcept
#if defined(FLUX_CLANG)
__asm__("GetProcessHeap")
#endif
;

#if (__has_cpp_attribute(__gnu__::__dllimport__) && !defined(__WINE__))
[[__gnu__::__dllimport__]]
#endif
#if (__has_cpp_attribute(__gnu__::__stdcall__) && !defined(__WINE__))
[[__gnu__::__stdcall__]]
#endif
extern void* FLUX_STDCALL HeapReAlloc(void*, ::std::uint_least32_t, void*, ::std::size_t) noexcept
#if defined(FLUX_CLANG)
__asm__("HeapReAlloc")
#endif
;
// clang-format on

} // namespace flux::win32