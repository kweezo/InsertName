#include "Semaphore.hpp"

namespace renderer{

__Semaphore::__Semaphore(): semaphore(VK_NULL_HANDLE){
}

__Semaphore::__Semaphore(__SemaphoreCreateInfo createInfo): semaphore(VK_NULL_HANDLE){
    if(!__Device::IsInitialized()){
        return;
    }

    VkSemaphoreCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if(vkCreateSemaphore(__Device::GetDevice(), &fenceInfo, nullptr, &semaphore) != VK_SUCCESS){
        throw std::runtime_error("Failed to create fence");
    }

    useCount = std::make_shared<uint32_t>(1);
}

VkSemaphore __Semaphore::GetSemaphore() const{
    if(semaphore == VK_NULL_HANDLE){
        throw std::runtime_error("Tried to return an uninitialized semaphore");
    }

    return semaphore;
}

bool __Semaphore::IsInitialized() const{
    return semaphore != VK_NULL_HANDLE;
}

__Semaphore::__Semaphore(const __Semaphore& other){
    if(useCount.get() == nullptr){
        return;
    }

    semaphore = other.semaphore;
    useCount = other.useCount;
    (*useCount.get())++;
}

__Semaphore& __Semaphore::operator=(const __Semaphore& other){
    if(this == &other){
        return *this;
    }

    if(useCount.get() == nullptr){
        return *this;
    }

    semaphore = other.semaphore;
    useCount = other.useCount;
    (*useCount.get())++;
    return *this;
}

__Semaphore::~__Semaphore(){
    if(useCount.get() == nullptr){
        return;
    }

    if(*useCount <= 1){
        vkDestroySemaphore(__Device::GetDevice(), semaphore, nullptr);
        useCount.reset();
    }else{
        (*useCount.get())--;
    }
}

}