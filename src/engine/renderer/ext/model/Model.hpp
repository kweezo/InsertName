#include <iostream>
#include <vector>
#include <cstdint>
#include <string>
#include <memory>
#include <unordered_map>
#include <functional>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>

#include "engine/renderer/core/Texture.hpp"
#include "engine/renderer/core/Shader.hpp"
#include "engine/renderer/core/CommandBuffer.hpp"
#include "engine/renderer/core/DataBuffer.hpp"

#include "Mesh.hpp"

#define ModelHandle ModelImpl*

namespace renderer{

class ModelImpl;


class Model{
public:
    static ModelHandle CreateModel(std::string path, ShaderHandle shader, BufferDescriptions extraDescriptions, std::function<void(void)> extraDrawCommands);
    static void Free(ModelHandle model);
};

class ModelImpl{
public:
    ModelImpl(std::string path, ShaderHandle shader, BufferDescriptions extraDescriptions, std::function<void(void)> extraDrawCommands);

    void RecordDrawCommands(CommandBuffer& commandBuffer, uint32_t instanceCount);
    
    static std::unordered_map<ShaderHandle, std::vector<ModelHandle>> GetModelList();

    ShaderHandle GetShader();
    std::function<void(void)> GetExtraDrawCommands();
    BufferDescriptions GetExtraDescriptions();

private: //copied from learnopengl.com *mostly* shamelessly
    void ProcessNode(aiNode* node, const aiScene* scene);

    static std::unordered_map<ShaderHandle, std::vector<ModelHandle>> modelList;

    std::vector<Mesh> meshes;
    std::unordered_map<std::string, Texture> loadedTextures;

    ShaderHandle shader;
    BufferDescriptions extraDescriptions;
    std::function<void(void)> extraDrawCommands;
};


}