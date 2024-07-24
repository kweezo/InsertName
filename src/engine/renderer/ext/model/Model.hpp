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

#include <boost/container/flat_map.hpp>

#include "engine/renderer/core/Texture.hpp"
#include "engine/renderer/core/Shader.hpp"
#include "engine/renderer/core/CommandBuffer.hpp"
#include "engine/renderer/core/DataBuffer.hpp"

#include "Mesh.hpp"

#define MISSING_TEXTURE_PATH "./client_data/res/textures/no_texture.png"

#define ModelHandle std::weak_ptr<_Model>

namespace renderer{

class _Model;

struct _ModelCreateInfo{
    std::string path;
    std::string name;
    std::weak_ptr<_Shader> shader;
    std::function<void(void)> extraDrawCalls;
};

class ModelManager{
public:
    static ModelHandle Create(_ModelCreateInfo createInfo);
    static void Destoy(ModelHandle handle);

    static void Cleanup();
private:
    static std::vector<std::shared_ptr<_Model>> models;
};


class _Model{
public:
    _Model(_ModelCreateInfo createInfo);

    _Model(const _Model& other) = delete;
    _Model(_Model&& other) = delete;
    _Model& operator=(const _Model& other) = delete;
    _Model& operator=(_Model&& other) = delete;

    void RecordDrawCommands(_CommandBuffer& commandBuffer, uint32_t instanceCount);
    
    std::weak_ptr<_Shader> GetShader();
    std::function<void(void)> GetExtraDrawCommands();
    std::string GetName();


private: //copied from learnopengl.com *mostly* shamelessly
    void LoadModelFile();
    void ProcessNode(aiNode* node, const aiScene* scene);

    std::vector<_Mesh> meshes;
    std::unordered_map<std::string, _Texture> loadedTextures;

    __VertexInputDescriptions extraDescriptions;

    _ModelCreateInfo createInfo;

};


}