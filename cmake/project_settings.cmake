include(CTest)
include(CheckLanguage)

check_language(OBJC)
if(CMAKE_OBJC_COMPILER)
    enable_language(OBJC)
    string(APPEND CMAKE_OBJC_FLAGS " -fobjc-arc -std=gnu11")
endif()

check_language(OBJCXX)
if(CMAKE_OBJCXX_COMPILER)
    enable_language(OBJCXX)
    string(APPEND CMAKE_OBJCXX_FLAGS " -fobjc-arc -std=gnu++23")
endif()

add_library(flux::project_settings INTERFACE IMPORTED)

target_compile_features(flux::project_settings INTERFACE cxx_std_23)

# Enable output of compile commands during generation. This file will be used by clangd.
if(NOT DEFINED CMAKE_EXPORT_COMPILE_COMMANDS AND NOT DEFINED ENV{CMAKE_EXPORT_COMPILE_COMMANDS})
  set(CMAKE_EXPORT_COMPILE_COMMANDS "ON" CACHE BOOL "Enable/Disable output of compile commands during generation.")
  mark_as_advanced(CMAKE_EXPORT_COMPILE_COMMANDS)

  message(STATUS "CMAKE_EXPORT_COMPILE_COMMANDS: ${CMAKE_EXPORT_COMPILE_COMMANDS}")
endif()

# Let CMake know where to find custom modules.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

#-----------------------------------------------------------------------------------------------------------------------
# Detect and initialize the target platform.
#-----------------------------------------------------------------------------------------------------------------------

