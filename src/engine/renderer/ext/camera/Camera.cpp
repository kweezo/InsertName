#include "Camera.hpp"

namespace renderer {
    glm::mat4 Camera::view = {};
    glm::mat4 Camera::perspectiveProjection = {};
    glm::mat4 Camera::orthoProjection = {};

    i_UniformBuffer Camera::orthoCamera = {};
    i_UniformBuffer Camera::perspectiveCamera = {};

    i_Semaphore Camera::orthoSignalSemaphore = {};
    i_Semaphore Camera::perspectiveSignalSemaphore = {};

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


        std::vector<ShaderHandle > shaders = i_ShaderManager::GetShaderCategory("models");

        i_SemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.initalValue = 0;
        semaphoreInfo.type = VK_SEMAPHORE_TYPE_TIMELINE;

        orthoSignalSemaphore = i_Semaphore(semaphoreInfo);
        perspectiveSignalSemaphore = i_Semaphore(semaphoreInfo);


        i_UniformBufferCreateInfo createInfo{};

        CameraMatrices orthoMatrices = {view, orthoProjection};

        createInfo.binding = 0;
        createInfo.data = &orthoMatrices;
        createInfo.size = sizeof(CameraMatrices);
        createInfo.threadIndex = 0;
        createInfo.shaders = shaders;
        createInfo.signalSemaphore = orthoSignalSemaphore;
        createInfo.signalSemaphoreValue = i_Swapchain::GetFrameCount();

        orthoCamera = i_UniformBuffer(createInfo);

        CameraMatrices perspectiveMatrices = {view, perspectiveProjection};

        createInfo.binding = 0;
        createInfo.data = &perspectiveMatrices;
        createInfo.signalSemaphore = perspectiveSignalSemaphore;

        perspectiveCamera = i_UniformBuffer(createInfo);
    }


    void Camera::i_Update() {
        VkExtent2D extent = Window::GetExtent();

        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));

        glm::mat4 perspectiveProjection = glm::perspective(glm::radians(45.0f), (float) extent.width / extent.height,
                                                           0.1f, 100.0f);

        glm::mat4 orthoProjection = glm::ortho(0.0f, (float) extent.width, 0.0f, (float) extent.height, 0.1f, 100.0f);

        CameraMatrices orthoMatrices = {view, orthoProjection};
        CameraMatrices perspectiveMatrices = {view, perspectiveProjection};

        orthoCamera.SetSignalSemaphoreValue(i_Swapchain::GetFrameCount());
        perspectiveCamera.SetSignalSemaphoreValue(i_Swapchain::GetFrameCount());

        orthoCamera.UpdateData(&orthoMatrices, sizeof(glm::mat4)*2, 0);
        perspectiveCamera.UpdateData(&perspectiveCamera, sizeof(glm::mat4)*2, 0);
    }

    void Camera::i_Cleanup() {
        orthoCamera.Destructor();
        perspectiveCamera.Destructor();
    }


    i_Semaphore Camera::i_GetPerspectiveSignalSemaphore(){
        return perspectiveSignalSemaphore;
    }
    
    i_Semaphore Camera::i_GetOrthoSignalSemaphore(){
        return orthoSignalSemaphore;
    }
}

