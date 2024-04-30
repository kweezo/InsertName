#include "Model.hpp"

namespace renderer{

ModelHandle Model::CreateModel(std::string path){
    return new ModelImpl(path);
}

void Model::Free(ModelHandle model){
    delete model;
}

ModelImpl::ModelImpl(std::string path){
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs |
     aiProcess_GenNormals);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
        throw std::runtime_error("Failed to load model at path: " + path);
    }

    ProcessNode(scene->mRootNode, scene);
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


        if(mesh->mMaterialIndex >= 0){
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            if(material->GetTextureCount(aiTextureType_DIFFUSE) == 1){
                aiString path;
                material->GetTexture(aiTextureType_DIFFUSE, 0, &path);
                std::string texturePath = path.C_Str();
//                 TextureHandle texture = Texture::CreateTexture(texturePath, 0, descriptorSet);
            }
            else{
                std::runtime_error("No texture found for mesh or there is more than one, neither is supported");
            }
        }

        Mesh meshObj = Mesh(vertices, indices);
    }

    for(uint32_t i = 0; i < node->mNumChildren; i++){
        ProcessNode(node->mChildren[i], scene);
    }

}


Mesh::Mesh(std::vector<BasicMeshVertex>& vertices, std::vector<uint32_t>& indices){
    BufferDescriptions descriptions{};

    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(BasicMeshVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;


    VkVertexInputAttributeDescription attributeDescription = {};
    attributeDescription.binding = 0;

    attributeDescription.location = 0;
    attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescription.offset = offsetof(BasicMeshVertex, pos);

    descriptions.attributeDescriptions.push_back(attributeDescription);

    attributeDescription.location = 1;
    attributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescription.offset = offsetof(BasicMeshVertex, texCoord);

    descriptions.attributeDescriptions.push_back(attributeDescription);

    attributeDescription.location = 2;
    attributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescription.offset = offsetof(BasicMeshVertex, normal);

    descriptions.attributeDescriptions.push_back(attributeDescription);
    


    vtnBuffer = DataBuffer(descriptions, sizeof(BasicMeshVertex) * vertices.size(), vertices.data(),
     true, DATA_BUFFER_VERTEX_BIT);

    indexBuffer = DataBuffer({}, sizeof(uint32_t) * indices.size(), indices.data(), true,
     DATA_BUFFER_INDEX_BIT);
}

}