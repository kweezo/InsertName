#pragma once

//NOTE shitty wrappers like these were only created to manage their lifetime easier


#include <stdexcept>
#include <memory>

#include <vulkan/vulkan.h>

#include "Device.hpp"

namespace renderer {
    class i_Fence {
    public:
        i_Fence();

        i_Fence(bool signaled);

        ~i_Fence();

        i_Fence(const i_Fence &other);

        i_Fence &operator=(const i_Fence &other);

        [[nodiscard]] bool IsInitialized() const;

        [[nodiscard]] VkFence GetFence() const;

        void Destruct();

    private:
        VkFence fence = VK_NULL_HANDLE;

        std::shared_ptr<uint32_t> useCount;
    };
}
