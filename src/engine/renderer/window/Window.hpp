#pragma once



#include <stdexcept>

#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan_win32.h>
#endif

#include "../core/Instance.hpp"

class Window {

public:
    static void CreateWindowContext(int width, int height, const char* title);
    static void DestroyWindowContext();

    static GLFWwindow* GetGLFWwindow();

    static bool GetGLFWInitialized();
    static VkSurfaceKHR GetVulkanSurface();

    static VkExtent2D GetExtent();

    static void Init();

private:
    static GLFWwindow* window;

    static bool glfwInitialized;

    static VkSurfaceKHR surface;
};