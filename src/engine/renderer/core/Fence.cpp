#include "Fence.hpp"

namespace renderer{

Fence::Fence(): fence(VK_NULL_HANDLE){
    useCount = std::make_shared<uint32_t>(1);
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

    useCount = std::make_shared<uint32_t>(1);
}

VkFence Fence::GetFence(){
    return fence;
}

Fence::Fence(const Fence& other){
    fence = other.fence;
    useCount = other.useCount;
    (*useCount.get())++;
}

Fence& Fence::operator=(const Fence& other){
    if(this == &other){
        return *this;
    }

    fence = other.fence;
    useCount = other.useCount;
    (*useCount.get())++;

    return *this;
}

Fence::~Fence(){
    if(useCount.get() == nullptr){
        return;
    }

    if(*useCount == 1){
        if(fence != VK_NULL_HANDLE){
            vkDestroyFence(Device::GetDevice(), fence, nullptr);
        }
        useCount.reset();
    }else{
        (*useCount.get())--;
    }
}

}