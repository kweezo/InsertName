#include "Window.hpp"

GLFWwindow* Window::window = nullptr;

void Window::CreateWindowContext(int width, int height, const char* title){
    if (!glfwInitialized) {
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }
        glfwInitialized = true;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window) {
        throw std::runtime_error("Failed to create GLFW window");
    }
}

void Window::DestroyWindowContext(){
    glfwDestroyWindow(window);
    glfwTerminate();
}


GLFWwindow* GetGLFWwindow();