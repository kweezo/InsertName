#include "Semaphore.hpp"

namespace renderer{

Semaphore::Semaphore(){
    VkSemaphoreCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    if(vkCreateSemaphore(Device::GetDevice(), &fenceInfo, nullptr, &fence) != VK_SUCCESS){
        throw std::runtime_error("Failed to create fence");
    }

    useCount = new uint32_t;
    useCount[0] = 1;
}

VkSemaphore Semaphore::GetSemaphore(){
    return fence;
}

Semaphore::Semaphore(const Semaphore& other){
    fence = other.fence;
    useCount = other.useCount;
    useCount[0]++;
}

Semaphore& Semaphore::operator=(const Semaphore& other){
    if(this == &other){
        return *this;
    }

    fence = other.fence;
    useCount = other.useCount;
    useCount[0]++;
    return *this;
}

Semaphore::~Semaphore(){
    if(useCount[0] <= 1){
        vkDestroySemaphore(Device::GetDevice(), fence, nullptr);
        delete useCount;
    }else{
        useCount[0]--;
    }
}

}