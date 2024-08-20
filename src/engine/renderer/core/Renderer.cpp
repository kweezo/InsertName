#include "Renderer.hpp"

#include "engine/renderer/ext/model/InstanceManager.hpp"

namespace renderer {
    std::array<i_Semaphore, MAX_FRAMES_IN_FLIGHT> Renderer::presentSemaphores{};
    std::array<i_Semaphore, MAX_FRAMES_IN_FLIGHT> Renderer::renderSemaphores{};
    std::array<std::array<i_Fence, DRAW_QUEUE_SUBMIT_COUNT>, MAX_FRAMES_IN_FLIGHT> Renderer::inFlightFences{};
    std::array<std::array<VkFence, DRAW_QUEUE_SUBMIT_COUNT>, MAX_FRAMES_IN_FLIGHT> Renderer::inFlightFenceHandles{};
    std::array<std::vector<VkCommandBuffer>, MAX_FRAMES_IN_FLIGHT> Renderer::commandBuffers{};

    void Renderer::Init() {
        HardInit();
        SoftInit();
    }

    void Renderer::HardInit() {
        i_Instance::Init();
        i_Device::Init();
        Window::Init();
        i_CommandPool::Init();
        i_CommandBuffer::Init();
        i_DataBuffer::Init();
        i_Image::Init();
    }

    void Renderer::SoftInit() {
        i_Swapchain::Init();
        i_GraphicsPipeline::Init();
        i_ShaderManager::Init();
        Camera::i_Init();
        i_InstanceManager::Init();

        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            for (uint32_t y = 0; y < DRAW_QUEUE_SUBMIT_COUNT; y++) {
                inFlightFences[i][y] = i_Fence((i == 0));
                inFlightFenceHandles[i][y] = inFlightFences[i][y].GetFence();
            }
        }

        i_SemaphoreCreateInfo semaphoreInfo{};

        for (i_Semaphore &semaphore: renderSemaphores) {
            semaphore = i_Semaphore(semaphoreInfo);
        }

        for (i_Semaphore &semaphore: presentSemaphores) {
            semaphore = i_Semaphore(semaphoreInfo);
        }

        i_Image::Update();
    }

    void Renderer::UpdatePrepare() {
        vkWaitForFences(i_Device::GetDevice(), inFlightFenceHandles[i_Swapchain::GetFrameInFlight()].size(),
                        &inFlightFenceHandles[i_Swapchain::GetFrameInFlight()][0], VK_TRUE,
                        std::numeric_limits<uint64_t>::max());
       vkResetFences(i_Device::GetDevice(), inFlightFenceHandles[i_Swapchain::GetFrameInFlight()].size(),
                      inFlightFenceHandles[i_Swapchain::GetFrameInFlight()].data());

        i_Swapchain::IncrementCurrentFrameInFlight();

        i_Swapchain::IncrementCurrentFrameIndex(presentSemaphores[i_Swapchain::GetFrameInFlight()]);
    }

    void Renderer::UpdateComponents() {
        i_UniformBuffer::Update();
        Camera::i_Update();
        i_Texture::Update();
        i_InstanceManager::EarlyUpdate();

        std::array<std::thread, 2> threads = {
            std::thread(i_DataBuffer::Update),
            std::thread(i_Image::Update)
        };

        for (std::thread &thread: threads) {
            if (thread.joinable()) {
                thread.join(); 
            }
        }

        i_InstanceManager::Update();
    }

    void Renderer::Update() {
        UpdatePrepare();

        UpdateComponents();

        Submit();
        Present();

        UpdateCleanup();
    }

    void Renderer::Submit() {
        std::array<i_Fence, 2> instanceFences = {
            inFlightFences[i_Swapchain::GetFrameInFlight()][0], inFlightFences[i_Swapchain::GetFrameInFlight()][0]
        };

        ModelInstance::i_Draw(presentSemaphores[i_Swapchain::GetFrameInFlight()], instanceFences);
    }

    void Renderer::Present() {
        std::array<VkSemaphore, 2> modelRenderFinishedSemaphores = ModelInstance::GetRenderFinishedSemaphores();

        std::array<VkSwapchainKHR, 1> swapchains = {i_Swapchain::GetSwapchain()};
        std::array<uint32_t, 1> imageIndices = {i_Swapchain::GetImageIndex()};
        std::array<VkSemaphore, 1> waitSemaphores = {modelRenderFinishedSemaphores[0]};

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.swapchainCount = swapchains.size();
        presentInfo.pSwapchains = swapchains.data();
        presentInfo.pImageIndices = imageIndices.data();

        presentInfo.waitSemaphoreCount = waitSemaphores.size();
        presentInfo.pWaitSemaphores = waitSemaphores.data();

        if (vkQueuePresentKHR(i_Device::GetGraphicsQueue(), &presentInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to present to screen");
        }
    }

    void Renderer::UpdateCleanup() {
        for (std::vector<VkCommandBuffer> &commandBuffersForFrame: commandBuffers) {
            commandBuffersForFrame.clear();
        }
    }

    void Renderer::Cleanup() {
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            for (uint32_t y = 0; y < DRAW_QUEUE_SUBMIT_COUNT; y++) {
                inFlightFences[i][y].Destruct();
            }
        }

        for (i_Semaphore &semaphore: renderSemaphores) {
            semaphore.Destruct();
        }

        for (i_Semaphore &semaphore: presentSemaphores) {
            semaphore.Destruct();
        }

        i_InstanceManager::Cleanup();
        ModelManager::i_Cleanup();
        i_ShaderManager::Cleanup();
        i_Swapchain::Cleanup();
        Camera::i_Cleanup();
        i_DescriptorManager::Cleanup();
        i_DataBuffer::Cleanup();
        i_Image::Cleanup();
        vkDestroySurfaceKHR(i_Instance::GetInstance(), Window::GetVulkanSurface(), nullptr);
        i_GraphicsPipeline::Cleanup();
        i_CommandPool::Cleanup();
        i_Device::Cleanup();
        i_Instance::Cleanup();
    }
}
