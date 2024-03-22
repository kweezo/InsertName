#include "Window.hpp"

GLFWwindow* Window::window = nullptr;
bool Window::glfwInitialized = false;
VkSurfaceKHR Window::surface = VK_NULL_HANDLE;

void Window::CreateWindowContext(int width, int height, const char* title){
    if (glfwInitialized) {
        throw std::runtime_error("GLFW already initialized");
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

bool Window::GetGLFWInitialized(){
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

VkExtent2D  Window::GetExtent(){
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

VkSurfaceKHR Window::GetVulkanSurface(){
    return surface;
}