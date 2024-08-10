#pragma once

#include <array>
#include <vector>
#include <thread>
#include <limits>

#include <vulkan/vulkan.h>

#include "Instance.hpp"
#include "Swapchain.hpp"
#include "GraphicsPipeline.hpp"
#include "DataBuffer.hpp"
#include "Image.hpp"
#include "Shader.hpp"
#include "engine/renderer/ext/camera/Camera.hpp"
#include "engine/renderer/ext/model/ModelInstance.hpp"
#include "Semaphore.hpp"
#include "Fence.hpp"

#define DRAW_QUEUE_SUBMIT_COUNT 2

namespace renderer {
    class Renderer {
    public:
        static void Init();

        static void Cleanup();

        static void Update();

    private:
        static void HardInit();

        static void SoftInit();

        static void UpdatePrepare();

        static void UpdateComponents();

        static void Submit();

        static void Present();

        static void UpdateCleanup();

        static std::array<i_Semaphore, MAX_FRAMES_IN_FLIGHT> presentSemaphores;
        static std::array<i_Semaphore, MAX_FRAMES_IN_FLIGHT> renderSemaphores;
        static std::array<std::vector<VkCommandBuffer>, MAX_FRAMES_IN_FLIGHT> commandBuffers;

        static std::array<std::array<i_Fence, DRAW_QUEUE_SUBMIT_COUNT>, MAX_FRAMES_IN_FLIGHT> inFlightFences;
        //size is the number of draw operations, 1 for each queue submit
        static std::array<std::array<VkFence, DRAW_QUEUE_SUBMIT_COUNT>, MAX_FRAMES_IN_FLIGHT> inFlightFenceHandles;


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
