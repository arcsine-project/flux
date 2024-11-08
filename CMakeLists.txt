cmake_minimum_required(VERSION 3.23)

project(flux LANGUAGES CXX)

include("cmake/project_settings.cmake")
include("cmake/bootstrap.cmake")

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