# This function is required to get the correct target architecture because the CMAKE_SYSTEM_PROCESSOR
# variable does not always guarantee that it will correspond to the target architecture for the build.
# See: https://cmake.org/cmake/help/latest/variable/CMAKE_SYSTEM_PROCESSOR.html
function(flux_set_target_architecture _OUT)
    # On MacOSX we use `CMAKE_OSX_ARCHITECTURES` *if* it was set.
    if(APPLE AND CMAKE_OSX_ARCHITECTURES)
        list(LENGTH CMAKE_OSX_ARCHITECTURES _ARCH_COUNT)
        if(_ARCH_COUNT STREQUAL "1")
            if(CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64")
                set(_FLUX_TARGET_ARCH "x86_64" CACHE STRING "The target architecture." FORCE)
            elseif(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
                set(_FLUX_TARGET_ARCH "arm64" CACHE STRING "The target architecture." FORCE)
            else()
                message(FATAL_ERROR "Invalid target architecture. Flux Engine only supports 64-bit architecture.")
            endif()
        else()
            message(FATAL_ERROR "Incorrectly initialized CMAKE_OSX_ARCHITECTURES variable.\n"
                                "Do not target multiple architectures at once.")
        endif()
    else()
        file(WRITE "${CMAKE_BINARY_DIR}/generated/arch/detect_arch.c"
        [[#if defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(_M_X64)
        #   error TARGET_ARCH x86_64
        #if defined(__arm64) || defined(_M_ARM64) || defined(__aarch64__) || defined(__AARCH64EL__)
        #   error TARGET_ARCH arm64
        #endif
        #error TARGET_ARCH unsupported
        ]])

        enable_language(C)

        # Detect the architecture in a rather creative way...
        # This compiles a small C program which is a series of `#ifdefs` that selects a particular `#error`
        # preprocessor directive whose message string contains the target architecture. The program will
        # always fail to compile (both because the file is not a valid C program, and obviously because of the
        # presence of the `#error` preprocessor directives... but by exploiting the preprocessor in this way,
        # we can detect the correct target architecture even when cross-compiling, since the program itself
        # never needs to be run (only the compiler/preprocessor).
        try_run(
            run_result_unused
            compile_result_unused
            "${CMAKE_BINARY_DIR}"
            "${CMAKE_BINARY_DIR}/generated/arch/detect_arch.c"
            COMPILE_OUTPUT_VARIABLE _FLUX_TARGET_ARCH
        )
        # Parse the architecture name from the compiler output.
        string(REGEX MATCH "TARGET_ARCH ([a-zA-Z0-9_]+)" _FLUX_TARGET_ARCH "${_FLUX_TARGET_ARCH}")

        # Get rid of the value marker leaving just the architecture name.
        string(REPLACE "TARGET_ARCH " "" _FLUX_TARGET_ARCH "${_FLUX_TARGET_ARCH}")

        if (_FLUX_TARGET_ARCH STREQUAL "unsupported")
            message(FATAL_ERROR "Invalid target architecture. Flux Engine only supports 64-bit architecture.")
        endif()
    endif()

    set(${_OUT} "${_FLUX_TARGET_ARCH}" PARENT_SCOPE)
endfunction(flux_set_target_architecture)

function(flux_add_graphics_definitions _TARGET_GRAPHICS)
    string(TOUPPER ${_TARGET_GRAPHICS} GRAPHICS)
    string(CONCAT  FLUX_TARGET "-DFLUX_TARGET_" ${GRAPHICS})
    add_definitions(${FLUX_TARGET})
endfunction(flux_add_graphics_definitions)

if(NOT (DEFINED CACHE{FLUX_TARGET_CPU}    AND
        DEFINED CACHE{FLUX_TARGET_OS}     AND
        DEFINED CACHE{FLUX_TARGET_VENDOR} AND
        DEFINED CACHE{FLUX_TARGET_GRAPHICS}))
    if(WIN32)
        if(NOT CMAKE_SYSTEM_VERSION)
            set(CMAKE_SYSTEM_VERSION ${CMAKE_HOST_SYSTEM_VERSION} CACHE STRING "The version of the target platform." FORCE)
        endif()

        if(NOT CMAKE_SYSTEM_PROCESSOR)
            set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_HOST_SYSTEM_PROCESSOR} CACHE STRING "The target architecture." FORCE)
        endif()
        # Get the correct target architecture.
        flux_set_target_architecture(FLUX_TARGET_CPU)

        set(FLUX_TARGET_VENDOR   Microsoft        CACHE STRING "[READONLY] The target vendor."       FORCE)
        set(FLUX_TARGET_OS       Windows          CACHE STRING "[READONLY] The current platform."    FORCE)
        set(FLUX_TARGET_GRAPHICS ${FLUX_GRAPHICS} CACHE STRING "[READONLY] The target graphics api." FORCE)

        flux_add_graphics_definitions(${FLUX_TARGET_GRAPHICS})

        if(FLUX_TARGET_GRAPHICS STREQUAL "OpenGL")
            set(FLUX_API_VERSION_MAJOR 4 CACHE STRING "[READONLY] The target graphics api version major." FORCE)
            set(FLUX_API_VERSION_MINOR 6 CACHE STRING "[READONLY] The target graphics api version minor." FORCE)
            add_definitions("-DFLUX_OPENGL_VERSION_MAJOR=${FLUX_API_VERSION_MAJOR}")
            add_definitions("-DFLUX_OPENGL_VERSION_MINOR=${FLUX_API_VERSION_MINOR}")
        endif()
    elseif(APPLE)
        if(NOT DEFINED CMAKE_OSX_SYSROOT)
            message(FATAL_ERROR "The required variable CMAKE_OSX_SYSROOT does not exist in CMake cache.\n"
                                "CMAKE_OSX_SYSROOT holds the path to the SDK.")
        endif()

        list(LENGTH CMAKE_OSX_ARCHITECTURES _ARCH_COUNT)
        if(_ARCH_COUNT STREQUAL "1")
            set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_OSX_ARCHITECTURES} CACHE STRING "The target architecture." FORCE)
        endif()

        if(CMAKE_OSX_SYSROOT MATCHES ".*/MacOSX.platform/*")
            if(NOT CMAKE_SYSTEM_VERSION)
                set(CMAKE_SYSTEM_VERSION 11.3 CACHE STRING "The version of the target platform." FORCE)
            endif()
            # Get the correct target architecture.
            flux_set_target_architecture(FLUX_TARGET_CPU)

            set(FLUX_TARGET_VENDOR   Apple            CACHE STRING "[READONLY] The target vendor."       FORCE)
            set(FLUX_TARGET_OS       MacOSX           CACHE STRING "[READONLY] The current platform."    FORCE)
            set(FLUX_TARGET_GRAPHICS ${FLUX_GRAPHICS} CACHE STRING "[READONLY] The target graphics api." FORCE)

            flux_add_graphics_definitions(${FLUX_TARGET_GRAPHICS})

            if(FLUX_TARGET_GRAPHICS STREQUAL "OpenGL")
                set(FLUX_API_VERSION_MAJOR 4 CACHE STRING "[READONLY] The target graphics api version major." FORCE)
                set(FLUX_APIL_VERSION_MINOR 1 CACHE STRING "[READONLY] The target graphics api version minor." FORCE)
                add_definitions("-DFLUX_OPENGL_VERSION_MAJOR=${FLUX_API_VERSION_MAJOR}")
                add_definitions("-DFLUX_OPENGL_VERSION_MINOR=${FLUX_API_VERSION_MINOR}")
            endif()
        endif()
    elseif(UNIX)
        if(NOT CMAKE_SYSTEM_VERSION)
            set(CMAKE_SYSTEM_VERSION ${CMAKE_HOST_SYSTEM_VERSION} CACHE STRING "The version of the target platform." FORCE)
        endif()

        if(NOT CMAKE_SYSTEM_PROCESSOR)
            set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_HOST_SYSTEM_PROCESSOR} CACHE STRING "The target architecture." FORCE)
        endif()
        # Get the correct target architecture.
        flux_set_target_architecture(FLUX_TARGET_CPU)

        set(FLUX_TARGET_VENDOR   "Linus Torvalds" CACHE STRING "[READONLY] The target vendor."       FORCE)
        set(FLUX_TARGET_OS       Linux            CACHE STRING "[READONLY] The current platform."    FORCE)
        set(FLUX_TARGET_GRAPHICS ${FLUX_GRAPHICS} CACHE STRING "[READONLY] The target graphics api." FORCE)

        flux_add_graphics_definitions(${FLUX_TARGET_GRAPHICS})

        if(FLUX_TARGET_GRAPHICS STREQUAL "OpenGL")
            set(FLUX_API_VERSION_MAJOR 4 CACHE STRING "[READONLY] The target graphics api version major." FORCE)
            set(FLUX_API_VERSION_MINOR 6 CACHE STRING "[READONLY] The target graphics api version minor." FORCE)
            add_definitions("-DFLUX_OPENGL_VERSION_MAJOR=${FLUX_API_VERSION_MAJOR}")
            add_definitions("-DFLUX_OPENGL_VERSION_MINOR=${FLUX_API_VERSION_MINOR}")
        endif()
    endif()
