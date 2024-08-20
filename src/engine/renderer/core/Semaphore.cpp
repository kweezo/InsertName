#include "Semaphore.hpp"

namespace renderer {
    i_Semaphore::i_Semaphore(): semaphore(VK_NULL_HANDLE) {
    }

    i_Semaphore::i_Semaphore(i_SemaphoreCreateInfo createInfo): semaphore(VK_NULL_HANDLE) {
        if (!i_Device::IsInitialized()) {
            return;
        }


        isTimeline = createInfo.type;

        VkSemaphoreTypeCreateInfo typeInfo{};
        typeInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        typeInfo.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;

        if(isTimeline){
            typeInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
            typeInfo.initialValue = createInfo.initalValue;
        }

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreInfo.pNext = &typeInfo;
        

        if (vkCreateSemaphore(i_Device::GetDevice(), &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create fence");
        }

        useCount = std::make_shared<uint32_t>(1);
    }

    VkSemaphore i_Semaphore::GetSemaphore() const {
        if (semaphore == VK_NULL_HANDLE) {
            throw std::runtime_error("Tried to return an uninitialized semaphore");
        }


        return semaphore;
    }

    bool i_Semaphore::IsInitialized() const {
        return semaphore != VK_NULL_HANDLE;
    }

    i_Semaphore::i_Semaphore(const i_Semaphore &other) {
        if (other.useCount == nullptr) {
            return;
        }

        semaphore = other.semaphore;
        useCount = other.useCount;
        (*useCount)++;
    }

    i_Semaphore &i_Semaphore::operator=(const i_Semaphore &other) {
        if (this == &other) {
            return *this;
        }

        if (other.useCount == nullptr) {
            return *this;
        }

        Destruct();

        semaphore = other.semaphore;
        useCount = other.useCount;
        (*useCount)++;
        return *this;
    }

    void i_Semaphore::Destruct() {
        if (useCount == nullptr) {
            return;
        }

        if (*useCount <= 1) {
            vkDestroySemaphore(i_Device::GetDevice(), semaphore, nullptr);
            useCount.reset();
        } else {
            (*useCount)--;
        }
    }

    i_Semaphore::~i_Semaphore() {
        Destruct();
    }
}
