#pragma once

//NOTE shitty wrappers like these were only created to manage their lifetime easier


#include <stdexcept>
#include <memory>

#include <vulkan/vulkan.h>

#include "Device.hpp"

namespace renderer{

struct __SemaphoreCreateInfo{

};

class __Semaphore{
public:
    __Semaphore();
    __Semaphore(__SemaphoreCreateInfo createInfo);
    ~__Semaphore();

    __Semaphore(const __Semaphore& other);
    __Semaphore& operator=(const __Semaphore& other);

    bool IsInitialized() const;

    VkSemaphore GetSemaphore() const;
private:
    VkSemaphore semaphore;
    std::shared_ptr<uint32_t> useCount;
};

typedef struct RenderSemaphores{
    __Semaphore renderFinishedSemaphore;
    __Semaphore imageAvailableSemaphore;
} RenderSemaphores;

}