#ifndef SNITCH_CONFIG_HPP
#define SNITCH_CONFIG_HPP

#include <version> // for C++ feature check macros

// These are defined from build-time configuration.
// clang-format off
#define SNITCH_VERSION "1.2.4"
#define SNITCH_FULL_VERSION "1.2.4.2f62308"
#define SNITCH_VERSION_MAJOR 1
#define SNITCH_VERSION_MINOR 2
#define SNITCH_VERSION_PATCH 4

#if !defined(SNITCH_MAX_TEST_CASES)
#    define SNITCH_MAX_TEST_CASES 5000
#endif
#if !defined(SNITCH_MAX_NESTED_SECTIONS)
#    define SNITCH_MAX_NESTED_SECTIONS 8
#endif
#if !defined(SNITCH_MAX_EXPR_LENGTH)
#    define SNITCH_MAX_EXPR_LENGTH 1024
#endif
#if !defined(SNITCH_MAX_MESSAGE_LENGTH)
#    define SNITCH_MAX_MESSAGE_LENGTH 1024
#endif
#if !defined(SNITCH_MAX_TEST_NAME_LENGTH)
#    define SNITCH_MAX_TEST_NAME_LENGTH 1024
#endif
#if !defined(SNITCH_MAX_TAG_LENGTH)
#    define SNITCH_MAX_TAG_LENGTH 256
#endif
#if !defined(SNITCH_MAX_CAPTURES)
#    define SNITCH_MAX_CAPTURES 8
#endif
#if !defined(SNITCH_MAX_CAPTURE_LENGTH)
#    define SNITCH_MAX_CAPTURE_LENGTH 256
#endif
#if !defined(SNITCH_MAX_UNIQUE_TAGS)
#    define SNITCH_MAX_UNIQUE_TAGS 1024
#endif
#if !defined(SNITCH_MAX_COMMAND_LINE_ARGS)
#    define SNITCH_MAX_COMMAND_LINE_ARGS 1024
#endif
#if !defined(SNITCH_MAX_REGISTERED_REPORTERS)
#    define SNITCH_MAX_REGISTERED_REPORTERS 8
#endif
#if !defined(SNITCH_MAX_PATH_LENGTH)
#    define SNITCH_MAX_PATH_LENGTH 1024
#endif
#if !defined(SNITCH_DEFINE_MAIN)
#define SNITCH_DEFINE_MAIN 1
#endif
#if !defined(SNITCH_WITH_EXCEPTIONS)
#define SNITCH_WITH_EXCEPTIONS 1
#endif
#if !defined(SNITCH_WITH_TIMINGS)
#define SNITCH_WITH_TIMINGS 1
#endif
#if !defined(SNITCH_WITH_SHORTHAND_MACROS)
#define SNITCH_WITH_SHORTHAND_MACROS 1
#endif
#if !defined(SNITCH_DEFAULT_WITH_COLOR)
#define SNITCH_DEFAULT_WITH_COLOR 1
#endif
#if !defined(SNITCH_CONSTEXPR_FLOAT_USE_BITCAST)
#define SNITCH_CONSTEXPR_FLOAT_USE_BITCAST 1
#endif
#if !defined(SNITCH_DECOMPOSE_SUCCESSFUL_ASSERTIONS)
#define SNITCH_DECOMPOSE_SUCCESSFUL_ASSERTIONS 0
#endif
#if !defined(SNITCH_WITH_ALL_REPORTERS)
#define SNITCH_WITH_ALL_REPORTERS 1
#endif
#if !defined(SNITCH_WITH_TEAMCITY_REPORTER)
#define SNITCH_WITH_TEAMCITY_REPORTER 0
#endif
#if !defined(SNITCH_WITH_CATCH2_REPORTER)
#define SNITCH_WITH_CATCH2_REPORTER 0
#endif
#if !defined(SNITCH_SHARED_LIBRARY)
#define SNITCH_SHARED_LIBRARY 0
#endif
// clang-format on

#if defined(_MSC_VER)
#    if defined(_KERNEL_MODE) || (defined(_HAS_EXCEPTIONS) && !_HAS_EXCEPTIONS)
#        define SNITCH_EXCEPTIONS_NOT_AVAILABLE
#    endif
#elif defined(__clang__) || defined(__GNUC__)
#    if !defined(__EXCEPTIONS)
#        define SNITCH_EXCEPTIONS_NOT_AVAILABLE
#    endif
#endif

#if defined(SNITCH_EXCEPTIONS_NOT_AVAILABLE)
#    undef SNITCH_WITH_EXCEPTIONS
#    define SNITCH_WITH_EXCEPTIONS 0
#endif

#if !defined(__cpp_lib_bit_cast)
#    undef SNITCH_CONSTEXPR_FLOAT_USE_BITCAST
#    define SNITCH_CONSTEXPR_FLOAT_USE_BITCAST 0
#endif

#if SNITCH_SHARED_LIBRARY
#    if defined(_MSC_VER)
#        if defined(SNITCH_EXPORTS)
#            define SNITCH_EXPORT __declspec(dllexport)
#        else
#            define SNITCH_EXPORT __declspec(dllimport)
#        endif
#    elif defined(__clang__) || defined(__GNUC__)
#        define SNITCH_EXPORT [[gnu::visibility("default")]]
#    else
#        define SNITCH_EXPORT
#    endif
#else
#    define SNITCH_EXPORT
#endif

#endif
