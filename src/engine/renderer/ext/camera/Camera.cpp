#include "Camera.hpp"

namespace renderer{

glm::mat4 Camera::view = {};
glm::mat4 Camera::perspectiveProjection = {};
glm::mat4 Camera::orthoProjection = {};

_UniformBuffer Camera::orthoCamera = {};
_UniformBuffer Camera::perspectiveCamera = {};

struct CameraMatrices{
    glm::mat4 view;
    glm::mat4 proj;
};

void Camera::__Init(){
    VkExtent2D extent = Window::GetExtent();


    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 perspectiveProjection = glm::perspective(glm::radians(45.0f), (float)extent.width / extent.height, 0.1f, 100.0f);
    glm::mat4 orthoProjection = glm::ortho(0.0f, (float)extent.width, 0.0f, (float)extent.height, 0.1f, 100.0f);


    std::vector<std::weak_ptr<_Shader>> shaders = _ShaderManager::GetShaderCategory("models");

    _UniformBufferCreateInfo createInfo{};

    CameraMatrices orthoMatrices = {view, orthoProjection};

    createInfo.binding = 0;
    createInfo.data = &orthoMatrices;
    createInfo.size = sizeof(CameraMatrices);
    createInfo.threadIndex = 0;
    createInfo.shaders = shaders;

    orthoCamera = _UniformBuffer(createInfo);

    CameraMatrices perspectiveMatrices = {view, perspectiveProjection};

    createInfo.binding = 0;
    createInfo.data = &perspectiveMatrices;

    perspectiveCamera = _UniformBuffer(createInfo);
}


void Camera::__Update(){
    VkExtent2D extent = Window::GetExtent();

    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 perspectiveProjection = glm::perspective(glm::radians(45.0f), (float)extent.width / extent.height, 0.1f, 100.0f);
    glm::mat4 orthoProjection = glm::ortho(0.0f, (float)extent.width, 0.0f, (float)extent.height, 0.1f, 100.0f);

    //orthoCamera.UpdateData(&orthoProjection, sizeof(glm::mat4), 0);
    //perspectiveCamera.UpdateData(&perspectiveProjection, sizeof(glm::mat4), 0);
}

void Camera::__Cleanup(){
    orthoCamera.~_UniformBuffer();
    perspectiveCamera.~_UniformBuffer();
}

}