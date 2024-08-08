#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_map>
#include <memory>
#include <list>

#include "../core/DataBuffer.hpp"
#include "../core/Device.hpp"
#include "../core/DescriptorManager.hpp"
#include "client/account/Settings.hpp"
#include "../core/Image.hpp"
#include "../core/Shader.hpp"

namespace renderer{

struct i_TextureCreateInfo{
    std::string path;
    uint32_t binding;

    std::vector<std::weak_ptr<i_Shader>> shaders;
};


class i_Texture{
public:
    static void Update();

    i_Texture();
    i_Texture(i_TextureCreateInfo createInfo);
    i_Texture(const i_Texture& other);
    i_Texture& operator=(const i_Texture& other);
    ~i_Texture();

    void SetBinding(uint32_t binding);

private:
    void Destruct();

    void LoadImageFile(std::string path);
    void CreateImage();
    void CreateSampler();

    i_Image image;
    VkSampler sampler;
    VkDescriptorSet descriptorSet;
    
    int width;
    int height;
    int channels;
    void* data;

    std::vector<std::weak_ptr<i_Shader>> shaders;

    uint32_t binding;

    std::shared_ptr<uint32_t> useCount;


    static std::vector<VkWriteDescriptorSet> writeDescriptorSetsQueue;
    static std::list<VkDescriptorImageInfo> imageInfoList;
};

}