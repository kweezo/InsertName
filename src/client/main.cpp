#include "window/Window.hpp"
#include "renderer/Renderer.hpp"
#include "../settings.hpp"


int main(){
    Settings settings;
    ReadSettings(settings, "../src/settings.bin");
    #include <iostream>
    std::cout << "Width: " << settings.width << std::endl;
    std::cout << "Height: " << settings.height << std::endl;

    Window::CreateWindowContext(settings.width, settings.height, "Vulkan");
    Renderer::InitRenderer();

    while(!glfwWindowShouldClose(Window::GetGLFWwindow())){
        glfwPollEvents();
        Renderer::RenderFrame();
    }

    Renderer::DestroyRenderer();
    Window::DestroyWindowContext();

    return 0;
}