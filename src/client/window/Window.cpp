#include "Window.hpp"

GLFWwindow* Window::window = nullptr;
bool Window::glfwInitialized = false;
VkSurfaceKHR Window::surface = VK_NULL_HANDLE;

void Window::CreateWindowContext(int width, int height, const char* title){
    if (glfwInitialized) {
        std::runtime_error("GLFW already initialized");
    }

    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    
    glfwInitialized = true;
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    

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

void Window::CreateVulkanSurface(){
    if(Instance::GetInstance == VK_NULL_HANDLE){
        throw std::runtime_error("Vulkan instance not created");
    }
    #ifdef _WIN32
        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hwnd = glfwGetWin32Window(window);
        createInfo.hinstance = GetModuleHandle(nullptr);

        if (vkCreateWin32SurfaceKHR(Instance::GetInstance(), &createInfo, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    #else
        if(glfwCreateWindowSurface(Instance::GetInstance(), window, nullptr, &surface) != VK_SUCCESS){
            throw std::runtime_error("Failed to create window surface!");
        }
    #endif
}