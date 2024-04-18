#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../deps/stb_image.h"

namespace renderer{


Texture::Texture(const std::string& path){
    LoadTexture(path);
    CreateTextureImage();
    CreateTextureImageView();
    CreateTextureSampler();
}

void Texture::LoadTexture(const std::string& path){
    imageData.dat = stbi_load(path.c_str(), &imageData.width, &imageData.height,
     &imageData.channels, STBI_rgb_alpha);

    if(imageData.dat == nullptr){
        throw std::runtime_error("Failed to load texture, invalid path?");
    }
}

void Texture::CreateTextureImage(){
    VkImageCreateInfo imageInfo = {};

    QueueFamilyInfo queueFamilyInfo = Device::GetQueueFamilyInfo();
    if(queueFamilyInfo.transferFamilyFound){
        imageInfo.queueFamilyIndexCount = 2;

        uint32_t queueFamilyIndices[] = {queueFamilyInfo.graphicsQueueCreateInfo.queueFamilyIndex,
         queueFamilyInfo.transferQueueCreateInfo.queueFamilyIndex};

        imageInfo.pQueueFamilyIndices = queueFamilyIndices;
        imageInfo.queueFamilyIndexCount = 2;
        imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
    }else{
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = imageData.width;
    imageInfo.extent.height = imageData.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    if(vkCreateImage(Device::GetDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS){
        throw std::runtime_error("Failed to create image");
    }
}

void Texture::CreateTextureImageView(){

}

void Texture::CreateTextureSampler(){

}

}