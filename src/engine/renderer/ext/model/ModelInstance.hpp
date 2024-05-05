#pragma once

#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "Model.hpp"
#include "engine/renderer/core/DataBuffer.hpp"
#include "engine/types/Transform.hpp"

#define ModelInstanceHandle ModelInstanceImpl*



namespace renderer{

class ModelInstanceImpl;

class ModelInstance{
public:
    static ModelInstanceHandle Create(ModelHandle model, Transform trasnform, bool isStatic);
    static void Free(ModelInstanceHandle handle);
};


class ModelInstanceImpl{
public:
    ModelInstanceImpl(ModelHandle model, Transform transform, bool isStatic);
    void ShouldDraw(bool shouldDraw);

    static void UpdateStaticInstances();
private:
    glm::mat4 model; 

    static std::unordered_map<ModelHandle, DataBuffer> staticModelMatrices;

};

}