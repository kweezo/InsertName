#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_map>
#include <memory>

#include "../core/DataBuffer.hpp"
#include "../core/Device.hpp"
#include "../core/Image.hpp"
#include "../core/DescriptorManager.hpp"
#include "client/account/Settings.hpp"

namespace renderer{

struct _TextureCreateInfo{
    std::string path;
    uint32_t binding;
    VkDescriptorSet descriptorSet;
};

class _Texture{
public:
    static void Update();

    _Texture();
    _Texture(_TextureCreateInfo createInfo);
    _Texture(const _Texture& other);
    _Texture operator=(const _Texture& other);
    ~_Texture();

    VkWriteDescriptorSet GetWriteDescriptorSet();

private:
    void Destruct();

    void LoadImageFile(const std::string path);
    void CreateImage();
    void CreateSampler();

    _Image image;
    VkSampler sampler;
    VkDescriptorSet descriptorSet;
    
    int width;
    int height;
    int channels;
    void* data;

    uint32_t binding;

    std::shared_ptr<uint32_t> useCount;
};

}