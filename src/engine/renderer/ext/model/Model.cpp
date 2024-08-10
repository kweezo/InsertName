#include "Model.hpp"

namespace renderer {
    std::vector<std::shared_ptr<i_Model> > ModelManager::models = {};

    ModelHandle ModelManager::Create(i_ModelCreateInfo createInfo) {
        models.emplace_back(std::make_shared<i_Model>(createInfo));

        return models.back();
    }

    void ModelManager::Destoy(ModelHandle handle) {
        std::vector<std::shared_ptr<i_Model> >::iterator it = std::find(models.begin(), models.end(), handle.lock());
        if (it == models.end()) {
            throw std::runtime_error("Tried to erase a nonexistent model!");
        }
        models.erase(it);
    }

    void ModelManager::i_Cleanup() {
        models.clear();
    }

    i_Model::i_Model(i_ModelCreateInfo createInfo): createInfo(createInfo) {
        LoadModelFile();
    }

    void i_Model::LoadModelFile() {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(createInfo.path, aiProcess_Triangulate | aiProcess_FlipUVs |
                                                                  aiProcess_GenNormals);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            throw std::runtime_error("Failed to load model at path: " + createInfo.path);
        }

        ProcessNode(scene->mRootNode, scene);
    }

    void i_Model::ProcessNode(aiNode *node, const aiScene *scene) {
        for (uint32_t i = 0; i < node->mNumMeshes; i++) {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            std::vector<i_BasicMeshVertex> vertices;
            std::vector<uint32_t> indices;

            for (uint32_t j = 0; j < mesh->mNumVertices; j++) {
                i_BasicMeshVertex vertex;
                vertex.pos = glm::vec3(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z);
                vertex.texCoord = glm::vec2(mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y);
                vertex.normal = glm::vec3(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z);
                vertices.push_back(vertex);
            }

            for (uint32_t j = 0; j < mesh->mNumFaces; j++) {
                aiFace face = mesh->mFaces[j];
                for (uint32_t k = 0; k < face.mNumIndices; k++) {
                    indices.push_back(face.mIndices[k]);
                }
            }

            i_TextureMaps textureMaps{};


            if (mesh->mMaterialIndex >= 0) {
                aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

                if (material->GetTextureCount(aiTextureType_BASE_COLOR) == 1) {
                    aiString path;
                    material->GetTexture(aiTextureType_BASE_COLOR, 0, &path);

                    i_TextureCreateInfo createInfo;
                    createInfo.binding = 1;
                    createInfo.path = std::string(path.C_Str());
                    createInfo.shaders = i_ShaderManager::GetShaderCategory("models");

                    textureMaps.albedoMap = std::make_shared<i_Texture>(createInfo);
                } else {
                    std::cerr <<
                            "No texture found for mesh or there is more than one, neither is supported (yet) for model: "
                            << createInfo.path << "\n";

                    i_TextureCreateInfo createInfo;
                    createInfo.binding = 1;
                    createInfo.path = std::string(MISSING_TEXTURE_PATH);
                    createInfo.shaders = i_ShaderManager::GetShaderCategory("models");

                    textureMaps.albedoMap = std::make_shared<i_Texture>(createInfo);
                }
                //TODO: add support for other maps
            }

            meshes.emplace_back(vertices, indices, textureMaps);
        }

        for (uint32_t i = 0; i < node->mNumChildren; i++) {
            ProcessNode(node->mChildren[i], scene);
        }
    }

    void i_Model::RecordDrawCommands(i_CommandBuffer &commandBuffer, uint32_t instanceCount) {
        for (i_Mesh &mesh: meshes) {
            mesh.RecordDrawCommands(commandBuffer, instanceCount);
        }
    }

    std::function<void(void)> i_Model::GetExtraDrawCommands() {
        return createInfo.extraDrawCalls;
    }

    std::weak_ptr<i_Shader> i_Model::GetShader() {
        return createInfo.shader;
    }

    std::string i_Model::GetName() {
        return createInfo.name;
    }
}
