#include "Window.hpp"

GLFWwindow* Window::window = nullptr;
bool Window::glfwInitialized = false;

void Window::CreateWindowContext(int width, int height, const char* title){
    if (!glfwInitialized) {
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }
        glfwInitialized = true;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#ifdef WIN32
    window = glfwCreateWindow(width, height, title, glfwGetPrimaryMonitor(), NULL);
#else
    window = glfwCreateWindow(width, height, title, NULL, NULL);
#endif
//TODO: REMOVE
    if (!window) {
        throw std::runtime_error("Failed to create GLFW window");
    }
}

void Window::DestroyWindowContext(){
    glfwDestroyWindow(window);
    glfwTerminate();
}


GLFWwindow* Window::GetGLFWwindow(){
    return window;
}