endif()

#-----------------------------------------------------------------------------------------------------------------------
# Helper macros for imposing vendor/OS requirements on the built modules.
#-----------------------------------------------------------------------------------------------------------------------

macro(flux_requires_vendor _ARG_VENDOR)
    if(NOT FLUX_TARGET_VENDOR STREQUAL ${_ARG_VENDOR})
        get_filename_component(_TMP_BASENAME ${CMAKE_CURRENT_LIST_DIR} NAME)
        message("The subdirectory '${_TMP_BASENAME}' is ignored because the target OS vendor is set to "
                "'${FLUX_TARGET_VENDOR}'")
        unset(_TMP_BASENAME)
        return()
    endif()
endmacro(flux_requires_vendor)

#-----------------------------------------------------------------------------------------------------------------------
# Installation.
#-----------------------------------------------------------------------------------------------------------------------

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    # Defines the directory where the build artifacts will be placed.
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/output" CACHE PATH "" FORCE)
endif()

#-----------------------------------------------------------------------------------------------------------------------
# Helper functions.
#-----------------------------------------------------------------------------------------------------------------------

function(flux_common_app _NAME)
    if(NOT (FLUX_TARGET_OS STREQUAL "Windows" OR FLUX_TARGET_OS STREQUAL "Linux"))
        message("Target '${_NAME}' is ignored because the target OS is set to '${FLUX_TARGET_OS}'.")
        return()
    endif()
    cmake_parse_arguments(PARSE_ARGV 1         # start at the 1st argument
                          _FLUX_COMMON_APP
                          ""                   # options
                          ""                   # one   value keywords
                          "SOURCE;LINK")       # multi value keywords
    if(NOT _FLUX_COMMON_APP_SOURCE)
        message(FATAL_ERROR "Target '${_NAME}' has no sources.\n"
                            "Perhaps you have forgotten to provide the 'SOURCE' argument?")
    endif()
    add_executable(flux_${_NAME})
    target_sources(flux_${_NAME} PRIVATE "${_FLUX_COMMON_APP_SOURCE}")
    target_link_libraries(flux_${_NAME} PRIVATE flux::project_settings)
    if(_FLUX_COMMON_APP_LINK)
        target_link_libraries(flux_${_NAME} PRIVATE "${_FLUX_COMMON_APP_LINK}")
    endif()
    install(TARGETS flux_${_NAME}
            RUNTIME DESTINATION flux-${_NAME})
endfunction(flux_common_app)

