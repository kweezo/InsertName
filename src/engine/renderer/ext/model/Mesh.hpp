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

typedef struct _BasicMeshVertex{
    glm::vec3 pos;
    glm::vec2 texCoord;
    glm::vec3 normal;
}_BasicMeshVertex;

typedef struct _TextureMaps{
    std::shared_ptr<_Texture> diffuseMap{};
    std::shared_ptr<_Texture> specularMap{};
    std::shared_ptr<_Texture> normalMap{};
    std::shared_ptr<_Texture> albedoMap{};
}_TextureMaps;

class _Mesh{
public:
    _Mesh(std::vector<_BasicMeshVertex>& vertices, std::vector<uint32_t>& indices, _TextureMaps textureMaps);

    void RecordDrawCommands(_CommandBuffer& commandBuffer, uint32_t instanceCount);
private:
    _DataBuffer vtnBuffer;
    _DataBuffer indexBuffer;

    _TextureMaps textureMaps;

    uint32_t indexCount;

    static uint32_t GetCurrentThreadIndex();

    static uint32_t currentThreadIndex;
};

}