#include "Camera.hpp"

namespace renderer {
    glm::mat4 Camera::view = {};
    glm::mat4 Camera::perspectiveProjection = {};
    glm::mat4 Camera::orthoProjection = {};

    i_UniformBuffer Camera::orthoCamera = {};
    i_UniformBuffer Camera::perspectiveCamera = {};

    struct CameraMatrices {
        glm::mat4 view;
        glm::mat4 proj;
    };

    void Camera::i_Init() {
        VkExtent2D extent = Window::GetExtent();


        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 perspectiveProjection = glm::perspective(glm::radians(45.0f), (float) extent.width / extent.height,
                                                           0.1f, 100.0f);
        glm::mat4 orthoProjection = glm::ortho(0.0f, static_cast<float>(extent.width), 0.0f,
                                               static_cast<float>(extent.height), 0.1f, 100.0f);


        std::vector<std::weak_ptr<i_Shader> > shaders = i_ShaderManager::GetShaderCategory("models");

        i_UniformBufferCreateInfo createInfo{};

        CameraMatrices orthoMatrices = {view, orthoProjection};

        createInfo.binding = 0;
        createInfo.data = &orthoMatrices;
        createInfo.size = sizeof(CameraMatrices);
        createInfo.threadIndex = 0;
        createInfo.shaders = shaders;

        orthoCamera = i_UniformBuffer(createInfo);

        CameraMatrices perspectiveMatrices = {view, perspectiveProjection};

        createInfo.binding = 0;
        createInfo.data = &perspectiveMatrices;

        perspectiveCamera = i_UniformBuffer(createInfo);
    }


    void Camera::i_Update() {
        VkExtent2D extent = Window::GetExtent();

        glm::mat4 view = glm::translate(view, glm::vec3(0.0f, 0.0f, -1.0f));
        glm::mat4 perspectiveProjection = glm::perspective(glm::radians(45.0f), (float) extent.width / extent.height,
                                                           0.1f, 100.0f);
        glm::mat4 orthoProjection = glm::ortho(0.0f, (float) extent.width, 0.0f, (float) extent.height, 0.1f, 100.0f);

        //orthoCamera.UpdateData(&orthoProjection, sizeof(glm::mat4), 0);
        //perspectiveCamera.UpdateData(&perspectiveProjection, sizeof(glm::mat4), 0);
    }

    void Camera::i_Cleanup() {
        orthoCamera.Destructor();
        perspectiveCamera.Destructor();
    }
}
