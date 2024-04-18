if($CACHE{FLUX_BOOTSTRAP_DONE})
    return()
endif()

find_package(Git REQUIRED)

########################################################################################################################
# Update submodules.
########################################################################################################################

message(" ==============================================================================\n"
        " Updating Git submodules, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE COMMAND_RESULT)
if(NOT COMMAND_RESULT EQUAL "0")
    message(FATAL_ERROR "Failed to update Git submodules.")
else()
    message(STATUS "Git submodules are already up-to-date.")
endif()

########################################################################################################################
# CMake arguments that are used to configure thirdparty libraries.
########################################################################################################################

set(FLUX_CONFIG_ARGUMENTS
    -DCMAKE_GENERATOR=${CMAKE_GENERATOR}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}
    -DCMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}

    -DCMAKE_CXX_STANDARD=23

    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}

    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}

    -DCMAKE_C_LINKER_FLAGS=${CMAKE_C_LINKER_FLAGS}
    -DCMAKE_CXX_LINKER_FLAGS=${CMAKE_CXX_LINKER_FLAGS}

    -DCMAKE_C_FLAGS_${FLUX_BUILD_TYPE}=${CMAKE_C_FLAGS_${FLUX_BUILD_TYPE}}
    -DCMAKE_CXX_FLAGS_${FLUX_BUILD_TYPE}=${CMAKE_CXX_FLAGS_${FLUX_BUILD_TYPE}}
)

if(FLUX_TARGET_OS STREQUAL "MacOSX")
    list(APPEND FLUX_CONFIG_ARGUMENTS "-DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}")
    list(APPEND FLUX_CONFIG_ARGUMENTS "-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    list(APPEND FLUX_CONFIG_ARGUMENTS "-DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}")

    list(APPEND FLUX_CONFIG_ARGUMENTS "-DCMAKE_OBJC_COMPILER=${CMAKE_OBJC_COMPILER}")
    list(APPEND FLUX_CONFIG_ARGUMENTS "-DCMAKE_OBJCXX_COMPILER=${CMAKE_OBJCXX_COMPILER}")

    list(APPEND FLUX_CONFIG_ARGUMENTS "-DCMAKE_OBJC_FLAGS=${CMAKE_OBJC_FLAGS}")
    list(APPEND FLUX_CONFIG_ARGUMENTS "-DCMAKE_OBJCXX_FLAGS=${CMAKE_OBJCXX_FLAGS}")

    list(APPEND FLUX_CONFIG_ARGUMENTS "-DCMAKE_OBJC_LINKER_FLAGS=${CMAKE_OBJC_LINKER_FLAGS}")
    list(APPEND FLUX_CONFIG_ARGUMENTS "-DCMAKE_OBJCXX_LINKER_FLAGS=${CMAKE_OBJCXX_LINKER_FLAGS}")
endif()

########################################################################################################################
# Configure, build, and install Catch2.
########################################################################################################################

string(CONCAT FLUX_CATCH2_CONFIG_CONSOLE_WIDTH "${CMAKE_CXX_FLAGS} -DCATCH_CONFIG_CONSOLE_WIDTH=300")

message(" ==============================================================================\n"
        " Configuring Catch2, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${CMAKE_COMMAND}
                        -B${CMAKE_BINARY_DIR}/external/Catch2
                        -S${CMAKE_SOURCE_DIR}/external/Catch2
                        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/output/external/Catch2
                        -DCATCH_BUILD_STATIC_LIBRARY=ON
                        -DCATCH_BUILD_TESTING=OFF
                        -DCATCH_INSTALL_DOCS=OFF
                        -DCATCH_INSTALL_HELPERS=OFF
                        ${FLUX_CONFIG_ARGUMENTS}
                        -DCMAKE_CXX_FLAGS=${FLUX_CATCH2_CONFIG_CONSOLE_WIDTH}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE COMMAND_RESULT
                COMMAND_ECHO STDOUT)
if(NOT COMMAND_RESULT STREQUAL "0")
    message(FATAL_ERROR "Failed to configure Catch2.")
endif()

message(" ==============================================================================\n"
        " Building and installing Catch2, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${CMAKE_COMMAND}
                        --build ${CMAKE_BINARY_DIR}/external/Catch2
                        --target install
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE COMMAND_RESULT
                COMMAND_ECHO STDOUT)