function(flux_macosx_app _NAME)
    if(NOT FLUX_TARGET_OS STREQUAL "MacOSX")
        message("Target '${_NAME}' is ignored because the target OS is set to '${FLUX_TARGET_OS}'.")
        return()
    endif()
    cmake_parse_arguments(PARSE_ARGV 1         # start at the 1st argument
                          _FLUX_MACOSX_APP
                          ""                   # options
                          "BUNDLE_NAME"        # one   value keywords
                          "SOURCE;LINK")       # multi value keywords
    if(NOT _FLUX_MACOSX_APP_SOURCE)
        message(FATAL_ERROR "Target '${_NAME}' has no sources.\n"
                            "Perhaps you have forgotten to provide the 'SOURCE' argument?")
    endif()
    if(NOT _FLUX_MACOSX_BUNDLE_NAME)
        set(_FLUX_MACOSX_BUNDLE_NAME "${_NAME}")
    endif()
    add_executable(flux_${_NAME} MACOSX_BUNDLE)
    set_target_properties(flux_${_NAME} PROPERTIES
                          BUNDLE True
                          MACOSX_BUNDLE_GUI_IDENTIFIER "com.arcsine-project.${_FLUX_MACOSX_BUNDLE_NAME}"
                          MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
                          MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
                          MACOSX_BUNDLE_BUNDLE_NAME "${_FLUX_MACOSX_BUNDLE_NAME}"
                          XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
                          XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")
    target_sources(flux_${_NAME} PRIVATE "${_FLUX_MACOSX_APP_SOURCE}")
    target_link_libraries(flux_${_NAME} PRIVATE flux::project_settings)
    target_link_libraries(flux_${_NAME} PRIVATE " -framework AppKit")
    if(_FLUX_MACOSX_APP_LINK)
        target_link_libraries(flux_${_NAME} PRIVATE "${_FLUX_MACOSX_APP_LINK}")
    endif()
    install(TARGETS flux_${_NAME}
            BUNDLE DESTINATION flux-${_NAME})
endfunction(flux_macosx_app)

# flux_executable(<name>
#     <WINDOWS|MACOSX|LINUX|COMMON>
#          <SOURCE|LINK> items...
#         [<SOURCE|LINK> items...]...
#     [<WINDOWS|MACOSX|LINUX|COMMON>
#          <SOURCE|LINK> items...
#         [<SOURCE|LINK> items...]...]...)
function(flux_executable _ARG_NAME)
    cmake_parse_arguments(PARSE_ARGV 1                 # start at the 1st argument
                      _ARG                             # variable prefix
                      ""                               # options
                      ""                               # one   value keywords
                      "WINDOWS;MACOSX;LINUX;COMMON")   # multi value keywords
    if (FLUX_TARGET_OS STREQUAL "MacOSX")
        if (DEFINED _ARG_MACOSX OR DEFINED _ARG_COMMON)
            flux_macosx_app(${_ARG_NAME} ${_ARG_MACOSX} ${_ARG_COMMON})
        else()
            message("Ignoring flux::${_ARG_NAME}, this target is not supported on MacOSX.")
        endif()
    elseif(FLUX_TARGET_OS STREQUAL "Windows")
        if (DEFINED _ARG_WINDOWS OR DEFINED _ARG_COMMON)
            flux_common_app(${_ARG_NAME} ${_ARG_WINDOWS} ${_ARG_COMMON})
        else()
            message("Ignoring flux::${_ARG_NAME}, this target is not supported on Windows.")
        endif()
    elseif(FLUX_TARGET_OS STREQUAL "Linux")
        if (DEFINED _ARG_LINUX OR DEFINED _ARG_COMMON)
            flux_common_app(${_ARG_NAME} ${_ARG_LINUX} ${_ARG_COMMON})
        else()
            message("Ignoring flux::${_ARG_NAME}, this target is not supported on Linux.")
        endif()
    endif()
endfunction(flux_executable)

# This macro is used by `flux_static_library` and `flux_interface_library` functions. Don't call
# it unless you know what you are doing.
macro(_flux_unit_tests _ARG_NAME _TESTS_SOURCE)
    if(NOT "${_TESTS_SOURCE}" STREQUAL "" AND "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        find_package(Catch2 REQUIRED)
        set(_TESTS flux_${_ARG_NAME}_tests)
        add_executable(${_TESTS})
        target_sources(${_TESTS} PRIVATE "${_TESTS_SOURCE}")
        target_compile_definitions(${_TESTS} PRIVATE -DCATCH_CONFIG_CONSOLE_WIDTH=300)
        target_link_libraries(${_TESTS}
                              PRIVATE Catch2::Catch2
                                      flux::${_ARG_NAME})
        install(TARGETS ${_TESTS}
                RUNTIME DESTINATION flux-${_ARG_NAME}/bin)
        add_test(NAME ${_ARG_NAME} COMMAND ${_TESTS})
    endif()
endmacro(_flux_unit_tests)

# This macro is used by `flux_static_library` and `flux_interface_library` functions. Don't call
# it unless you know what you are doing.
macro(_flux_install_headers _ARG_NAME _ARG_DIRS)
    foreach(DIR IN ITEMS ${_ARG_DIRS})
        install(DIRECTORY   ${CMAKE_CURRENT_LIST_DIR}/flux/
                DESTINATION flux-${_ARG_NAME}/include
                FILES_MATCHING
                    PATTERN "*.hpp"
                    PATTERN "*.h")
    endforeach()
endmacro(_flux_install_headers)

# flux_metal_library(<name>
#       TARGET <target>
#       SOURCE <source>...)
function(flux_metal_library _ARG_NAME)
    cmake_parse_arguments(PARSE_ARGV 1    # start at the 1st argument
                          _ARG            # variable prefix
                          ""              # options
                          "TARGET"        # one   value keywords
                          "SOURCE")       # multi value keywords
    if (FLUX_TARGET_VENDOR STREQUAL "Apple")
        if(NOT _ARG_TARGET)
            message(FATAL_ERROR "The required argument TARGET is missing.")
        endif()

        get_target_property(_TARGET "${_ARG_TARGET}" ALIASED_TARGET)

        add_custom_command(TARGET "${_TARGET}" POST_BUILD
                           COMMAND xcrun -sdk ${CMAKE_OSX_SYSROOT} metal
                                -o ${CMAKE_CURRENT_BINARY_DIR}/${_ARG_NAME}.air
                                -c ${_ARG_SOURCE}
                           COMMAND xcrun -sdk ${CMAKE_OSX_SYSROOT} metallib
                                   ${CMAKE_CURRENT_BINARY_DIR}/${_ARG_NAME}.air
                                -o ${CMAKE_CURRENT_BINARY_DIR}/${_ARG_NAME}.metallib
                           WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
endfunction(flux_metal_library)

# flux_static_library(<name>
#     <WINDOWS|MACOSX|LINUX|COMMON>
#          <SOURCE|TEST|LINK|INCLUDE_DIR> items...
#         [<SOURCE|TEST|LINK|INCLUDE_DIR> items...]...
#     [<WINDOWS|MACOSX|LINUX|COMMON>
#          <SOURCE|TEST|LINK|INCLUDE_DIR> items...
#         [<SOURCE|TEST|LINK|INCLUDE_DIR> items...]...]...)
function(_flux_static_library _ARG_NAME)
    cmake_parse_arguments(PARSE_ARGV 1                      # start at the 1st argument
                          _ARG                              # variable prefix
                          ""                                # options
                          ""                                # one   value keywords
                          "SOURCE;TEST;LINK;INCLUDE_DIR")   # multi value keywords
    set(_TARGET "flux_${_ARG_NAME}")
    add_library(${_TARGET} STATIC)
    add_library("flux::${_ARG_NAME}" ALIAS ${_TARGET})
    target_include_directories(${_TARGET} PUBLIC "${CMAKE_CURRENT_LIST_DIR}" "${_ARG_INCLUDE_DIR}")
    target_link_libraries(${_TARGET}
                          PUBLIC  flux::project_settings
                                  "${_ARG_LINK}")
    target_sources(${_TARGET} PRIVATE "${_ARG_SOURCE}")
    install(TARGETS ${_TARGET}
            ARCHIVE DESTINATION flux-${_ARG_NAME}/lib)
    _flux_install_headers("${_ARG_NAME}" ".;${_ARG_INCLUDE_DIR}")
    _flux_unit_tests("${_ARG_NAME}" "${_ARG_TEST}")
endfunction(_flux_static_library)

function(flux_static_library _ARG_NAME)
    cmake_parse_arguments(PARSE_ARGV 1                     # start at the 1st argument
                          _ARG                             # variable prefix
                          ""                               # options
                          ""                               # one   value keywords
                          "WINDOWS;MACOSX;LINUX;COMMON")   # multi value keywords
    if (FLUX_TARGET_OS STREQUAL "MacOSX")
        if (DEFINED _ARG_MACOSX OR DEFINED _ARG_COMMON)
            _flux_static_library(${_ARG_NAME} ${_ARG_MACOSX} ${_ARG_COMMON})
        else()
            message("Ignoring flux::${_ARG_NAME}, this target is not supported on MacOSX.")
        endif()
    elseif(FLUX_TARGET_OS STREQUAL "Windows")
        if (DEFINED _ARG_WINDOWS OR DEFINED _ARG_COMMON)
            _flux_static_library(${_ARG_NAME} ${_ARG_WINDOWS} ${_ARG_COMMON})
        else()
            message("Ignoring flux::${_ARG_NAME}, this target is not supported on Windows.")
        endif()
    elseif(FLUX_TARGET_OS STREQUAL "Linux")
        if (DEFINED _ARG_LINUX OR DEFINED _ARG_COMMON)
            _flux_static_library(${_ARG_NAME} ${_ARG_LINUX} ${_ARG_COMMON})
        else()
            message("Ignoring flux::${_ARG_NAME}, this target is not supported on Linux.")
        endif()
    endif()
endfunction(flux_static_library)

# flux_interface_library(<name>
#     <WINDOWS|MACOSX|LINUX|COMMON>
#          <TEST|LINK> items...
#         [<TEST|LINK> items...]...
#     [<WINDOWS|MACOSX|LINUX|COMMON>
#          <TEST|LINK> items...
#         [<TEST|LINK> items...]...]...)
function(_flux_interface_library _ARG_NAME)
    cmake_parse_arguments(PARSE_ARGV 1   # start at the 1st argument
                          _ARG           # variable prefix
                          ""             # options
                          ""             # one   value keywords
                          "TEST;LINK")   # multi value keywords
    set(_TARGET "flux_${_ARG_NAME}")
    add_library(${_TARGET} INTERFACE)
    add_library("flux::${_ARG_NAME}" ALIAS ${_TARGET})
    target_include_directories(${_TARGET} INTERFACE "${CMAKE_CURRENT_LIST_DIR}")
    target_link_libraries(${_TARGET}
                          INTERFACE flux::project_settings
                                    "${_ARG_LINK}")
    _flux_install_headers("${_ARG_NAME}" ".")
    _flux_unit_tests("${_ARG_NAME}" "${_ARG_TEST}")
endfunction(_flux_interface_library)

function(flux_interface_library _ARG_NAME)
    cmake_parse_arguments(PARSE_ARGV 1                     # start at the 1st argument
                          _ARG                             # variable prefix
                          ""                               # options
                          ""                               # one   value keywords
                          "WINDOWS;MACOSX;LINUX;COMMON")   # multi value keywords
    if (FLUX_TARGET_OS STREQUAL "MacOSX")
        if (DEFINED _ARG_MACOSX OR DEFINED _ARG_COMMON)
            _flux_interface_library(${_ARG_NAME} ${_ARG_MACOSX} ${_ARG_COMMON})
        else()
            message("Ignoring flux::${_ARG_NAME}, this target is not supported on MacOSX.")
        endif()
    elseif(FLUX_TARGET_OS STREQUAL "Windows")
        if (DEFINED _ARG_WINDOWS OR DEFINED _ARG_COMMON)
            _flux_interface_library(${_ARG_NAME} ${_ARG_WINDOWS} ${_ARG_COMMON})
        else()
            message("Ignoring flux::${_ARG_NAME}, this target is not supported on Windows.")
        endif()
    elseif(FLUX_TARGET_OS STREQUAL "Linux")
        if (DEFINED _ARG_LINUX OR DEFINED _ARG_COMMON)
            _flux_interface_library(${_ARG_NAME} ${_ARG_LINUX} ${_ARG_COMMON})
        else()
            message("Ignoring flux::${_ARG_NAME}, this target is not supported on Linux.")
        endif()
    endif()
endfunction(flux_interface_library)

#-----------------------------------------------------------------------------------------------------------------------
# Build type.
#-----------------------------------------------------------------------------------------------------------------------

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
endif()

# Possible values of build type for cmake-gui and ccmake.
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
string(TOUPPER ${CMAKE_BUILD_TYPE} FLUX_BUILD_TYPE)

#-----------------------------------------------------------------------------------------------------------------------
# Setting a properly used compiler.
#-----------------------------------------------------------------------------------------------------------------------

if(CMAKE_CXX_COMPILER_ID MATCHES "^(Apple)?(C|c)?lang$")
    target_compile_definitions(flux::project_settings INTERFACE FLUX_CLANG)
endif()

#-----------------------------------------------------------------------------------------------------------------------
# Setting the correct CMAKE_<LANG>_FLAGS_<BUILD_TYPE> variables for the Clang-Windows bundle.
#-----------------------------------------------------------------------------------------------------------------------

# The code below changes the CMAKE_<LANG>_FLAGS_<BUILD_TYPE> variables. It does this for a good reason.
# Don't do this in normal code. Instead add the necessary compile/linker flags to flux::project_settings.
if(FLUX_TARGET_OS STREQUAL "Windows" AND CMAKE_CXX_COMPILER_ID MATCHES "(C|c)lang")
    set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF)
    if(FLUX_BUILD_TYPE STREQUAL "DEBUG")
        string(APPEND CMAKE_C_FLAGS_${FLUX_BUILD_TYPE}   " -D_DEBUG -D_MT -Xclang --dependent-lib=msvcrtd")
        string(APPEND CMAKE_CXX_FLAGS_${FLUX_BUILD_TYPE} " -D_DEBUG -D_MT -Xclang --dependent-lib=msvcrtd")
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        set(FLUX_MSVC_RUNTIME_LIBRARY  "MultiThreadedDebug")
    else()
        string(APPEND CMAKE_C_FLAGS_${FLUX_BUILD_TYPE}   " -D_MT -Xclang --dependent-lib=msvcrt")
        string(APPEND CMAKE_CXX_FLAGS_${FLUX_BUILD_TYPE} " -D_MT -Xclang --dependent-lib=msvcrt")
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
        set(FLUX_MSVC_RUNTIME_LIBRARY  "MultiThreaded")
    endif()
endif()

#-----------------------------------------------------------------------------------------------------------------------
# Getting LLVM Bitcode.
#-----------------------------------------------------------------------------------------------------------------------

# The code below changes the CMAKE_<LANG>_FLAGS and CMAKE_<LANG>_LINK_FLAGS variables. It does this for a good reason.
# Don't do this in normal code. Instead add the necessary compile/linker flags to flux::project_settings.
if (FLUX_TARGET_OS STREQUAL "MacOSX")
    option(FLUX_ENABLE_BITCODE "Enable Bitcode generation." YES)

    if(FLUX_ENABLE_BITCODE)
        string(APPEND CMAKE_C_FLAGS       " -fembed-bitcode")
        string(APPEND CMAKE_CXX_FLAGS     " -fembed-bitcode")
        string(APPEND CMAKE_OBJC_FLAGS    " -fembed-bitcode")
        string(APPEND CMAKE_OBJCXX_FLAGS  " -fembed-bitcode")

        # The flag '-headerpad_max_install_names' should not be used with '-fembed-bitcode'. CMake always adds
        # '-headerpad_max_install_names' flag. There's no appernt way to disable this flag otherwise.
        #   See: https://github.com/Kitware/CMake/blob/master/Modules/Platform/Darwin.cmake
        string(REPLACE "-Wl,-headerpad_max_install_names" "" CMAKE_C_LINK_FLAGS      ${CMAKE_C_LINK_FLAGS})
        string(REPLACE "-Wl,-headerpad_max_install_names" "" CMAKE_CXX_LINK_FLAGS    ${CMAKE_CXX_LINK_FLAGS})
        string(REPLACE "-Wl,-headerpad_max_install_names" "" CMAKE_OBJC_LINK_FLAGS   ${CMAKE_C_LINK_FLAGS})
        string(REPLACE "-Wl,-headerpad_max_install_names" "" CMAKE_OBJCXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS})
    endif()
endif()

#-----------------------------------------------------------------------------------------------------------------------
# Link time optimization.
#-----------------------------------------------------------------------------------------------------------------------

if(CMAKE_BUILD_TYPE MATCHES "Release")
    option(FLUX_ENABLE_LTO "Enable link time optimization. This is only valid for Release builds." YES)
endif()

if(FLUX_ENABLE_LTO)
    # The code below changes the CMAKE_<LANG>_FLAGS and CMAKE_<LANG>_LINK_FLAGS variables. It does this for a good
    # reason. Don't do this in normal code. Instead add the necessary compile/linker flags to flux::project_settings.

    # For LTO to work, we have to pass `-flto` flag to the compiler both at compile...
    string(APPEND CMAKE_C_FLAGS   " -flto")
    string(APPEND CMAKE_CXX_FLAGS " -flto")

    # and link time.
    string(APPEND CMAKE_C_LINKER_FLAGS   " -flto")
    string(APPEND CMAKE_CXX_LINKER_FLAGS " -flto")

    if(FLUX_TARGET_OS STREQUAL "MacOSX")
        # And the same thing for Obj-C/CXX compiler...
        string(APPEND CMAKE_OBJC_FLAGS   " -flto")
        string(APPEND CMAKE_OBJCXX_FLAGS " -flto")
        
        # and linker.
        string(APPEND CMAKE_OBJC_LINKER_FLAGS   " -flto")
        string(APPEND CMAKE_OBJCXX_LINKER_FLAGS " -flto")
    endif()
endif()

#-----------------------------------------------------------------------------------------------------------------------
# Enable colored diagnostics.
#-----------------------------------------------------------------------------------------------------------------------

if(CMAKE_CXX_COMPILER_ID MATCHES "^(Apple)?(C|c)?lang$")
    target_compile_options(flux::project_settings INTERFACE -fcolor-diagnostics)
endif()

#-----------------------------------------------------------------------------------------------------------------------
# Compiler cache.
#-----------------------------------------------------------------------------------------------------------------------

find_program(FLUX_CCACHE_COMMAND ccache)

if(FLUX_CCACHE_COMMAND)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${FLUX_CCACHE_COMMAND})
else()
    message(WARNING "Cannot find ccache. Incremental builds may get slower.")
