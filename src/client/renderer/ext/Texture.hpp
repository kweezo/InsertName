#pragma once

#include <string>
#include <vector>
#include <stdexcept>

#include "../core/DataBuffer.hpp"
#include "../core/Device.hpp"
#include "../core/Image.hpp"


namespace renderer{


typedef struct {
    int width, height, channels;
    unsigned char* dat;
}do_not_use_ImageData;

class Texture {
public:
    Texture(const std::string& path);
    ~Texture();
    Texture(const Texture& other);
    Texture& operator=(const Texture& other);


    VkImageView GetTextureImageView();
    VkSampler GetTextureSampler();

    static void EnableNewTextures();
private:
    void LoadTexture(const std::string& path);
    void CreateTextureImage();
    void CreateTextureImageView();
    void CreateTextureSampler();

    uint32_t *useCount;

    ImageHandle image;
    do_not_use_ImageData imageData;
};

}