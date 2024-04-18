if(TARGET glad::glad)
    return()
endif()

if(FLUX_TARGET_GRAPHICS STREQUAL "OpenGL")
    add_library(glad::glad INTERFACE IMPORTED)
    if(FLUX_TARGET_OS STREQUAL "Windows")
        target_link_libraries(glad::glad
                              INTERFACE "${CMAKE_BINARY_DIR}/output/external/glad/lib/glad.lib")
    elseif(FLUX_TARGET_OS STREQUAL "MacOSX" OR FLUX_TARGET_OS STREQUAL "Linux")
        target_link_libraries(glad::glad
                              INTERFACE "${CMAKE_BINARY_DIR}/output/external/glad/lib/libglad.a")
    endif()
    target_include_directories(glad::glad
                               INTERFACE "${CMAKE_BINARY_DIR}/output/external/glad/include")
endif()

# code: language="CMake" insertSpaces=true tabSize=4