#pragma once

#if __has_cpp_attribute(msvc::no_unique_address)
// MSVC implements [[no_unique_address]] as a silent no-op currently. If/when MSVC breaks its C++
// ABI, it will be changed to work as intended. However, MSVC implements [[msvc::no_unique_address]]
// which does what [[no_unique_address]] is supposed to do, in general.

// Clang-cl does not yet (14.0) implement either [[no_unique_address]] or
// [[msvc::no_unique_address]] though. If/when it does implement [[msvc::no_unique_address]], this
// should be preferred though.
#    define FLUX_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#elif __has_cpp_attribute(no_unique_address)
#    define FLUX_NO_UNIQUE_ADDRESS [[no_unique_address]]
#else
// Note that this can be replaced by #error as soon as clang-cl implements msvc::no_unique_address,
// since there should be no C++20 compiler that doesn't support one of the two attributes at that
// point. I generally don't want to use this macro outside of C++20-only code, because using it
// conditionally in one language version only would make the ABI inconsistent.
#    define FLUX_NO_UNIQUE_ADDRESS /* nothing */
#endif

#if __has_attribute(__no_sanitize__)
#    define FLUX_NO_SANITIZE(...) __attribute__((__no_sanitize__(__VA_ARGS__)))
#else
#    define FLUX_NO_SANITIZE(...)
#endif