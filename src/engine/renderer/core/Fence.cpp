#include "Fence.hpp"

namespace renderer{

_Fence::_Fence(): fence(VK_NULL_HANDLE){
}

_Fence::_Fence(bool signaled): fence(VK_NULL_HANDLE) {
    if(!_Device::IsInitialized()){
        return;
    }

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

    if(vkCreateFence(_Device::GetDevice(), &fenceInfo, nullptr, &fence) != VK_SUCCESS){
        throw std::runtime_error("Failed to create fence");
    }

    useCount = std::make_shared<uint32_t>(1);
}

VkFence _Fence::GetFence() const{
    if(fence == VK_NULL_HANDLE){
        throw std::runtime_error("Tried to return an uninitialized fence");
    }

    return fence;
}

bool _Fence::IsInitialized() const{
    return fence != VK_NULL_HANDLE;
}

_Fence::_Fence(const _Fence& other){
    if(other.useCount.get() == nullptr){
        return;
    }

    fence = other.fence;
    useCount = other.useCount;
    (*useCount.get())++;
}

_Fence& _Fence::operator=(const _Fence& other){
    if(this == &other){
        return *this;
    }

    if(other.useCount.get() == nullptr){
        return *this;
    }

    Destruct();

    fence = other.fence;
    useCount = other.useCount;
    (*useCount.get())++;

    return *this;
}

void _Fence::Destruct(){
    if(useCount.get() == nullptr){
        return;
    }

    if(*useCount == 1){
        if(fence != VK_NULL_HANDLE){
            vkDestroyFence(_Device::GetDevice(), fence, nullptr);
        }
        useCount.reset();
    }else{
        (*useCount.get())--;
    }

}

_Fence::~_Fence(){
    Destruct();
}

}