#pragma once

#include <vector>
#include <iostream>
#include <stdexcept>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "../window/Window.hpp"

namespace renderer{

class Instance{
public:
    static void CreateInstance();
    static void DestroyInstance();

    static VkInstance GetInstance();
private:
    static void SetupDebugMessenger();

    static VkInstance instance;
    static VkDebugUtilsMessengerEXT debugMessenger;
};

}