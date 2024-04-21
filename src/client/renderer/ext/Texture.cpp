#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../deps/stb_image.h"

namespace renderer{

void Texture::EnableNewTextures(){
    Image::UpdateCommandBuffers();
}

Texture::Texture(const std::string& path){
    LoadTexture(path);
    CreateTextureImage();
    CreateTextureImageView();
    CreateTextureSampler();

    useCount = new uint32_t;
    (*useCount) = 1;
}

void Texture::LoadTexture(const std::string& path){
    imageData.dat = stbi_load(path.c_str(), &imageData.width, &imageData.height,
     &imageData.channels, STBI_rgb_alpha);

    if(imageData.dat == nullptr){
        throw std::runtime_error("Failed to load texture, invalid path?");
    }
}

void Texture::CreateTextureImage(){
    image = Image::CreateImage (VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_FORMAT_R8G8B8A8_SRGB,
     imageData.width, imageData.height, imageData.width * imageData.height * 4, imageData.dat);
    stbi_image_free(imageData.dat);
}

void Texture::CreateTextureImageView(){

}

void Texture::CreateTextureSampler(){

}

Texture::Texture(const Texture& other){
    image = other.image;
    imageData = other.imageData;
    useCount = other.useCount;
    (*useCount)++;

}

Texture& Texture::operator=(const Texture& other){
    if(this == &other){
        return *this;
    }
    image = other.image;
    imageData = other.imageData;
    useCount = other.useCount;
    (*useCount)++;
    return *this;
   
}

Texture::~Texture(){
    if((*useCount) <= 1){
        Image::Free(image);
    }
}

}