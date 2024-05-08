#pragma once

#define FLUX_TARGET_APPLE   (0)
#define FLUX_TARGET_MACOSX  (0)
#define FLUX_TARGET_WINDOWS (0)
#define FLUX_TARGET_LINUX   (0)

// clang-format off
#if defined(_WIN64) && !defined(__WINE__)
#    undef  FLUX_TARGET_WINDOWS
#    define FLUX_TARGET_WINDOWS (1)
#elif defined(__APPLE__)
#    undef  FLUX_TARGET_APPLE
#    define FLUX_TARGET_APPLE (1)
#    include <TargetConditionals.h>
#    if defined(TARGET_OS_MAC)
#        undef  FLUX_TARGET_MACOSX
#        define FLUX_TARGET_MACOSX (1)
#    endif
#elif defined(__linux__)
#    undef  FLUX_TARGET_LINUX
#    define FLUX_TARGET_LINUX (1)
#endif
// clang-format on

#define FLUX_TARGET(X) FLUX_JOIN(FLUX_TARGET_, X)