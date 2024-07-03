#pragma once

//NOTE shitty wrappers like these were only created to manage their lifetime easier


#include <stdexcept>
#include <memory>

#include <vulkan/vulkan.h>

#include "Device.hpp"

namespace renderer{


class Semaphore{
public:
    Semaphore();
    ~Semaphore();

    Semaphore(const Semaphore& other);
    Semaphore& operator=(const Semaphore& other);

    VkSemaphore GetSemaphore();
private:
    VkSemaphore fence;
    std::shared_ptr<uint32_t> useCount;
};

typedef struct RenderSemaphores{
    Semaphore renderFinishedSemaphore;
    Semaphore imageAvailableSemaphore;
} RenderSemaphores;

}