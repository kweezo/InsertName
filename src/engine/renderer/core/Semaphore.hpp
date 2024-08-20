#pragma once

//NOTE shitty wrappers like these were only created to manage their lifetime easier


#include <stdexcept>
#include <memory>

#include <vulkan/vulkan.h>

#include "Device.hpp"

namespace renderer {
    struct i_SemaphoreCreateInfo {
        VkSemaphoreType type;
        uint64_t initalValue;
    };

    class i_Semaphore {
    public:
        i_Semaphore();

        i_Semaphore(i_SemaphoreCreateInfo createInfo);

        ~i_Semaphore();

        i_Semaphore(const i_Semaphore &other);

        i_Semaphore &operator=(const i_Semaphore &other);

        [[nodiscard]] bool IsInitialized() const;

        [[nodiscard]] VkSemaphore GetSemaphore() const;

        [[nodiscard]] bool IsTimelineSemaphore();

        void Destruct();

    private:
        VkSemaphore semaphore = VK_NULL_HANDLE;

        std::shared_ptr<uint32_t> useCount;

        bool isTimeline;
    };
}