endif()

#-----------------------------------------------------------------------------------------------------------------------
# Warnings.
#-----------------------------------------------------------------------------------------------------------------------

target_compile_options(flux::project_settings INTERFACE
    # Enable all the warnings about constructions that some users consider questionable, and that are easy to avoid.
    -Wall
    # Enable some extra warnings that are not enabled by -Wall.
    -Wextra
    # Warn whenever a local variable or type shadows another one.
    -Wshadow
    # Warn whenever a class has virtual functions and an accessible non-virtual destructor.
    -Wnon-virtual-dtor
    # Warn if old-style (C-style) cast to a non-void type is used within a C++ program.
    -Wold-style-cast
    # Warn whenever a pointer is cast such that the required alignment of the target is increased. For example, warn if
    # a char* is cast to an int* on machines where integers can only be accessed at two- or four-byte boundaries.
    -Wcast-align
    # Warn on anything being unused.
    -Wunused
    # Warn when a function declaration hides virtual functions from a base class.
    -Woverloaded-virtual
    # Warn whenever non-standard C++ is used.
    -Wpedantic
    # Warn on implicit conversions that may alter a value. This includes conversions between real and integer,
    # like abs(x) when x is double.
    -Wconversion
    # Warn for implicit conversions that may change the sign of an integer value, like assigning a signed integer
    # expression to an unsigned integer variable.
    -Wsign-conversion
    # Warn if the compiler detects paths that trigger erroneous or undefined behaviour due to dereferencing a null
    # pointer.
    -Wnull-dereference
    # Warn whenever a value of type float is implicitly promoted to double.
    -Wdouble-promotion
    # Check calls to printf and scanf, etc., to make sure that the arguments supplied have types appropriate to the
    # format string.
    -Wformat=2)

