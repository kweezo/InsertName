#pragma once

//NOTE shitty wrappers like these were only created to manage their lifetime easier


#include <stdexcept>

#include <vulkan/vulkan.h>

#include "Device.hpp"

class Fence{
public:
    Fence();
    Fence(bool signaled);
    ~Fence();

    Fence(const Fence& other);
    Fence& operator=(const Fence& other);

    VkFence GetFence();
private:
    VkFence fence;
    uint32_t* useCount;
};
