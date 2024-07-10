#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../deps/stb_image.h"

namespace renderer{


//TODO delete image contents

__Texture::__Texture(){
    useCount = std::make_shared<uint32_t>(1);
}

__Texture::__Texture(__TextureCreateInfo createInfo): descriptorSet(createInfo.descriptorSet), binding(createInfo.binding) {
    LoadImageFile(createInfo.path);
    CreateImage();
    CreateSampler();

    image.TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    useCount = std::make_shared<uint32_t>(1);
}

void __Texture::LoadImageFile(const std::string path){
    data = stbi_load(path.c_str(), &width, &height,
     &channels, STBI_rgb_alpha);

    if(data == nullptr){
        throw std::runtime_error("Failed to load texture, invalid path?");
    }  
}

void __Texture::CreateImage(){
    __ImageCreateInfo createInfo{};

    createInfo.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    createInfo.format = VK_FORMAT_R8G8B8A8_SRGB; 
    createInfo.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    createInfo.imageExtent = {(unsigned int)width, (unsigned int)height};
    createInfo.size = width * height * 4;
    createInfo.data = data;
    createInfo.copyToLocalDeviceMemory = true;

    image = __Image(createInfo);

    image.TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
}

void __Texture::CreateSampler(){
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = Settings::GetInstance().anisotropyEnable;
    samplerInfo.maxAnisotropy = std::min((float)Settings::GetInstance().anisotropy, (float)__Device::GetPhysicalDeviceProperties().limits.maxSamplerAnisotropy);
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if(vkCreateSampler(__Device::GetDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS){
        throw std::runtime_error("Failed to create texture sampler");
    }
}

VkWriteDescriptorSet __Texture::GetWriteDescriptorSet(){
    static VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = image.GetImageView();
    imageInfo.sampler = sampler;

    VkWriteDescriptorSet writeDescriptorSet{};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = descriptorSet;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.pImageInfo = &imageInfo;

    return writeDescriptorSet;
}

void __Texture::Update(){
    __Image::Update();
}

__Texture::__Texture(const __Texture& other){
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

__Texture __Texture::operator=(const __Texture& other){
    if(this == &other){
        return *this;
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

    return *this;
}

__Texture::~__Texture(){
    if(useCount.get() == nullptr){
        return;
    }

    if(*useCount.get() == 1){
        vkDestroySampler(__Device::GetDevice(), sampler, nullptr);

        useCount.reset();
        return;
    }

    (*useCount.get())--;
}

}