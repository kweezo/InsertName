#pragma once

#include <unordered_map>
#include <vector>
#include <thread>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "Model.hpp"
#include "engine/renderer/core/DataBuffer.hpp"
#include "engine/types/Transform.hpp"
#include "engine/renderer/core/GraphicsPipeline.hpp"

#define ModelInstanceHandle ModelInstanceImpl*



namespace renderer{

class ModelInstanceImpl;

typedef struct ModelInstanceData{
    std::vector<ModelInstanceHandle> instanceList;
    ShaderHandle shader;
    CommandBuffer commandBuffer;
    DataBuffer instanceBuffer;
} ModelInstanceData;

class ModelInstance{
public:
    static void Update();
    static ModelInstanceHandle Create(ModelHandle model, Transform trasnform, bool isStatic);
    static void Free(ModelInstanceHandle handle);
    static void DrawStatic();
};


class ModelInstanceImpl{
public:
    ModelInstanceImpl(ModelHandle model, Transform transform, bool isStatic);
    bool GetShouldDraw();
    void SetShouldDraw(bool shouldDraw);

    glm::mat4 GetModelMatrix();

    static void Update();
    static void UpdateStaticInstances();
    static void DrawStatic();

private:

    static void RecordStaticCommandBuffers();

    static std::unordered_map<ShaderHandle, GraphicsPipeline> pipeline;

    bool shouldDraw;
    glm::mat4 model; 

    static std::unordered_map<ModelHandle, ModelInstanceData> staticModelMatrices;
    static BufferDescriptions bufferDescriptions;

    static CommandBuffer staticInstancesCommandBuffer;

};

}