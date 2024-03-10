#pragma once

#define GLFW_INCLUDE_VULKAN

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include <stdexcept>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include "../renderer/Instance.hpp"

class Window {

public:
    static void CreateWindowContext(int width, int height, const char* title);
    static void DestroyWindowContext();

    static GLFWwindow* GetGLFWwindow();

    static bool GetGlfwInitialized();

    static void CreateVulkanSurface();

private:
    static GLFWwindow* window;

    static bool glfwInitialized;

    static VkSurfaceKHR surface;
};