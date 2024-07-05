#pragma once

//NOTE shitty wrappers like these were only created to manage their lifetime easier


#include <stdexcept>
#include <memory>

#include <vulkan/vulkan.h>

#include "Device.hpp"

namespace renderer{


class __Semaphore{
public:
    __Semaphore();
    ~__Semaphore();

    __Semaphore(const __Semaphore& other);
    __Semaphore& operator=(const __Semaphore& other);

    VkSemaphore GetSemaphore();
private:
    VkSemaphore fence;
    std::shared_ptr<uint32_t> useCount;
};

typedef struct RenderSemaphores{
    __Semaphore renderFinishedSemaphore;
    __Semaphore imageAvailableSemaphore;
} RenderSemaphores;

}