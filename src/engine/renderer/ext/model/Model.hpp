#pragma once


#include <iostream>
#include <vector>
#include <list>
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

#define ModelHandle std::weak_ptr<i_Model>

namespace renderer{

class i_Model;

struct i_ModelCreateInfo{
    std::string path;
    std::string name;
    std::weak_ptr<i_Shader> shader;
    std::function<void(void)> extraDrawCalls;
};

class ModelManager{
public:
    static ModelHandle Create(i_ModelCreateInfo createInfo);
    static void Destoy(ModelHandle handle);

    static void i_Cleanup();
private:
    static std::vector<std::shared_ptr<i_Model>> models;
};


class i_Model{
public:
    i_Model(i_ModelCreateInfo createInfo);

    i_Model(const i_Model& other) = delete;
    i_Model(i_Model&& other) = delete;
    i_Model& operator=(const i_Model& other) = delete;
    i_Model& operator=(i_Model&& other) = delete;

    void RecordDrawCommands(i_CommandBuffer& commandBuffer, uint32_t instanceCount);
    
    std::weak_ptr<i_Shader> GetShader();
    std::function<void(void)> GetExtraDrawCommands();
    std::string GetName();


private: //copied from learnopengl.com *mostly* shamelessly
    void LoadModelFile();
    void ProcessNode(aiNode* node, const aiScene* scene);

    std::list<i_Mesh> meshes;
    std::unordered_map<std::string, i_Texture> loadedTextures;

    i_VertexInputDescriptions extraDescriptions;

    i_ModelCreateInfo createInfo;

};


}