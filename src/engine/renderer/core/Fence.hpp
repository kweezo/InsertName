#pragma once

//NOTE shitty wrappers like these were only created to manage their lifetime easier


#include <stdexcept>
#include <memory>

#include <vulkan/vulkan.h>

#include "Device.hpp"

namespace renderer{

class __Fence{
public:
    __Fence();
    __Fence(bool signaled);
    ~__Fence();

    __Fence(const __Fence& other);
    __Fence& operator=(const __Fence& other);

    bool IsInitialized() const;

    VkFence GetFence() const;
private:
    VkFence fence;
    
    std::shared_ptr<uint32_t> useCount;
};

}