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

#include "../core/DataBuffer.hpp"
#include "../core/Texture.hpp"

#define ModelHandle ModelImpl*

namespace renderer{

class ModelImpl;

typedef struct BasicMeshVertex{
    glm::vec3 pos;
    glm::vec2 texCoord;
    glm::vec3 normal;
}BasicMeshVertex;

class Model{
public:
    static ModelHandle CreateModel(std::string path);
    static void Free(ModelHandle model);
};

class ModelImpl{
public:
    ModelImpl(std::string path);

private: //copied from learnopengl.com mostly shamelessly
    void ProcessNode(aiNode* node, const aiScene* scene);

    std::unordered_map<std::string, Texture> loadedTextures;
};

class Mesh{
public:
    Mesh(std::vector<BasicMeshVertex>& vertices, std::vector<uint32_t>& indices);
private:
    DataBuffer vtnBuffer;
    DataBuffer indexBuffer;
};

}