option(FLUX_WARNINGS_AS_ERRORS "Treat compiler warnings as errors." YES)

if(FLUX_WARNINGS_AS_ERRORS)
    target_compile_options(flux::project_settings INTERFACE
        # Make all warnings into errors.
        -Werror)
endif()

#-----------------------------------------------------------------------------------------------------------------------
# C++ Options.
#-----------------------------------------------------------------------------------------------------------------------

target_compile_options(flux::project_settings INTERFACE
    # Disable exceptions support and use the variant of C++ libraries without exceptions.
    -fno-exceptions
    # Disable generation of information about every class with virtual functions for use by the C++ runtime type
    # identification features (`dynamic_cast' and `typeid').
    -fno-rtti) 

#-----------------------------------------------------------------------------------------------------------------------
# Sanitizers.
#-----------------------------------------------------------------------------------------------------------------------

option(FLUX_ENABLE_SANITIZER_ADDRESS   "Enable address sanitizer."            YES)
option(FLUX_ENABLE_SANITIZER_THREAD    "Enable thread sanitizer."             NO )
option(FLUX_ENABLE_SANITIZER_UNDEFINED "Enable undefined behavior sanitizer." YES)
option(FLUX_ENABLE_SANITIZER_LEAK      "Enable leak sanitizer."               NO )

