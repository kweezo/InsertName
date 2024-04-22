#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../deps/stb_image.h"

namespace renderer{

std::vector<TextureHandle> TextureImpl::handles = {};

TextureHandle Texture::CreateTexture(const std::string& path, uint32_t binding, VkDescriptorSet descriptorSet){
    return new TextureImpl(path, binding, descriptorSet);
}

void Texture::Free(TextureHandle texture){
    delete texture;
}

void Texture::EnableTextures(){
    TextureImpl::EnableTextures();
}

void TextureImpl::SetDescriptorSet(VkDescriptorSet descriptorSet){
    this->descriptorSet = descriptorSet;
}

void TextureImpl::EnableTextures(){
    Image::UpdateCommandBuffers();
    for(TextureHandle handle : handles){
        handle->GetImage()->LoadDataIntoImage();
    }
    DataBuffer::UpdateCommandBuffer();
    for(TextureHandle handle : handles){
        handle->GetImage()->TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    Image::UpdateCommandBuffers();
    handles.clear();
}

ImageHandle TextureImpl::GetImage(){
    return image;
}

TextureImpl::TextureImpl(const std::string& path, uint32_t binding, VkDescriptorSet descriptorSet){
    LoadTexture(path);
    CreateTextureImage();
    CreateTextureImageView();
    CreateTextureSampler();

    handles.push_back(this);
    

    useCount = new uint32_t;
    (*useCount) = 1;

    this->descriptorSet = descriptorSet;
    this->binding = binding;
}

VkWriteDescriptorSet TextureImpl::GetWriteDescriptorSet(){
    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = descriptorSet;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptorSet.descriptorCount = 1;


    static VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;

    writeDescriptorSet.pImageInfo = &imageInfo;

    return writeDescriptorSet;
}


void TextureImpl::LoadTexture(const std::string& path){
    imageData.dat = stbi_load(path.c_str(), &imageData.width, &imageData.height,
     &imageData.channels, STBI_rgb_alpha);

    if(imageData.dat == nullptr){
        throw std::runtime_error("Failed to load texture, invalid path?");
    }
}

void TextureImpl::CreateTextureImage(){
    image = Image::CreateImage (VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_FORMAT_R8G8B8A8_SRGB,
     imageData.width, imageData.height, imageData.width * imageData.height * 4, imageData.dat);
    stbi_image_free(imageData.dat);


}


void TextureImpl::CreateTextureImageView(){
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image->GetImage();
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if(vkCreateImageView(Device::GetDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS){
        throw std::runtime_error("Failed to create texture image view");
    }
}

void TextureImpl::CreateTextureSampler(){
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE; //!!!!!
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if(vkCreateSampler(Device::GetDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS){
        throw std::runtime_error("Failed to create texture sampler");
    }
}

TextureImpl::TextureImpl(const TextureImpl& other){
    image = other.image;
    imageData = other.imageData;
    useCount = other.useCount;
    imageView = other.imageView;
    sampler = other.sampler;
    binding = other.binding;
    descriptorSet = other.descriptorSet;
    (*useCount)++;
}

TextureImpl& TextureImpl::operator=(const TextureImpl& other){
    if(this == &other){
        return *this;
    }
    image = other.image;
    imageData = other.imageData;
    useCount = other.useCount;
    imageView = other.imageView;
    binding = other.binding;
    sampler = other.sampler;
    descriptorSet = other.descriptorSet;
    (*useCount)++;
    return *this;
}

TextureImpl::~TextureImpl(){
    if((*useCount) <= 1){
        vkDestroySampler(Device::GetDevice(), sampler, nullptr);
        vkDestroyImageView(Device::GetDevice(), imageView, nullptr);
        Image::Free(image);
    }
}

}