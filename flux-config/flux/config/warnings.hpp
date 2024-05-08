#pragma once

// clang-format off
#if defined(FLUX_CLANG)
#    define _FLUX_PRAGMA(diagnostic)                     _Pragma(FLUX_TO_STRING(diagnostic))
#    define FLUX_DISABLE_WARNING_PUSH                    _FLUX_PRAGMA(clang diagnostic push)
#    define FLUX_DISABLE_WARNING_POP                     _FLUX_PRAGMA(clang diagnostic pop)
#    define FLUX_DISABLE_WARNING(warning)                _FLUX_PRAGMA(clang diagnostic ignored #warning)
#    define FLUX_DISABLE_WARNING_BLOCK(warning, ...)                                                        \
         FLUX_DISABLE_WARNING_PUSH                                                                          \
         FLUX_DISABLE_WARNING(warning)                                                                      \
         __VA_ARGS__                                                                                        \
         FLUX_DISABLE_WARNING_POP
// Warnings you want to deactivate:
#    define FLUX_DISABLE_WARNING_DEPRECATED_DECLARATIONS FLUX_DISABLE_WARNING(-Wdeprecated-declarations)
#else
#    define FLUX_DISABLE_WARNING_PUSH                    /* nothing */
#    define FLUX_DISABLE_WARNING_POP                     /* nothing */
#    define FLUX_DISABLE_WARNING(warning)                /* nothing */
#    define FLUX_DISABLE_WARNING_BLOCK(warning, ...)     /* nothing */
#    define FLUX_DISABLE_WARNING_DEPRECATED_DECLARATIONS /* nothing */
#endif
// clang-format on