#include "Renderer.hpp"

namespace renderer{

std::array<_Semaphore, MAX_FRAMES_IN_FLIGHT> Renderer::presentSemaphores{};
std::array<_Semaphore, MAX_FRAMES_IN_FLIGHT> Renderer::renderSemaphores{};
std::array<std::array<_Fence, 2>, MAX_FRAMES_IN_FLIGHT> Renderer::inFlightFences{};
std::array<std::array<VkFence, DRAW_QUEUE_SUBMIT_COUNT>, MAX_FRAMES_IN_FLIGHT> Renderer::inFlightFenceHandles{};
std::array<std::vector<VkCommandBuffer>, MAX_FRAMES_IN_FLIGHT> Renderer::commandBuffers{};
void Renderer::Init(){
    HardInit();
    SoftInit();
}
void Renderer::HardInit(){
    _Instance::Init();
    _Device::Init();
    Window::Init();
    _CommandPool::Init();
    _CommandBuffer::Init();
    _DataBuffer::Init();
    _Image::Init();
    Camera::__Init();
}

void Renderer::SoftInit(){
    _Swapchain::Init();
    _GraphicsPipeline::Init();
    _ShaderManager::Init();

    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        for(uint32_t y = 0; y < DRAW_QUEUE_SUBMIT_COUNT; y++){
            inFlightFences[i][y] = _Fence((i == 0));
            inFlightFenceHandles[i][y] = inFlightFences[i][y].GetFence(); 
        }
    }

    _SemaphoreCreateInfo semaphoreInfo{};

    for(_Semaphore& semaphore : renderSemaphores){
        semaphore = _Semaphore(semaphoreInfo);
    }

    for(_Semaphore& semaphore : presentSemaphores){
        semaphore = _Semaphore(semaphoreInfo);
    }

    ModelInstance::__Init();

    _Image::Update();
}

void Renderer::UpdatePrepare(){

    vkWaitForFences(_Device::GetDevice(), inFlightFenceHandles[_Swapchain::GetFrameInFlight()].size()-1, &inFlightFenceHandles[_Swapchain::GetFrameInFlight()][0], VK_TRUE, std::numeric_limits<uint64_t>::max());
    vkResetFences(_Device::GetDevice(), inFlightFenceHandles[_Swapchain::GetFrameInFlight()].size(), inFlightFenceHandles[_Swapchain::GetFrameInFlight()].data());

    _Swapchain::IncrementCurrentFrameInFlight();

    _Swapchain::IncrementCurrentFrameIndex(presentSemaphores[_Swapchain::GetFrameInFlight()]);
}

void Renderer::UpdateComponents(){
    _UniformBuffer::Update();
    Camera::__Update();


    std::array<std::thread, 2> threads = {
        std::thread(_DataBuffer::Update),
        std::thread(_Image::Update)
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
    std::array<_Fence, 2> instanceFences = {inFlightFences[_Swapchain::GetFrameInFlight()][0], inFlightFences[_Swapchain::GetFrameInFlight()][1]};

    ModelInstance::__Draw(presentSemaphores[_Swapchain::GetFrameInFlight()], instanceFences);
}

void Renderer::Present(){
    std::array<VkSemaphore, 2> modelRenderFinishedSemaphores = ModelInstance::GetRenderFinishedSemaphores(_Swapchain::GetFrameInFlight());

    std::array<VkSwapchainKHR, 1> swapchains = {_Swapchain::GetSwapchain()};
    std::array<uint32_t, 1> imageIndices = {_Swapchain::GetImageIndex()};
    std::array<VkSemaphore, 1> waitSemaphores = {modelRenderFinishedSemaphores[0]};

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.swapchainCount = swapchains.size();
    presentInfo.pSwapchains = swapchains.data();
    presentInfo.pImageIndices = imageIndices.data();

    presentInfo.waitSemaphoreCount = waitSemaphores.size();
    presentInfo.pWaitSemaphores = waitSemaphores.data();

    if(vkQueuePresentKHR(_Device::GetGraphicsQueue(), &presentInfo) != VK_SUCCESS){
        throw std::runtime_error("Failed to present to screen");
    }
}

void Renderer::UpdateCleanup(){
    for(std::vector<VkCommandBuffer>& commandBuffersForFrame : commandBuffers){
        commandBuffersForFrame.clear();
    }
}

void Renderer::Cleanup(){
    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        for(uint32_t y = 0; y < DRAW_QUEUE_SUBMIT_COUNT; y++){
            inFlightFences[i][y].~_Fence();
        }
    }

    for(_Semaphore& semaphore : renderSemaphores){
        semaphore.~_Semaphore();
    }

    for(_Semaphore& semaphore : presentSemaphores){
        semaphore.~_Semaphore();
    }

    ModelInstance::__Cleanup();
    _ShaderManager::Cleanup();
    _Swapchain::Cleanup();
    Camera::__Cleanup();
    _DescriptorManager::Cleanup();
    _DataBuffer::Cleanup();
    _Image::Cleanup();
    vkDestroySurfaceKHR(_Instance::GetInstance(), Window::GetVulkanSurface(), nullptr);
    _GraphicsPipeline::Cleanup();
    _CommandPool::Cleanup();
    _Device::Cleanup();
    _Instance::Cleanup();
}
}