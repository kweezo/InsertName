#include "Fence.hpp"

namespace renderer {
    i_Fence::i_Fence(): fence(VK_NULL_HANDLE) {
    }

    i_Fence::i_Fence(bool signaled): fence(VK_NULL_HANDLE) {
        if (!i_Device::IsInitialized()) {
            return;
        }

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

        if (vkCreateFence(i_Device::GetDevice(), &fenceInfo, nullptr, &fence) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create fence");
        }

        useCount = std::make_shared<uint32_t>(1);
    }

    VkFence i_Fence::GetFence() const {
        if (fence == VK_NULL_HANDLE) {
            throw std::runtime_error("Tried to return an uninitialized fence");
        }

        return fence;
    }

    bool i_Fence::IsInitialized() const {
        return fence != VK_NULL_HANDLE;
    }

    i_Fence::i_Fence(const i_Fence &other) {
        if (other.useCount == nullptr) {
            return;
        }

        fence = other.fence;
        useCount = other.useCount;
        (*useCount)++;
    }

    i_Fence &i_Fence::operator=(const i_Fence &other) {
        if (this == &other) {
            return *this;
        }

        if (other.useCount == nullptr) {
            return *this;
        }

        Destruct();

        fence = other.fence;
        useCount = other.useCount;
        (*useCount)++;

        return *this;
    }

    void i_Fence::Destruct() {
        if (useCount == nullptr) {
            return;
        }

        if (*useCount == 1) {
            if (fence != VK_NULL_HANDLE) {
                vkDestroyFence(i_Device::GetDevice(), fence, nullptr);
            }
            useCount.reset();
        } else {
            (*useCount)--;
        }
    }

    i_Fence::~i_Fence() {
        Destruct();
    }
}
