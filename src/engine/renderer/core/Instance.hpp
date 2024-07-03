#pragma once

#include <vector>
#include <iostream>
#include <stdexcept>
#include <cstring>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "../window/Window.hpp"

namespace renderer{

class Instance{
public:
    static void Init();
    static void Cleanup();

    static VkInstance GetInstance();
private:
    static void SetupDebugMessenger();

    static VkInstance instance;
    static VkDebugUtilsMessengerEXT debugMessenger;
};

}