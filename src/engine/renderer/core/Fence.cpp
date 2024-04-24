#include "Fence.hpp"

namespace renderer{

Fence::Fence(){
    useCount = new uint32_t;
    useCount[0] = 1;
}

Fence::Fence(bool signaled){
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

    if(vkCreateFence(Device::GetDevice(), &fenceInfo, nullptr, &fence) != VK_SUCCESS){
        throw std::runtime_error("Failed to create fence");
    }

    useCount = new uint32_t;
    useCount[0] = 1;
}

VkFence Fence::GetFence(){
    return fence;
}

Fence::Fence(const Fence& other){
    fence = other.fence;
    useCount = other.useCount;
    useCount[0]++;
}

Fence& Fence::operator=(const Fence& other){
    if(this == &other){
        return *this;
    }

    fence = other.fence;
    useCount = other.useCount;
    useCount[0]++;
    return *this;
}

Fence::~Fence(){
    if(useCount[0] <= 1){
        vkDestroyFence(Device::GetDevice(), fence, nullptr);
        delete useCount;
    }else{
        useCount[0]--;
    }
}

}