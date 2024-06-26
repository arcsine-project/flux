cmake_minimum_required(VERSION 3.23)

project(flux LANGUAGES CXX)

include("cmake/project_settings.cmake")
include("cmake/bootstrap.cmake")

#-----------------------------------------------------------------------------------------------------------------------
# Enable code coverage.
#-----------------------------------------------------------------------------------------------------------------------

option(FLUX_ENABLE_COVERAGE "Enable code coverage" OFF)

if(FLUX_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "^(Apple)?(C|c)?lang$")
        string(APPEND CMAKE_C_FLAGS   " -O0 -g -fprofile-instr-generate -fcoverage-mapping")
        string(APPEND CMAKE_CXX_FLAGS " -O0 -g -fprofile-instr-generate -fcoverage-mapping")

        message(STATUS "Code coverage: ON")
    else()
        message(FATAL_ERROR "Code coverage is not supported by the ${CMAKE_CXX_COMPILER_ID} compiler.")
    endif()
endif()

#-----------------------------------------------------------------------------------------------------------------------
# Libraries.
#-----------------------------------------------------------------------------------------------------------------------

add_subdirectory("flux-config")
add_subdirectory("flux-foundation")
add_subdirectory("flux-io")
add_subdirectory("flux-logging")
add_subdirectory("flux-meta")
add_subdirectory("flux-platform")

#-----------------------------------------------------------------------------------------------------------------------
# Applications.
#-----------------------------------------------------------------------------------------------------------------------

add_subdirectory("flux-playground")

# code: language="CMake" insertSpaces=true tabSize=4