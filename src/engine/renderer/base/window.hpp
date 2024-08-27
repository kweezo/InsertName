#pragma once

#include <exception>
#include <memory>

#include <GLFW/glfw3.h>


namespace renderer{

    struct WindowCreateInfo{
        uint32_t width;
        uint32_t height;
        std::string windowName;
        bool fullscreen;
        bool vsync;
    };

    class i_Window{
        public:
            static void Create(WindowCreateInfo& createInfo);
            static void Destroy();

            static GLFWwindow* GetGLFWWindow();


            ~i_Window();
        private:
            i_Window(WindowCreateInfo& createInfo); 
            i_Window(const i_Window& other) = delete; 
            i_Window& operator=(const i_Window& other) = delete;

            static std::unique_ptr<i_Window> window;

            GLFWwindow* glfwWindow;

            bool fullscreen;
            bool vsync;
        
    };
}