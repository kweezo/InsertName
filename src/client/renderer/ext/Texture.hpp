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
//    ~Texture();


    VkImageView GetTextureImageView();
    VkSampler GetTextureSampler();
private:
    void LoadTexture(const std::string& path);
    void CreateTextureImage();
    void CreateTextureImageView();
    void CreateTextureSampler();


    Image image;

    do_not_use_ImageData imageData;
};

}