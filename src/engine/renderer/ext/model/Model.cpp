#include "Model.hpp"

namespace renderer{

ModelHandle Model::CreateModel(std::string path, ShaderHandle shader){
    return new ModelImpl(path, shader);
}

void Model::Free(ModelHandle model){
    delete model;
}

ModelImpl::ModelImpl(std::string path, ShaderHandle shader){
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs |
     aiProcess_GenNormals);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
        throw std::runtime_error("Failed to load model at path: " + path);
    }

    ProcessNode(scene->mRootNode, scene);

    this->shader = shader;
}

void ModelImpl::ProcessNode(aiNode* node, const aiScene* scene){
    for(uint32_t i = 0; i < node->mNumMeshes; i++){
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        std::vector<BasicMeshVertex> vertices;
        std::vector<uint32_t> indices;

        for(uint32_t j = 0; j < mesh->mNumVertices; j++){
            BasicMeshVertex vertex;
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

        TextureMaps textureMaps{};


        if(mesh->mMaterialIndex >= 0){
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            if(material->GetTextureCount(aiTextureType_BASE_COLOR) == 1){
                aiString path;
                material->GetTexture(aiTextureType_BASE_COLOR, 0, &path);
                std::string texturePath = path.C_Str();
                TextureHandle texture = Texture::CreateTexture(texturePath, 0, shader->GetDescriptorSet());
                textureMaps.albedoMap = texture;
            }
            else{
                std::runtime_error("No texture found for mesh or there is more than one, neither is supported");
            }
            //TODO: add support for other maps
        }

        Mesh meshObj = Mesh(vertices, indices, textureMaps);
    }

    for(uint32_t i = 0; i < node->mNumChildren; i++){
        ProcessNode(node->mChildren[i], scene);
    }

}



}