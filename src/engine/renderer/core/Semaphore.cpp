#include "Semaphore.hpp"

namespace renderer{

__Semaphore::__Semaphore(){
    if(!__Device::IsInitialized()){
        return;
    }

    VkSemaphoreCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if(vkCreateSemaphore(__Device::GetDevice(), &fenceInfo, nullptr, &fence) != VK_SUCCESS){
        throw std::runtime_error("Failed to create fence");
    }

    useCount = std::make_shared<uint32_t>(1);
}

VkSemaphore __Semaphore::GetSemaphore(){
    return fence;
}

__Semaphore::__Semaphore(const __Semaphore& other){
    fence = other.fence;
    useCount = other.useCount;
    (*useCount.get())++;
}

__Semaphore& __Semaphore::operator=(const __Semaphore& other){
    if(this == &other){
        return *this;
    }

    fence = other.fence;
    useCount = other.useCount;
    (*useCount.get())++;
    return *this;
}

__Semaphore::~__Semaphore(){
    if(useCount.get() == nullptr){
        return;
    }

    if(*useCount <= 1){
        vkDestroySemaphore(__Device::GetDevice(), fence, nullptr);
        useCount.reset();
    }else{
        (*useCount.get())--;
    }
}

}