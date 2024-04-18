find_package(imgui REQUIRED)
find_package(glfw REQUIRED)
find_package(glad REQUIRED)

flux_executable(playground
    COMMON
        LINK
            glfw::glfw
            glad::glad
            imgui::imgui
    WINDOWS
        SOURCE
            "playground.cpp"
    LINUX
        SOURCE
            "playground.cpp"
)

# code: language="CMake" insertSpaces=true tabSize=4