#include "Fence.hpp"

namespace renderer{

Fence::Fence(): fence(VK_NULL_HANDLE){

    useCount = new uint32_t;
    *useCount = 1;
}

Fence::Fence(bool signaled){
    if(!Device::IsInitialized()){
        return;
    }

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

    if(vkCreateFence(Device::GetDevice(), &fenceInfo, nullptr, &fence) != VK_SUCCESS){
        throw std::runtime_error("Failed to create fence");
    }

    useCount = new uint32_t;
    *useCount = 1;
}

VkFence Fence::GetFence(){
    return fence;
}

Fence::Fence(const Fence& other){
    fence = other.fence;
    useCount = other.useCount;
    *useCount += 1;
}

Fence& Fence::operator=(const Fence& other){
    if(this == &other){
        return *this;
    }

    fence = other.fence;
    useCount = other.useCount;
    *useCount += 1;

    return *this;
}

Fence::~Fence(){
    if(useCount == nullptr){
        return;
    }

    if(*useCount == 1){
        if(fence != VK_NULL_HANDLE){
            vkDestroyFence(Device::GetDevice(), fence, nullptr);
        }
        delete useCount;
        useCount = nullptr;
    }else{
        *useCount -= 1;
    }
}

}