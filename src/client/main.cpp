#include "window/Window.hpp"

int main(){
    Window::CreateWindowContext(1920, 1080, "Vulkan");

    while(!glfwWindowShouldClose(Window::GetGLFWwindow())){
        glfwPollEvents();
    }

    Window::DestroyWindowContext();

    return 0;
}