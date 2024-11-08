#pragma once

// clang-format off
#define FLUX_ASSERT_1_ARGS(expression)                                                             \
    (__builtin_expect(static_cast<bool>(expression), 1)                                            \
             ? (void)0                                                                             \
             : ::flux::io::panic(__FILE__ ":" FLUX_TO_STRING(__LINE__)                             \
                                 ": Assertion `" FLUX_TO_STRING(expression) "' failed."))

#define FLUX_ASSERT_2_ARGS(expression, message)                                                    \
    (__builtin_expect(static_cast<bool>(expression), 1)                                            \
             ? (void)0                                                                             \
             : ::flux::io::panic(__FILE__ ":" FLUX_TO_STRING(__LINE__)                             \
                                 ": Assertion `" FLUX_TO_STRING(expression) "' failed: " message "."))
// clang-format on

#define FLUX_ASSERT_GET_3RD_ARG(arg1, arg2, arg3, ...) arg3
#define FLUX_ASSERT_MACRO_CHOOSER(...)                                                             \
    FLUX_ASSERT_GET_3RD_ARG(__VA_ARGS__, FLUX_ASSERT_2_ARGS, FLUX_ASSERT_1_ARGS, )

#ifdef NDEBUG
#    define FLUX_ASSERT(...) ((void)0)
#else
#    define FLUX_ASSERT(...) FLUX_ASSERT_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)
#endif