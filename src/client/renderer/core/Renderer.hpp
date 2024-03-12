#pragma once

#include <vulkan/vulkan.h>

#include "Instance.hpp"
#include "Device.hpp"
#include "../window/Window.hpp"
#include "Swapchain.hpp"
#include "CommandPool.hpp"


class Renderer{
public:
    static void InitRenderer();
    static void DestroyRenderer();

    static void RenderFrame();
private:
};