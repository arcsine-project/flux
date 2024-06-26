if(TARGET Catch2::Catch2)
    return()
endif()
add_library(Catch2::Catch2 INTERFACE IMPORTED)

if(FLUX_TARGET_OS STREQUAL "Windows")
    target_link_libraries(Catch2::Catch2
                          INTERFACE "${CMAKE_BINARY_DIR}/output/external/Catch2/lib/Catch2WithMain.lib")
elseif(FLUX_TARGET_OS STREQUAL "MacOSX" OR FLUX_TARGET_OS STREQUAL "Linux")
    target_link_libraries(Catch2::Catch2
                          INTERFACE "${CMAKE_BINARY_DIR}/output/external/Catch2/lib/libCatch2WithMain.a")
endif()

target_include_directories(Catch2::Catch2
                           INTERFACE "${CMAKE_BINARY_DIR}/output/external/Catch2/include")

# code: language="CMake" insertSpaces=true tabSize=4