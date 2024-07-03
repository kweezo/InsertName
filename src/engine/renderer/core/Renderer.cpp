#include "Renderer.hpp"

namespace renderer{

void Renderer::Init(){
    HardInit();
    SoftInit();
}
void Renderer::HardInit(){
    Instance::Init();
    Device::Init();
    Window::CreateVulkanSurface();
    ImageImpl::Initialize();
    Camera::Init();
}

void Renderer::SoftInit(){
    Swapchain::Init();
    GraphicsPipeline::Init();
    ShaderManager::Init();
}

void Renderer::RenderFrame(){
    DataBuffer::UpdateCommandBuffer();
    Camera::Update();
}

void Renderer::DestroyRenderer(){
    ModelInstance::Cleanup();
    Camera::Cleanup();
    ShaderManager::Cleanup();
    Swapchain::Cleanup();
    DescriptorManager::Cleanup();
    DataBuffer::Cleanup();
    ImageImpl::Cleanup();
    vkDestroySurfaceKHR(Instance::GetInstance(), Window::GetVulkanSurface(), nullptr);
    GraphicsPipeline::Cleanup();
    Device::Cleanup();
    Instance::Cleanup();
}
}