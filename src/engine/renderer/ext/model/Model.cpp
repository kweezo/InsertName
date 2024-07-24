#include "Model.hpp"

namespace renderer{

std::vector<std::shared_ptr<_Model>> ModelManager::models = {};

ModelHandle ModelManager::Create(_ModelCreateInfo createInfo){
    models.emplace_back(std::make_shared<_Model>(createInfo));

    return models.back();
}

void ModelManager::Destoy(ModelHandle handle){
    std::vector<std::shared_ptr<_Model>>::iterator it = std::find(models.begin(), models.end(), handle.lock());
    if(it == models.end()){
        throw std::runtime_error("Tried to erase a nonexistent model!");
    }
    models.erase(it);
}

void ModelManager::Cleanup(){
    models.clear();
}

_Model::_Model(_ModelCreateInfo createInfo): createInfo(createInfo){
    LoadModelFile();
}

void _Model::LoadModelFile(){
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(createInfo.path, aiProcess_Triangulate | aiProcess_FlipUVs |
     aiProcess_GenNormals);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
        throw std::runtime_error("Failed to load model at path: " + createInfo.path);
    }

    ProcessNode(scene->mRootNode, scene);
}

void _Model::ProcessNode(aiNode* node, const aiScene* scene){
    for(uint32_t i = 0; i < node->mNumMeshes; i++){
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        std::vector<_BasicMeshVertex> vertices;
        std::vector<uint32_t> indices;

        for(uint32_t j = 0; j < mesh->mNumVertices; j++){
            _BasicMeshVertex vertex;
            vertex.pos = glm::vec3(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z);
            vertex.texCoord = glm::vec2(mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y);
            vertex.normal = glm::vec3(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z);
            vertices.push_back(vertex);
        }

        for(uint32_t j = 0; j < mesh->mNumFaces; j++){
            aiFace face = mesh->mFaces[j];
            for(uint32_t k = 0; k < face.mNumIndices; k++){
                indices.push_back(face.mIndices[k]);
            }
        }

        _TextureMaps textureMaps{};


        if(mesh->mMaterialIndex >= 0){
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            if(material->GetTextureCount(aiTextureType_BASE_COLOR) == 1){
                aiString path;
                material->GetTexture(aiTextureType_BASE_COLOR, 0, &path);

                _TextureCreateInfo createInfo;
                createInfo.binding = 0;
                createInfo.path = std::string(path.C_Str());
                {
                    std::shared_ptr<_Shader> shader = this->createInfo.shader.lock();
                    createInfo.descriptorSet = shader->GetDescriptorSet();
                }

                textureMaps.albedoMap = std::make_shared<_Texture>(createInfo);
            }
            else{
                std::runtime_error("No texture found for mesh or there is more than one, neither is supported");
            }
            //TODO: add support for other maps
        }

         meshes.emplace_back(vertices, indices, textureMaps);
    }

    for(uint32_t i = 0; i < node->mNumChildren; i++){
        ProcessNode(node->mChildren[i], scene);
    }

}

void _Model::RecordDrawCommands(_CommandBuffer& commandBuffer, uint32_t instanceCount){
    for(_Mesh& mesh : meshes){
        mesh.RecordDrawCommands(commandBuffer, instanceCount);
    }
}

std::function<void(void)> _Model::GetExtraDrawCommands(){
    return createInfo.extraDrawCalls;
}

std::weak_ptr<_Shader> _Model::GetShader(){
    return createInfo.shader;
}

std::string _Model::GetName(){
    return createInfo.name;
}

}