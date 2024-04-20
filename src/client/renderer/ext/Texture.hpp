#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_map>

#include "../core/DataBuffer.hpp"
#include "../core/Device.hpp"
#include "../core/Image.hpp"
#include "../core/DescriptorManager.hpp"

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
    static TextureHandle CreateTexture(const std::string& path, uint32_t binding);
    static void Free(TextureHandle texture);

    static void EnableTextures();
};

class TextureImpl {
public:
    TextureImpl(const std::string& path, uint32_t binding);
    ~TextureImpl();
    TextureImpl(const TextureImpl& other);
    TextureImpl& operator=(const TextureImpl& other);

    VkImageView GetTextureImageView();
    VkSampler GetTextureSampler();
    VkWriteDescriptorSet GetWriteDescriptorSet();

    void SetDescriptorSet(VkDescriptorSet descriptorSet);

    static void EnableTextures();
private:
    void LoadTexture(const std::string& path);
    void CreateTextureImage();
    void CreateTextureImageView();
    void CreateTextureSampler();

    static std::unordered_map<uint32_t, TextureBindingInfo> texturesPerBinding;
    uint32_t binding;

    uint32_t *useCount;

    ImageHandle image;
    VkImageView imageView;
    VkSampler sampler;

    VkDescriptorSet descriptorSet;

    TextureImageData imageData;
};

}