if(NOT COMMAND_RESULT STREQUAL "0")
    message(FATAL_ERROR "Failed to install Catch2.")
endif()

########################################################################################################################
# Configure, build, and install fast_io.
########################################################################################################################

message(" ==============================================================================\n"
        " Configuring fast_io, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${CMAKE_COMMAND}
                        -B${CMAKE_BINARY_DIR}/external/fast_io
                        -S${CMAKE_SOURCE_DIR}/external/fast_io
                        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/output/external/fast_io
                        -DUSE_MSVC_RUNTIME_LIBRARY_DLL=OFF
                        -DCMAKE_MSVC_RUNTIME_LIBRARY=${FLUX_MSVC_RUNTIME_LIBRARY}
                        ${FLUX_CONFIG_ARGUMENTS}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE COMMAND_RESULT
                COMMAND_ECHO STDOUT)
if(NOT COMMAND_RESULT STREQUAL "0")
    message(FATAL_ERROR "Failed to configure fast_io.")
endif()

message(" ==============================================================================\n"
        " Building and installing fast_io, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${CMAKE_COMMAND}
                        --build ${CMAKE_BINARY_DIR}/external/fast_io
                        --target install
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE COMMAND_RESULT
                COMMAND_ECHO STDOUT)
if(NOT COMMAND_RESULT STREQUAL "0")
    message(FATAL_ERROR "Failed to install fast_io.")
endif()

########################################################################################################################
# Configure, build, and install GLFW.
########################################################################################################################

message(" ==============================================================================\n"
        " Configuring GLFW, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${CMAKE_COMMAND}
                        -B${CMAKE_BINARY_DIR}/external/glfw
                        -S${CMAKE_SOURCE_DIR}/external/glfw
                        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/output/external/glfw
                        -DENKITS_BUILD_EXAMPLES=OFF
                        -DGLFW_BUILD_EXAMPLES=OFF
                        -DGLFW_BUILD_TESTS=OFF
                        -DGLFW_BUILD_DOCS=OFF
                        -DGLFW_BUILD_INSTALL=OFF
                        ${FLUX_CONFIG_ARGUMENTS}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE COMMAND_RESULT
                COMMAND_ECHO STDOUT)
if(NOT COMMAND_RESULT STREQUAL "0")
    message(FATAL_ERROR "Failed to configure GLFW.")
endif()

message(" ==============================================================================\n"
        " Building and installing GLFW, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${CMAKE_COMMAND}
                        --build ${CMAKE_BINARY_DIR}/external/glfw
                        --target install
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE COMMAND_RESULT
                COMMAND_ECHO STDOUT)
if(NOT COMMAND_RESULT STREQUAL "0")
    message(FATAL_ERROR "Failed to install GLFW.")
endif()

########################################################################################################################
# Configure, build, and install glad.
########################################################################################################################

message(" ==============================================================================\n"
        " Generating a project for glad, please wait...\n"
        " ==============================================================================")
file(WRITE "${CMAKE_BINARY_DIR}/generated/external/glad2/CMakeLists.txt"
[[cmake_minimum_required(VERSION 3.20)
project(glad2 CXX C)

set(GLAD_SOURCES_DIR "${GLAD_ROOT}")
add_subdirectory("${GLAD_SOURCES_DIR}/cmake" cmake)

# This target is OpenGL-specific.
if(FLUX_TARGET_GRAPHICS STREQUAL "OpenGL")
    set(GLAD_SPEC    gl)
    set(GLAD_PROFILE core)
    set(GLAD_ARGS    MX API ${GLAD_SPEC}:${GLAD_PROFILE}=${FLUX_API_VERSION_MAJOR}.${FLUX_API_VERSION_MINOR})
# This target is Vulkan-specific.
elseif(FLUX_TARGET_GRAPHICS STREQUAL "Vulkan")
    set(GLAD_SPEC    vulkan)
    set(GLAD_PROFILE core)
    set(GLAD_ARGS    API ${GLAD_SPEC}=${FLUX_API_VERSION_MAJOR}.${FLUX_API_VERSION_MINOR})
endif()

if(FLUX_TARGET_GRAPHICS STREQUAL "Metal")
    message(STATUS "Glad2 with Metal backend is not supported. Skipping...")
else()
    glad_add_library(glad STATIC REPRODUCIBLE LOCATION ${CMAKE_INSTALL_PREFIX} ${GLAD_ARGS})
    install(TARGETS glad)
endif()
]])
message(STATUS "GLAD project generation successfully finished.")

