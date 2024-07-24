#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../deps/stb_image.h"

namespace renderer{

std::vector<VkWriteDescriptorSet> _Texture::writeDescriptorSetsQueue = {};

//TODO delete image contents

_Texture::_Texture(){
}

_Texture::_Texture(_TextureCreateInfo createInfo): shaders(createInfo.shaders), binding(createInfo.binding) {
    LoadImageFile(createInfo.path);
    CreateImage();
    CreateSampler();

    image.TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    useCount = std::make_shared<uint32_t>(1);
}

void _Texture::LoadImageFile(const std::string path){
    data = stbi_load(path.c_str(), &width, &height,
     &channels, STBI_rgb_alpha);

    if(data == nullptr){
        throw std::runtime_error("Failed to load texture, invalid path?");
    }  
}

void _Texture::CreateImage(){
    _ImageCreateInfo createInfo{};

    createInfo.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    createInfo.format = VK_FORMAT_R8G8B8A8_SRGB; 
    createInfo.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    createInfo.imageExtent = {(unsigned int)width, (unsigned int)height};
    createInfo.size = width * height * 4;
    createInfo.data = data;
    createInfo.copyToLocalDeviceMemory = true;

    image = _Image(createInfo);

    image.TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
}

void _Texture::CreateSampler(){
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = Settings::GetInstance().anisotropyEnable;
    samplerInfo.maxAnisotropy = std::min((float)Settings::GetInstance().anisotropy, (float)_Device::GetPhysicalDeviceProperties().limits.maxSamplerAnisotropy);
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if(vkCreateSampler(_Device::GetDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS){
        throw std::runtime_error("Failed to create texture sampler");
    }
}

void _Texture::SetBinding(uint32_t binding){
    VkWriteDescriptorSet writeDescriptorSet{};

    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = sampler;
    imageInfo.imageView = image.GetImageView();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    for(std::weak_ptr<_Shader> shader : shaders){
        writeDescriptorSet.dstSet = shader.lock()->GetDescriptorSet();
        writeDescriptorSetsQueue.push_back(writeDescriptorSet);
    }
}

void _Texture::Update(){
    vkUpdateDescriptorSets(_Device::GetDevice(), writeDescriptorSetsQueue.size(), writeDescriptorSetsQueue.data(), 0, nullptr);
    writeDescriptorSetsQueue.clear();

    _Image::Update();
}

_Texture::_Texture(const _Texture& other){
    if(other.useCount.get() == nullptr){
        return;
    }

    width = other.width;
    height = other.height;
    channels = other.channels;
    data = other.data;

    image = other.image;
    sampler = other.sampler;
    descriptorSet = other.descriptorSet;

    useCount = other.useCount;
    (*useCount.get())++;
}

_Texture _Texture::operator=(const _Texture& other){
    if(this == &other){
        return *this;
    }

    if(other.useCount.get() == nullptr){
        return *this;
    }
    
    Destruct();

    width = other.width;
    height = other.height;
    channels = other.channels;
    data = other.data;

    image = other.image;
    sampler = other.sampler;
    descriptorSet = other.descriptorSet;

    useCount = other.useCount;
    (*useCount.get())++;

    return *this;
}

void _Texture::Destruct(){
    if(useCount.get() == nullptr){
        return;
    }

    if(*useCount.get() == 1){
        vkDestroySampler(_Device::GetDevice(), sampler, nullptr);

        useCount.reset();
        return;
    }

    (*useCount.get())--;

}

_Texture::~_Texture(){
    Destruct();
}

}