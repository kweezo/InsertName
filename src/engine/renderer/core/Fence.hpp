#pragma once

//NOTE shitty wrappers like these were only created to manage their lifetime easier


#include <stdexcept>
#include <memory>

#include <vulkan/vulkan.h>

#include "Device.hpp"

namespace renderer{

class _Fence{
public:
    _Fence();
    _Fence(bool signaled);
    ~_Fence();

    _Fence(const _Fence& other);
    _Fence& operator=(const _Fence& other);

    bool IsInitialized() const;

    VkFence GetFence() const;

    void Destruct();
private:

    VkFence fence = VK_NULL_HANDLE;
    
    std::shared_ptr<uint32_t> useCount;
};

}