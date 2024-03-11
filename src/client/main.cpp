#include "window/Window.hpp"
#include "renderer/Renderer.hpp"
#include "../settings.hpp"

#include "renderer/Shader.hpp"


int main(){
    Settings settings;
    ReadSettings(settings, "src/settings.bin");

    Window::CreateWindowContext(settings.width, settings.height, "Vulkan");
    Renderer::InitRenderer();

    Shader shader = Shader("shaders/bin/triangleVert.spv", "shaders/bin/triangleFrag.spv"); // TEMP REMOVE LATER

    while(!glfwWindowShouldClose(Window::GetGLFWwindow())){
        glfwPollEvents();
        Renderer::RenderFrame();
    }

    Renderer::DestroyRenderer();
    Window::DestroyWindowContext();

    return 0;
}