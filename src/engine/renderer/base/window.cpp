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
        glfwWindow(nullptr), fullscreen(createInfo.fullscreen), vsync(createInfo.vsync){

        if(glfwInit() == GLFW_FALSE){
            throw std::runtime_error("ERROR: Failed to init GLFW");
        }


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
    
    i_Window::~i_Window(){
        glfwDestroyWindow(glfwWindow);
        glfwTerminate();
    }

    GLFWwindow* i_Window::GetGLFWWindow(){
        return window->glfwWindow;
    }

}