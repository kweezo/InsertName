#pragma once


#include <iostream>
#include <vector>
#include <cstdint>
#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include <utility>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>

#include "engine/renderer/core/Texture.hpp"
#include "engine/renderer/core/Shader.hpp"
#include "engine/renderer/core/CommandBuffer.hpp"
#include "engine/renderer/core/DataBuffer.hpp"

#include "Mesh.hpp"

#define ModelHandle std::shared_ptr<__Model>

namespace renderer{

class __Model;

struct __ModelCreateInfo{
    std::string path;
    std::shared_ptr<__Shader> shader;
    std::function<void(void)> extraDrawCalls;
};

class ModelManager{
public:
    static ModelHandle Create(__ModelCreateInfo createInfo);
    static void Destoy(ModelHandle handle);

    static void Cleanup();
private:
    static boost::container::flat_map<ModelHandle, std::shared_ptr<__Model>> models;
};


class __Model{
public:
    __Model(__ModelCreateInfo createInfo);

    __Model(const __Model& other) = delete;
    __Model(__Model&& other) = delete;
    __Model& operator=(const __Model& other) = delete;
    __Model& operator=(__Model&& other) = delete;

    void RecordDrawCommands(__CommandBuffer& commandBuffer, uint32_t instanceCount);
    
    std::shared_ptr<__Shader> GetShader();
    std::function<void(void)> GetExtraDrawCommands();


private: //copied from learnopengl.com *mostly* shamelessly
    void LoadModelFile();
    void ProcessNode(aiNode* node, const aiScene* scene);

    std::vector<__Mesh> meshes;
    std::unordered_map<std::string, __Texture> loadedTextures;

    __VertexInputDescriptions extraDescriptions;

    __ModelCreateInfo createInfo;

};


}