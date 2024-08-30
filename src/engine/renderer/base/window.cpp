#include "window.hpp"

namespace renderer{
    std::unique_ptr<i_Window> i_Window::window;

    void i_Window::Create(WindowCreateInfo& createInfo){
       window.reset(new i_Window(createInfo)); 
    }

    void i_Window::Destroy(){
        window.reset();
    }


    i_Window::i_Window(WindowCreateInfo& createInfo):
        glfwWindow(nullptr), surface(nullptr), createInfo(createInfo), fullscreen(createInfo.fullscreen), vsync(createInfo.vsync){

        CreateWindow();
        CreateSurface();
    }

    void i_Window::CreateWindow(){
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate); //TODO vsync????

        if(fullscreen){
            glfwWindow = glfwCreateWindow(createInfo.width, createInfo.height, createInfo.windowName.c_str(), glfwGetPrimaryMonitor(), nullptr);
        }else{
            glfwWindow = glfwCreateWindow(createInfo.width, createInfo.height, createInfo.windowName.c_str(), nullptr, nullptr);
        }

        if(!glfwWindow){
            throw std::runtime_error("ERROR: Failed to create a GLFW window");
        }

    }

    void i_Window::CreateSurface(){
#ifdef _WIN32
        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hwnd = glfwGetWin32Window(window);
        createInfo.hinstance = GetModuleHandle(nullptr);

        if (vkCreateWin32SurfaceKHR(renderer::__Instance::GetInstance(), &createInfo, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("ERROR: Failed to create window surface!");
        }
#else
        if(glfwCreateWindowSurface(i_Instance::GetInstance(), glfwWindow, nullptr, &surface) != VK_SUCCESS){
            throw std::runtime_error("ERROR: Failed to create window surface!");
        }
#endif
    }
    
    i_Window::~i_Window(){
        vkDestroySurfaceKHR(i_Instance::GetInstance(), surface, nullptr);
        glfwDestroyWindow(glfwWindow);
        glfwTerminate();
    }

    GLFWwindow* i_Window::GetGLFWWindow(){
        return window->glfwWindow;
    }
    
    VkSurfaceKHR i_Window::GetSurface(){
        return window->surface;
    }

}