#include "Renderer.hpp"

namespace renderer{

std::array<__Semaphore, MAX_FRAMES_IN_FLIGHT> Renderer::presentSemaphores{};
std::array<__Semaphore, MAX_FRAMES_IN_FLIGHT> Renderer::renderSemaphores{};
std::array<__Fence, MAX_FRAMES_IN_FLIGHT> Renderer::inFlightFences{};
std::array<std::vector<VkCommandBuffer>, MAX_FRAMES_IN_FLIGHT> Renderer::commandBuffers{};
void Renderer::Init(){
    HardInit();
    SoftInit();
}
void Renderer::HardInit(){
    __Instance::Init();
    __Device::Init();
    Window::Init();
    __CommandPool::Init();
    __CommandBuffer::Init();
    __DataBuffer::Init();
    __Image::Init();
    Camera::__Init();
}

void Renderer::SoftInit(){
    __Swapchain::Init();
    __GraphicsPipeline::Init();
    __ShaderManager::Init();

    for(__Fence& fence : inFlightFences){
        fence = __Fence(true);
    }

    __SemaphoreCreateInfo semaphoreInfo{};

    for(__Semaphore& semaphore : renderSemaphores){
        semaphore = __Semaphore(semaphoreInfo);
    }

    for(__Semaphore& semaphore : presentSemaphores){
        semaphore = __Semaphore(semaphoreInfo);
    }


    __Image::Update();
}

void Renderer::UpdatePrepare(){
    std::array<VkFence, 1> waitFences = {inFlightFences[__Swapchain::GetFrameInFlight()].GetFence()}; 
    vkWaitForFences(__Device::GetDevice(), waitFences.size(), waitFences.data(), VK_TRUE, std::numeric_limits<uint64_t>::max());
    vkResetFences(__Device::GetDevice(), waitFences.size(), waitFences.data());

    __Swapchain::IncrementCurrentFrameIndex(presentSemaphores[__Swapchain::GetFrameInFlight()]);
}

void Renderer::UpdateComponents(){
    Camera::__Update();


    std::array<std::thread, 2> threads = {
        std::thread(__DataBuffer::Update),
        std::thread(__Image::Update)
    };

    for(std::thread& thread : threads){
        thread.join();
    }
    
}

void Renderer::Update(){
    UpdatePrepare();

    UpdateComponents();

    Submit();
    Present(); 

    UpdateCleanup();
}

void Renderer::Submit(){
    std::array<VkSemaphore, 1> waitSemaphores = {presentSemaphores[__Swapchain::GetFrameInFlight()].GetSemaphore()};
    std::array<VkSemaphore, 1> signalSemaphores = {renderSemaphores[__Swapchain::GetFrameInFlight()].GetSemaphore()};
    std::array<VkPipelineStageFlags, 1>  waitDestinationStageMask = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.commandBufferCount = commandBuffers[__Swapchain::GetFrameInFlight()].size();
    submitInfo.pCommandBuffers = commandBuffers[__Swapchain::GetFrameInFlight()].data();

    submitInfo.waitSemaphoreCount = waitSemaphores.size();
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitDestinationStageMask.data();

    submitInfo.signalSemaphoreCount = signalSemaphores.size();
    submitInfo.pSignalSemaphores = signalSemaphores.data();

    if(vkQueueSubmit(__Device::GetGraphicsQueue(), 1, &submitInfo, inFlightFences[__Swapchain::GetFrameInFlight()].GetFence()) != VK_SUCCESS){
        throw std::runtime_error("Failed to submit rendering commands to queue");
    }
}

void Renderer::Present(){
    std::array<VkSwapchainKHR, 1> swapchains = {__Swapchain::GetSwapchain()};
    std::array<uint32_t, 1> imageIndices = {__Swapchain::GetImageIndex()};
    std::array<VkSemaphore, 1> waitSemaphores = {renderSemaphores[__Swapchain::GetFrameInFlight()].GetSemaphore()};

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.swapchainCount = swapchains.size();
    presentInfo.pSwapchains = swapchains.data();
    presentInfo.pImageIndices = imageIndices.data();

    presentInfo.waitSemaphoreCount = waitSemaphores.size();
    presentInfo.pWaitSemaphores = waitSemaphores.data();

    if(vkQueuePresentKHR(__Device::GetGraphicsQueue(), &presentInfo) != VK_SUCCESS){
        throw std::runtime_error("Failed to present to screen");
    }

    __Swapchain::IncrementCurrentFrameInFlight();
}

void Renderer::UpdateCleanup(){
    for(std::vector<VkCommandBuffer>& commandBuffersForFrame : commandBuffers){
        commandBuffersForFrame.clear();
    }
}

void Renderer::AddCommandBuffer(__CommandBuffer& commandBuffer, uint32_t frameInFlight){
    commandBuffers[frameInFlight].push_back(commandBuffer.GetCommandBuffer());
}

void Renderer::Cleanup(){
    ModelInstance::__Cleanup();
    __ShaderManager::Cleanup();
    __Swapchain::Cleanup();
    Camera::__Cleanup();
    __DescriptorManager::Cleanup();
    __DataBuffer::Cleanup();
    __Image::Cleanup();
    vkDestroySurfaceKHR(__Instance::GetInstance(), Window::GetVulkanSurface(), nullptr);
    __GraphicsPipeline::Cleanup();
    __CommandPool::Cleanup();
    __Device::Cleanup();
    __Instance::Cleanup();
}
}