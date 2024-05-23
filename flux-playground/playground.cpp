// TODO:
//  Remove this dummy example asap.

// GLAD
#include <glad/gl.h>

// GLFW (include after glad)
#include <GLFW/glfw3.h>

// FLUX
#include <flux/foundation.hpp>
#include <flux/logging.hpp>

#if defined(_WIN64)
#    include <flux/platform/win32/api.hpp>
#endif

// Window dimensions
GLuint const WIDTH = 400, HEIGHT = 300;

GLFWwindow* create_window(char const* name, int major, int minor) {
    flux::log::debug("Creating Window, OpenGL ", major, ".", minor, ": ", flux::io::c_str(name));

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    return glfwCreateWindow(WIDTH, HEIGHT, name, NULL, NULL);
}

GladGLContext* create_context(GLFWwindow* window) {
    glfwMakeContextCurrent(window);

    GladGLContext* context =
            reinterpret_cast<GladGLContext*>(std::calloc(1, sizeof(GladGLContext)));
    if (!context)
        return nullptr;

    int version = gladLoadGLContext(context, glfwGetProcAddress);
    flux::log::trace("Loaded OpenGL ", GLAD_VERSION_MAJOR(version), ".",
                     GLAD_VERSION_MINOR(version));

    return context;
}

void free_context(GladGLContext* context) {
    free(context);
}

void draw(GLFWwindow* window, GladGLContext* gl, float r, float g, float b) {
    glfwMakeContextCurrent(window);

    gl->ClearColor(r, g, b, 1.0f);
    gl->Clear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(window);
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    (void)scancode;
    (void)mode;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

int main() {
#if defined(_WIN64)
    constexpr auto std_output_handle                  = static_cast<std::uint_least32_t>(-11);
    constexpr auto enable_processed_output            = static_cast<std::uint_least32_t>(0x0001);
    constexpr auto enable_virtual_terminal_processing = static_cast<std::uint_least32_t>(0x0004);

    auto*               handle       = flux::win32::GetStdHandle(std_output_handle);
    std::uint_least32_t console_mode = 0;
    flux::win32::GetConsoleMode(handle, &console_mode);
    console_mode |= enable_processed_output;
    console_mode |= enable_virtual_terminal_processing;
    flux::win32::SetConsoleMode(handle, console_mode);
#endif

    glfwInit();
    flux::log::info("GLFW initialized successfully.");

    GLFWwindow* window1 = create_window("Window 1", 4, 6);
    GLFWwindow* window2 = create_window("Window 2", 4, 5);

    if (!window1 || !window2) {
        flux::log::error("Failed to create GLFW window.");
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window1, key_callback);
    glfwSetKeyCallback(window2, key_callback);

    GladGLContext* context1 = create_context(window1);
    GladGLContext* context2 = create_context(window2);

    if (!context1 || !context2) {
        flux::log::error("Failed to initialize GL contexts.");
        free_context(context1);
        free_context(context2);
        return -1;
    }

    glfwMakeContextCurrent(window1);
    context1->Viewport(0, 0, WIDTH, HEIGHT);

    glfwMakeContextCurrent(window2);
    context2->Viewport(0, 0, WIDTH, HEIGHT);

    while (!glfwWindowShouldClose(window1) && !glfwWindowShouldClose(window2)) {
        glfwPollEvents();

        draw(window1, context1, 0.5f, 0.2f, 0.6f);
        draw(window2, context2, 0.0f, 0.1f, 0.8f);
    }

    free_context(context1);
    free_context(context2);

    glfwTerminate();

    return 0;
}