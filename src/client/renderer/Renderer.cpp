#include "Renderer.hpp"


void Renderer::InitRenderer(){
    Instance::CreateInstance();
}

void Renderer::DestroyRenderer(){
    Instance::DestroyInstance();
}