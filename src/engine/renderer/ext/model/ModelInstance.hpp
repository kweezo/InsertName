#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Model.hpp"
#include "engine/types/Transform.hpp"

namespace renderer{

class ModelInstance{
public:
    ModelInstance(ModelHandle model, Transform transform, bool isStatic);
    void ShouldDraw(bool shouldDraw);
private:
    glm::mat4 model; 
};

}