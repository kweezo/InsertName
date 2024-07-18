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

    ModelInstance::__Init();

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
    ModelInstance::__Draw(__Swapchain::GetImageIndex(), presentSemaphores[__Swapchain::GetFrameInFlight()]);
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