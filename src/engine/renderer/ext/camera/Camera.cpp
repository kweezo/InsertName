#include "Camera.hpp"

namespace renderer{

glm::mat4 Camera::view = {};
glm::mat4 Camera::perspectiveProjection = {};
glm::mat4 Camera::orthoProjection = {};

__UniformBuffer Camera::orthoCamera = {};
__UniformBuffer Camera::perspectiveCamera = {};

void Camera::__Init(){
    VkExtent2D extent = Window::GetExtent();


    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 perspectiveProjection = glm::perspective(glm::radians(45.0f), (float)extent.width / extent.height, 0.1f, 100.0f);
    glm::mat4 orthoProjection = glm::ortho(0.0f, (float)extent.width, 0.0f, (float)extent.height, 0.1f, 100.0f);

    __UniformBufferCreateInfo createInfo{};

    createInfo.binding = 0;
    createInfo.data = &orthoProjection;
    createInfo.size = sizeof(glm::mat4);
    createInfo.threadIndex = 0;
    createInfo.descriptorSet = VK_NULL_HANDLE; // TODO

    orthoCamera = __UniformBuffer(createInfo);

    createInfo.binding = 1;
    createInfo.data = &perspectiveProjection;

    perspectiveCamera = __UniformBuffer(createInfo);
}


void Camera::__Update(){
    VkExtent2D extent = Window::GetExtent();

    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 perspectiveProjection = glm::perspective(glm::radians(45.0f), (float)extent.width / extent.height, 0.1f, 100.0f);
    glm::mat4 orthoProjection = glm::ortho(0.0f, (float)extent.width, 0.0f, (float)extent.height, 0.1f, 100.0f);

    orthoCamera.UpdateData(&orthoProjection, sizeof(glm::mat4), 0);
    perspectiveCamera.UpdateData(&perspectiveProjection, sizeof(glm::mat4), 0);
}


VkWriteDescriptorSet Camera::__GetWriteDescriptorSetPerspective(uint32_t binding){
    return perspectiveCamera.GetWriteDescriptorSet();
}
VkWriteDescriptorSet Camera::__GetWriteDescriptorSetOrtho(uint32_t binding){
    return orthoCamera.GetWriteDescriptorSet();
}

}