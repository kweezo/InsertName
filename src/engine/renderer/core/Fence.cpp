#include "Fence.hpp"

namespace renderer{

__Fence::__Fence(): fence(VK_NULL_HANDLE){
    useCount = std::make_shared<uint32_t>(1);
}

__Fence::__Fence(bool signaled): fence(VK_NULL_HANDLE) {
    if(!__Device::IsInitialized()){
        return;
    }

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

    if(vkCreateFence(__Device::GetDevice(), &fenceInfo, nullptr, &fence) != VK_SUCCESS){
        throw std::runtime_error("Failed to create fence");
    }

    useCount = std::make_shared<uint32_t>(1);
}

VkFence __Fence::GetFence() const{
    if(fence == VK_NULL_HANDLE){
        throw std::runtime_error("Tried to return an uninitialized fence");
    }

    return fence;
}

bool __Fence::IsInitialized() const{
    return fence != VK_NULL_HANDLE;
}

__Fence::__Fence(const __Fence& other){
    fence = other.fence;
    useCount = other.useCount;
    (*useCount.get())++;
}

__Fence& __Fence::operator=(const __Fence& other){
    if(this == &other){
        return *this;
    }

    fence = other.fence;
    useCount = other.useCount;
    (*useCount.get())++;

    return *this;
}

__Fence::~__Fence(){
    if(useCount.get() == nullptr){
        return;
    }

    if(*useCount == 1){
        if(fence != VK_NULL_HANDLE){
            vkDestroyFence(__Device::GetDevice(), fence, nullptr);
        }
        useCount.reset();
    }else{
        (*useCount.get())--;
    }
}

}