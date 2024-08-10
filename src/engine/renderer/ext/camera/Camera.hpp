#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/types/Transform.hpp"
#include "engine/renderer/window/Window.hpp"
#include "engine/renderer/core/UniformBuffer.hpp"

namespace renderer {
    class Camera {
    public:
        static void i_Init();

        static void i_Update();

        static void i_Cleanup();

    private:
        static glm::mat4 view;
        static glm::mat4 perspectiveProjection;
        static glm::mat4 orthoProjection;

        static i_UniformBuffer orthoCamera;
        static i_UniformBuffer perspectiveCamera;
    };
}
