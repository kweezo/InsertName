#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_map>

#include "../core/DataBuffer.hpp"
#include "../core/Device.hpp"
#include "../core/Image.hpp"
#include "../core/DescriptorManager.hpp"
#include "client/account/Settings.hpp"

#define TextureHandle TextureImpl*

namespace renderer{


typedef struct {
    int width, height, channels;
    unsigned char* dat;
}TextureImageData;

class TextureImpl;

typedef struct TextureBindingInfo{
    uint32_t count;
    std::vector<TextureImpl*> handles;
}TextureBindingInfo;

class Texture{
public:
    static TextureHandle CreateTexture(const std::string& path, uint32_t binding, VkDescriptorSet descriptorSet);
    static void Free(TextureHandle texture);

    static void EnableTextures();
};

class TextureImpl {
public:
    TextureImpl(const std::string& path, uint32_t binding, VkDescriptorSet descriptorSet);
    ~TextureImpl();
    TextureImpl(const TextureImpl& other);
    TextureImpl& operator=(const TextureImpl& other);

    VkImageView GetTextureImageView();
    VkSampler GetTextureSampler();
    VkWriteDescriptorSet GetWriteDescriptorSet();

    ImageHandle GetImage();

    void SetDescriptorSet(VkDescriptorSet descriptorSet);
    void FreeImageData();

    static void EnableTextures();
private:
    void LoadTexture(const std::string& path);
    void CreateTextureImage();
    void CreateTextureSampler();

    uint32_t *useCount;

    static std::vector<TextureHandle> handles;

    uint32_t binding;

    ImageHandle image;
    VkSampler sampler;

    VkDescriptorSet descriptorSet;

    TextureImageData imageData;
};

}