message(" ==============================================================================\n"
        " Configuring glad, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${CMAKE_COMMAND}
                        -B${CMAKE_BINARY_DIR}/external/glad
                        -S${CMAKE_BINARY_DIR}/generated/external/glad2
                        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/output/external/glad
                        -DGLAD_ROOT=${CMAKE_SOURCE_DIR}/external/glad
                        -DFLUX_TARGET_GRAPHICS=${FLUX_TARGET_GRAPHICS}
                        -DFLUX_API_VERSION_MAJOR=${FLUX_API_VERSION_MAJOR}
                        -DFLUX_API_VERSION_MINOR=${FLUX_API_VERSION_MINOR}
                        ${FLUX_CONFIG_ARGUMENTS}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE COMMAND_RESULT
                COMMAND_ECHO STDOUT)
if(NOT COMMAND_RESULT STREQUAL "0")
    message(FATAL_ERROR "Failed to configure glad.")
endif()

message(" ==============================================================================\n"
        " Building and installing glad, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${CMAKE_COMMAND}
                        --build ${CMAKE_BINARY_DIR}/external/glad
                        --target install
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE COMMAND_RESULT
                COMMAND_ECHO STDOUT)
if(NOT COMMAND_RESULT STREQUAL "0")
    message(FATAL_ERROR "Failed to install glad.")
endif()

########################################################################################################################
# Configure, build, and install Dear ImGui library.
########################################################################################################################

message(" ==============================================================================\n"
        " Generating a project for Dear ImGui, please wait...\n"
        " ==============================================================================")
file(WRITE "${CMAKE_BINARY_DIR}/generated/external/imgui/CMakeLists.txt"
[[cmake_minimum_required(VERSION 3.20)
project(imgui CXX)

add_library(imgui STATIC)
target_include_directories(imgui PRIVATE "${IMGUI_ROOT}")
target_sources(imgui PRIVATE
               "${IMGUI_ROOT}/imgui.cpp"
               "${IMGUI_ROOT}/imgui_demo.cpp"
               "${IMGUI_ROOT}/imgui_draw.cpp"
               "${IMGUI_ROOT}/imgui_tables.cpp"
               "${IMGUI_ROOT}/imgui_widgets.cpp"
               "${IMGUI_ROOT}/misc/cpp/imgui_stdlib.cpp")
set(IMGUI_PUBLIC_HEADERS
    "${IMGUI_ROOT}/imgui.h"
    "${IMGUI_ROOT}/misc/cpp/imgui_stdlib.h")
set(IMGUI_PRIVATE_HEADERS
    "${IMGUI_ROOT}/imconfig.h"
    "${IMGUI_ROOT}/imstb_rectpack.h"
    "${IMGUI_ROOT}/imstb_textedit.h"
    "${IMGUI_ROOT}/imstb_truetype.h")
set_target_properties(imgui PROPERTIES
                      PUBLIC_HEADER  "${IMGUI_PUBLIC_HEADERS}"
                      PRIVATE_HEADER "${IMGUI_PRIVATE_HEADERS}")
install(TARGETS imgui)

# This target is OpenGL-specific.
if(FLUX_TARGET_GRAPHICS STREQUAL "OpenGL")
    add_library(imgui_opengl STATIC)
    target_include_directories(imgui_opengl PRIVATE "${IMGUI_ROOT}" "${GLFW_ROOT}/include")
    target_sources(imgui_opengl PRIVATE
                   "${IMGUI_ROOT}/backends/imgui_impl_glfw.cpp"
                   "${IMGUI_ROOT}/backends/imgui_impl_opengl3.cpp")
    set(IMGUI_OPENGL_PRIVATE_HEADERS
        "${IMGUI_ROOT}/backends/imgui_impl_glfw.h"
        "${IMGUI_ROOT}/backends/imgui_impl_opengl3.h")
    set_target_properties(imgui_opengl PROPERTIES
                          PRIVATE_HEADER "${IMGUI_OPENGL_PRIVATE_HEADERS}")
    install(TARGETS imgui_opengl)
# This target is Metal-specific.
elseif(FLUX_TARGET_GRAPHICS STREQUAL "Metal")
    enable_language(OBJCXX)

    add_library(imgui_metal STATIC)
    target_include_directories(imgui_metal PRIVATE "${IMGUI_ROOT}")
    target_compile_options(imgui_metal PRIVATE -fobjc-arc)
    target_sources(imgui_metal PRIVATE
                   "${IMGUI_ROOT}/backends/imgui_impl_metal.mm"
                   "${IMGUI_ROOT}/backends/imgui_impl_osx.mm")
    set(IMGUI_METAL_PRIVATE_HEADERS
        "${IMGUI_ROOT}/backends/imgui_impl_metal.h"
        "${IMGUI_ROOT}/backends/imgui_impl_osx.h")
    set_target_properties(imgui_metal PROPERTIES
                          PRIVATE_HEADER "${IMGUI_METAL_PRIVATE_HEADERS}")
    target_link_libraries(imgui_metal PRIVATE "-framework Metal" "-framework AppKit")
    install(TARGETS imgui_metal)
# This target is Vulkan-specific.
elseif(FLUX_TARGET_GRAPHICS STREQUAL "Vulkan")
    message(FATAL_ERROR "ImGui with Vulkan backend is not currently supported.")
endif()
]])
message(STATUS "Dear ImGui project generation successfully finished.")

