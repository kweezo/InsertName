#include "Camera.hpp"

namespace renderer{

glm::mat4 Camera::view = {};
glm::mat4 Camera::perspectiveProjection = {};
glm::mat4 Camera::orthoProjection = {};

UniformBufferHandle Camera::orthoCamera = {};
UniformBufferHandle Camera::perspectiveCamera = {};

void Camera::Init(){
    VkExtent2D extent = Window::GetExtent();


    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 perspectiveProjection = glm::perspective(glm::radians(45.0f), (float)extent.width / extent.height, 0.1f, 100.0f);
    glm::mat4 orthoProjection = glm::ortho(0.0f, (float)extent.width, 0.0f, (float)extent.height, 0.1f, 100.0f);

    orthoCamera = UniformBuffer::Create(&orthoProjection, sizeof(glm::mat4), 0, VK_NULL_HANDLE);
    perspectiveCamera = UniformBuffer::Create(&perspectiveProjection, sizeof(glm::mat4), 0, VK_NULL_HANDLE);
}


void Camera::Update(){
    VkExtent2D extent = Window::GetExtent();

    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 perspectiveProjection = glm::perspective(glm::radians(45.0f), (float)extent.width / extent.height, 0.1f, 100.0f);
    glm::mat4 orthoProjection = glm::ortho(0.0f, (float)extent.width, 0.0f, (float)extent.height, 0.1f, 100.0f);

    orthoCamera->UpdateData(&orthoProjection, sizeof(glm::mat4));
    perspectiveCamera->UpdateData(&perspectiveProjection, sizeof(glm::mat4));
}

void Camera::Cleanup(){
    UniformBuffer::Free(orthoCamera);
    UniformBuffer::Free(perspectiveCamera);
}

VkWriteDescriptorSet Camera::GetWriteDescriptorSetPerspective(uint32_t binding){
    return perspectiveCamera->GetWriteDescriptorSet();
}
VkWriteDescriptorSet Camera::GetWriteDescriptorSetOrtho(uint32_t binding){
    return orthoCamera->GetWriteDescriptorSet();
}

}