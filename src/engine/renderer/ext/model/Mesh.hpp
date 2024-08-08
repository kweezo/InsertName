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

typedef struct i_BasicMeshVertex{
    glm::vec3 pos;
    glm::vec2 texCoord;
    glm::vec3 normal;
}i_BasicMeshVertex;

typedef struct i_TextureMaps{
    std::shared_ptr<i_Texture> diffuseMap{};
    std::shared_ptr<i_Texture> specularMap{};
    std::shared_ptr<i_Texture> normalMap{};
    std::shared_ptr<i_Texture> albedoMap{};
}i_TextureMaps;

class i_Mesh{
public:
    i_Mesh(std::vector<i_BasicMeshVertex>& vertices, std::vector<uint32_t>& indices, const i_TextureMaps& textureMaps);

    void RecordDrawCommands(i_CommandBuffer& commandBuffer, uint32_t instanceCount);
private:
    i_DataBuffer vtnBuffer;
    i_DataBuffer indexBuffer;

    i_TextureMaps textureMaps;

    uint32_t indexCount;

    static uint32_t GetCurrentThreadIndex();

    static uint32_t currentThreadIndex;
};

}