message(" ==============================================================================\n"
        " Configuring Dear ImGui, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${CMAKE_COMMAND}
                        -B${CMAKE_BINARY_DIR}/external/imgui
                        -S${CMAKE_BINARY_DIR}/generated/external/imgui
                        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/output/external/imgui
                        -DIMGUI_ROOT=${CMAKE_SOURCE_DIR}/external/imgui
                        -DGLFW_ROOT=${CMAKE_SOURCE_DIR}/external/glfw
                        -DFLUX_TARGET_GRAPHICS=${FLUX_TARGET_GRAPHICS}
                        -DUSE_MSVC_RUNTIME_LIBRARY_DLL=OFF
                        -DCMAKE_MSVC_RUNTIME_LIBRARY=${FLUX_MSVC_RUNTIME_LIBRARY}
                        ${FLUX_CONFIG_ARGUMENTS}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE COMMAND_RESULT
                COMMAND_ECHO STDOUT)
if(NOT COMMAND_RESULT STREQUAL "0")
    message(FATAL_ERROR "Failed to configure Dear ImGui.")
endif()

message(" ==============================================================================\n"
        " Building and installing Dear ImGui, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${CMAKE_COMMAND}
                        --build ${CMAKE_BINARY_DIR}/external/imgui
                        --target install
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE COMMAND_RESULT
                COMMAND_ECHO STDOUT)
if(NOT COMMAND_RESULT STREQUAL "0")
    message(FATAL_ERROR "Failed to install Dear ImGui.")
endif()


########################################################################################################################
# Configure, build, and install entt.
########################################################################################################################

message(" ==============================================================================\n"
        " Configuring entt, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${CMAKE_COMMAND}
                        -B${CMAKE_BINARY_DIR}/external/entt
                        -S${CMAKE_SOURCE_DIR}/external/entt
                        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/output/external/entt
                        ${FLUX_CONFIG_ARGUMENTS}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE COMMAND_RESULT
                COMMAND_ECHO STDOUT)
if(NOT COMMAND_RESULT STREQUAL "0")
    message(FATAL_ERROR "Failed to configure entt.")
endif()

message(" ==============================================================================\n"
        " Building and installing entt, please wait...\n"
        " ==============================================================================")
execute_process(COMMAND ${CMAKE_COMMAND}
                        --build ${CMAKE_BINARY_DIR}/external/entt
                        --target install
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE COMMAND_RESULT
                COMMAND_ECHO STDOUT)
if(NOT COMMAND_RESULT STREQUAL "0")
    message(FATAL_ERROR "Failed to install entt.")
endif()

########################################################################################################################
# Update CMakeCache.txt
########################################################################################################################

set(FLUX_BOOTSTRAP_DONE TRUE CACHE BOOL "Whether the bootstrapping has been done." FORCE)

# code: language='CMake' insertSpaces=true tabSize=4