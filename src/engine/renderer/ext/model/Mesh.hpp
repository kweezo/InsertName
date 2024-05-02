#pragma once

#include <iostream>
#include <vector>
#include <cstdint>
#include <string>
#include <memory>
#include <unordered_map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>

#include "engine/renderer/core/Texture.hpp"
#include "engine/renderer/core/Shader.hpp"
#include "engine/renderer/core/CommandBuffer.hpp"
#include "engine/renderer/core/DataBuffer.hpp"



namespace renderer{

typedef struct BasicMeshVertex{
    glm::vec3 pos;
    glm::vec2 texCoord;
    glm::vec3 normal;
}BasicMeshVertex;

typedef struct TextureMaps{
    TextureHandle diffuseMap = nullptr;
    TextureHandle specularMap = nullptr;
    TextureHandle normalMap = nullptr;
    TextureHandle albedoMap = nullptr;
}TextureMaps;

class Mesh{
public:
    Mesh(std::vector<BasicMeshVertex>& vertices, std::vector<uint32_t>& indices, TextureMaps textureMaps);
    void RecordCommandBuffer(CommandBuffer commandBuffer, VkBuffer instanceBuffer, uint32_t instanceCount);
    void RecordCommandBuffer(CommandBuffer commandBuffer);
private:
    DataBuffer vtnBuffer;
    DataBuffer indexBuffer;

    TextureMaps textureMaps;

    uint32_t indexCount;
};

}