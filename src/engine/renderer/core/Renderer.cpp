#include "Renderer.hpp"

namespace renderer{

std::array<__Semaphore, MAX_FRAMES_IN_FLIGHT> Renderer::presentSemaphores{};
std::array<__Semaphore, MAX_FRAMES_IN_FLIGHT> Renderer::renderSemaphores{};
std::array<std::array<__Fence, 2>, MAX_FRAMES_IN_FLIGHT> Renderer::inFlightFences{};
std::array<std::array<VkFence, DRAW_QUEUE_SUBMIT_COUNT>, MAX_FRAMES_IN_FLIGHT> Renderer::inFlightFenceHandles{};
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

    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        for(uint32_t y = 0; y < DRAW_QUEUE_SUBMIT_COUNT; y++){
            inFlightFences[i][y] = __Fence(true);
            inFlightFenceHandles[i][y] = inFlightFences[i][y].GetFence(); 
        }
    }

    __SemaphoreCreateInfo semaphoreInfo{};

    for(__Semaphore& semaphore : renderSemaphores){
        semaphore = __Semaphore(semaphoreInfo);
    }

    for(__Semaphore& semaphore : presentSemaphores){
        semaphore = __Semaphore(semaphoreInfo);
    }

    ModelInstance::__Init();

    __Image::Update();
}

void Renderer::UpdatePrepare(){

    vkWaitForFences(__Device::GetDevice(), inFlightFenceHandles[__Swapchain::GetFrameInFlight()].size()-1, &inFlightFenceHandles[__Swapchain::GetFrameInFlight()][0], VK_TRUE, std::numeric_limits<uint64_t>::max());
        vkResetFences(__Device::GetDevice(), inFlightFenceHandles[__Swapchain::GetFrameInFlight()].size(), inFlightFenceHandles[__Swapchain::GetFrameInFlight()].data());


    __Swapchain::IncrementCurrentFrameIndex(presentSemaphores[__Swapchain::GetFrameInFlight()]);
}

void Renderer::UpdateComponents(){
    Camera::__Update();


    std::array<std::thread, 2> threads = {
        std::thread(__DataBuffer::Update),
        std::thread(__Image::Update)
    };

    for(std::thread& thread : threads){
        if(thread.joinable()){
            thread.join(); // TODO if proper sync we don't have to wait???
        }
    }

    ModelInstance::__Update();   
}

void Renderer::Update(){
    UpdatePrepare();

    UpdateComponents();

    Submit();
    Present(); 

    UpdateCleanup();
}

void Renderer::Submit(){
    std::array<__Fence, 2> instanceFences = {inFlightFences[__Swapchain::GetFrameInFlight()][0], inFlightFences[__Swapchain::GetFrameInFlight()][1]};

    ModelInstance::__Draw(__Swapchain::GetImageIndex(), presentSemaphores[__Swapchain::GetFrameInFlight()], instanceFences);
}

void Renderer::Present(){
    std::array<VkSemaphore, 2> modelRenderFinishedSemaphores = ModelInstance::GetRenderFinishedSemaphores(__Swapchain::GetImageIndex());

    std::array<VkSwapchainKHR, 1> swapchains = {__Swapchain::GetSwapchain()};
    std::array<uint32_t, 1> imageIndices = {__Swapchain::GetImageIndex()};
    std::array<VkSemaphore, 1> waitSemaphores = {modelRenderFinishedSemaphores[0]};

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