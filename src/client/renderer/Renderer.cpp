#include "Renderer.hpp"


void Renderer::InitRenderer(){
    Instance::CreateInstance();
    Device::CreateDevice();
    Window::CreateVulkanSurface();
}

void Renderer::RenderFrame(){
    // Render frame
}

void Renderer::DestroyRenderer(){
    Device::DestroyDevice();
    Instance::DestroyInstance();
}