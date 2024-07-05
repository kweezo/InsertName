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

struct __TextureCreateInfo{
    std::string path;
    uint32_t binding;
    VkDescriptorSet descriptorSet;
};

class __Texture{
public:
    static void Update();

    __Texture();
    __Texture(__TextureCreateInfo createInfo);
    __Texture(const __Texture& other);
    __Texture operator=(const __Texture& other);
    ~__Texture();

private:
    void LoadImageFile(const std::string path);
    void CreateImage();
    void CreateSampler();

    __Image image;
    VkSampler sampler;
    VkDescriptorSet descriptorSet;
    
    int width;
    int height;
    int channels;
    void* data;

    std::shared_ptr<uint32_t> useCount;
};

}