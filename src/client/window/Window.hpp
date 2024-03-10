#pragma once

#include <stdexcept>

#include <GLFW/glfw3.h>

class Window {

public:
    static void CreateWindowContext(int width, int height, const char* title);
    static void DestroyWindowContext();

    GLFWwindow* GetGLFWwindow() const;



private:
    static GLFWwindow* window;

    static bool glfwInitialized;
};