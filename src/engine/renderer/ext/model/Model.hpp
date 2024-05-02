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

#include "Mesh.hpp"

#define ModelHandle ModelImpl*

namespace renderer{

class ModelImpl;


class Model{
public:
    static ModelHandle CreateModel(std::string path, ShaderHandle shader);
    static void Free(ModelHandle model);
};

class ModelImpl{
public:
    ModelImpl(std::string path, ShaderHandle shader);

private: //copied from learnopengl.com mostly shamelessly
    void ProcessNode(aiNode* node, const aiScene* scene);

    std::vector<Mesh> meshes;
    std::unordered_map<std::string, Texture> loadedTextures;

    ShaderHandle shader;
};


}