if(FLUX_TARGET_OS STREQUAL "Windows")
    message(WARNING "Address Sanitizer is not currently supported on Windows. Turning it off...")
    set(FLUX_ENABLE_SANITIZER_ADDRESS   NO)
    set(FLUX_ENABLE_SANITIZER_THREAD    NO)
    set(FLUX_ENABLE_SANITIZER_UNDEFINED NO)
    set(FLUX_ENABLE_SANITIZER_LEAK      NO)
endif()

if((FLUX_ENABLE_SANITIZER_LEAK OR FLUX_ENABLE_SANITIZER_ADDRESS) AND FLUX_ENABLE_SANITIZER_THREAD)
    message(WARNING "Thread sanitizer does not work with Address or Leak sanitizer enabled.")
endif()

set(FLUX_SANITIZERS "")

if(FLUX_ENABLE_SANITIZER_ADDRESS)
    list(APPEND FLUX_SANITIZERS "address")
endif()

if(FLUX_ENABLE_SANITIZER_THREAD)
    list(APPEND FLUX_SANITIZERS "thread")
endif()

if(FLUX_ENABLE_SANITIZER_UNDEFINED)
    list(APPEND FLUX_SANITIZERS "undefined")
endif()

if(FLUX_ENABLE_SANITIZER_LEAK)
    list(APPEND FLUX_SANITIZERS "leak")
endif()

list(JOIN FLUX_SANITIZERS "," FLUX_ENABLED_SANITIZERS)

if(FLUX_BUILD_TYPE STREQUAL "DEBUG")
    target_compile_options(flux::project_settings INTERFACE -fsanitize=${FLUX_ENABLED_SANITIZERS})
    target_link_options   (flux::project_settings INTERFACE -fsanitize=${FLUX_ENABLED_SANITIZERS})
endif()

# code: language="CMake" insertSpaces=true tabSize=4