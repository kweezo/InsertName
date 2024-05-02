#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/types/Transform.hpp"
#include "engine/renderer/window/Window.hpp"
#include <engine/renderer/core/UniformBuffer.hpp>

namespace renderer{

class Camera{
public:
    static void Init();
    static void Update();
    static void Cleanup();

    static VkWriteDescriptorSet GetWriteDescriptorSetPerspective(uint32_t binding);
    static VkWriteDescriptorSet GetWriteDescriptorSetOrtho(uint32_t binding);
private:
    glm::mat4 view;
    glm::mat4 perspectiveProjection;
    glm::mat4 orthoProjection;

    UniformBufferHandle orthoCamera;
    UniformBufferHandle perspectiveCamera;
};

}