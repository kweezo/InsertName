#include "Model.hpp"

namespace renderer{

boost::container::flat_map<ModelHandle, std::shared_ptr<__Model>> ModelManager::models;

ModelHandle ModelManager::Create(__ModelCreateInfo createInfo){
    std::shared_ptr<__Model> model = std::make_shared<__Model>(__Model(createInfo));
    models[static_cast<ModelHandle>(model.get())] = model;

    return static_cast<ModelHandle>(model.get());
}

void ModelManager::Destoy(ModelHandle handle){
    models.erase(handle);
}

void ModelManager::Cleanup(){
    models.clear();
}

__Model::__Model(__ModelCreateInfo createInfo){
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(createInfo.path, aiProcess_Triangulate | aiProcess_FlipUVs |
     aiProcess_GenNormals);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
        throw std::runtime_error("Failed to load model at path: " + createInfo.path);
    }

    ProcessNode(scene->mRootNode, scene);

    this->shader = shader;
    this->extraDrawCommands = extraDrawCommands;
}

void __Model::ProcessNode(aiNode* node, const aiScene* scene){
    for(uint32_t i = 0; i < node->mNumMeshes; i++){
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        std::vector<__BasicMeshVertex> vertices;
        std::vector<uint32_t> indices;

        for(uint32_t j = 0; j < mesh->mNumVertices; j++){
            __BasicMeshVertex vertex;
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

        __TextureMaps textureMaps{};


        if(mesh->mMaterialIndex >= 0){
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            if(material->GetTextureCount(aiTextureType_BASE_COLOR) == 1){
                aiString path;
                material->GetTexture(aiTextureType_BASE_COLOR, 0, &path);

                __TextureCreateInfo createInfo;
                createInfo.binding = 0;
                createInfo.path = std::string(path.C_Str());
                createInfo.descriptorSet = shader->GetDescriptorSet();

                textureMaps.albedoMap = std::make_shared<__Texture>(createInfo);
            }
            else{
                std::runtime_error("No texture found for mesh or there is more than one, neither is supported");
            }
            //TODO: add support for other maps
        }

        __Mesh meshObj = __Mesh(vertices, indices, textureMaps);
    }

    for(uint32_t i = 0; i < node->mNumChildren; i++){
        ProcessNode(node->mChildren[i], scene);
    }

}

void __Model::RecordDrawCommands(__CommandBuffer& commandBuffer, uint32_t instanceCount){
    for(__Mesh& mesh : meshes){
        mesh.RecordDrawCommands(commandBuffer, instanceCount);
    }
}

std::function<void(void)> __Model::GetExtraDrawCommands(){
    return extraDrawCommands;
}

std::shared_ptr<__Shader> __Model::GetShader(){
    return shader;
}

}