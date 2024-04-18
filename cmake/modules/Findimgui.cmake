if(TARGET imgui::imgui)
    return()
endif()
add_library(imgui::imgui INTERFACE IMPORTED)
if(FLUX_TARGET_GRAPHICS STREQUAL "OpenGL")
    if(FLUX_TARGET_OS STREQUAL "Windows")
        target_link_libraries(imgui::imgui
                              INTERFACE "${CMAKE_BINARY_DIR}/output/external/imgui/lib/imgui.lib"
                                        "${CMAKE_BINARY_DIR}/output/external/imgui/lib/imgui_opengl.lib")
    elseif(FLUX_TARGET_OS STREQUAL "MacOSX" OR FLUX_TARGET_OS STREQUAL "Linux")
        target_link_libraries(imgui::imgui
                              INTERFACE "${CMAKE_BINARY_DIR}/output/external/imgui/lib/libimgui.a"
                                        "${CMAKE_BINARY_DIR}/output/external/imgui/lib/libimgui_opengl.a")
    endif()
elseif(FLUX_TARGET_GRAPHICS STREQUAL "Metal")
    if(FLUX_TARGET_OS STREQUAL "MacOSX")
        target_compile_options(imgui::imgui INTERFACE "-fobjc-arc")
        target_link_libraries(imgui::imgui INTERFACE "-framework Metal" "-framework AppKit")
        target_link_libraries(imgui::imgui 
                              INTERFACE "${CMAKE_BINARY_DIR}/output/external/imgui/lib/libimgui.a"
                                        "${CMAKE_BINARY_DIR}/output/external/imgui/lib/libimgui_metal.a")
    endif()
endif()
target_include_directories(imgui::imgui INTERFACE "${CMAKE_BINARY_DIR}/output/external/imgui/include")

# code: language="CMake" insertSpaces=true tabSize=4