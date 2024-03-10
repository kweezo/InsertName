#include "Window.hpp"

GLFWwindow* Window::window = nullptr;
bool Window::glfwInitialized = false;

void Window::CreateWindowContext(int width, int height, const char* title){
    if (glfwInitialized) {
        std::runtime_error("GLFW already initialized");
    }

    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    
    glfwInitialized = true;
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    

    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window) {
        throw std::runtime_error("Failed to create GLFW window");
    }
}

bool Window::GetGlfwInitialized(){
    return glfwInitialized;
}

void Window::DestroyWindowContext(){
    glfwDestroyWindow(window);
    glfwTerminate();
}


GLFWwindow* Window::GetGLFWwindow(){
    return window;
}