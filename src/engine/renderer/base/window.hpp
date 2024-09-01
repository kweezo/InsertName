#pragma once

#include <exception>
#include <memory>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

#include "instance.hpp"

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan_win32.h>
#endif


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
            static VkSurfaceKHR GetSurface();


            ~i_Window();
        private:
            i_Window(WindowCreateInfo& createInfo); 
            i_Window(const i_Window& other) = delete; 
            i_Window& operator=(const i_Window& other) = delete;

            static std::unique_ptr<i_Window> window;

            void CreateWindow();
            void CreateSurface();

            GLFWwindow* glfwWindow;
            VkSurfaceKHR surface;

            WindowCreateInfo createInfo;

            bool fullscreen;
            bool vsync;
        
    };
}