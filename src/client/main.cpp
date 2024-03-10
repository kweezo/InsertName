#include "window/Window.hpp"
#include "renderer/Renderer.hpp"
#include "settings.hpp"

int main(){
    Window::CreateWindowContext(1920, 1080, "Vulkan");
    Renderer::InitRenderer();

    while(!glfwWindowShouldClose(Window::GetGLFWwindow())){
        glfwPollEvents();
        Renderer::RenderFrame();
    }

    Renderer::DestroyRenderer();
    Window::DestroyWindowContext();

    return 0;
}