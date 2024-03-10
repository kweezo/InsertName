#pragma once

#include <vector>
#include <stdexcept>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "../window/Window.hpp"

class Instance{
public:
    static void CreateInstance();
    static void DestroyInstance();

    static VkInstance GetInstance();
private:
    static VkInstance instance;
};