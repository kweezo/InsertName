#pragma once

#include <vulkan/vulkan.h>

#include "Instance.hpp"
#include "Device.hpp"
#include "../window/Window.hpp"
#include "Swapchain.hpp"
#include "CommandPool.hpp"
#include "GraphicsPipeline.hpp"
#include "DataBuffer.hpp"
#include "DescriptorManager.hpp"
#include "Image.hpp"
#include "Shader.hpp"
#include "engine/renderer/ext/camera/Camera.hpp"
#include "engine/renderer/ext/model/ModelInstance.hpp"


namespace renderer{

class Renderer{
public:
    static void InitRenderer();
    static void DestroyRenderer();

    static void RenderFrame();
private:
    static void HardInit();
    static void SoftInit();
/*
    So basically when you start up the renderer hard init gets called
    which initializes everyhing that needs to be initialized
    *only once*, then soft init which initializes everything that
    may get recreated later on for example when you switch stages
    and potentially need to load in new shaders. That gets called
    multiple times and performs a soft *reset* on the renderer  
*/
};

}