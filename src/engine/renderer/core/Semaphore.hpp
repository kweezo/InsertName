#pragma once

//NOTE shitty wrappers like these were only created to manage their lifetime easier


#include <stdexcept>
#include <memory>

#include <vulkan/vulkan.h>

#include "Device.hpp"

namespace renderer{

struct _SemaphoreCreateInfo{

};

class _Semaphore{
public:
    _Semaphore();
    _Semaphore(_SemaphoreCreateInfo createInfo);
    ~_Semaphore();

    _Semaphore(const _Semaphore& other);
    _Semaphore& operator=(const _Semaphore& other);

    bool IsInitialized() const;

    VkSemaphore GetSemaphore() const;
private:
    void Destruct();

    VkSemaphore semaphore = VK_NULL_HANDLE;
    std::shared_ptr<uint32_t> useCount;
};

}