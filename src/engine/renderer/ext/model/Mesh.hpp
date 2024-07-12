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

typedef struct __BasicMeshVertex{
    glm::vec3 pos;
    glm::vec2 texCoord;
    glm::vec3 normal;
}__BasicMeshVertex;

typedef struct __TextureMaps{
    std::shared_ptr<__Texture> diffuseMap{};
    std::shared_ptr<__Texture> specularMap{};
    std::shared_ptr<__Texture> normalMap{};
    std::shared_ptr<__Texture> albedoMap{};
}__TextureMaps;

class __Mesh{
public:
    __Mesh(std::vector<__BasicMeshVertex>& vertices, std::vector<uint32_t>& indices, __TextureMaps textureMaps);

    void RecordDrawCommands(__CommandBuffer& commandBuffer, uint32_t instanceCount);
private:
    __DataBuffer vtnBuffer;
    __DataBuffer indexBuffer;

    __TextureMaps textureMaps;

    uint32_t indexCount;

    static uint32_t GetCurrentThreadIndex();

    static uint32_t currentThreadIndex;
};

}