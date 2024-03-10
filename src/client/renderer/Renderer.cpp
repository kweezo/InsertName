#include "Renderer.hpp"


void Renderer::InitRenderer(){
    Instance::CreateInstance();
}

void Renderer::RenderFrame(){
    // Render frame
}

void Renderer::DestroyRenderer(){
    Instance::DestroyInstance();
}