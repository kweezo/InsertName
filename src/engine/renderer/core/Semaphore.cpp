#include "Semaphore.hpp"

namespace renderer{

_Semaphore::_Semaphore(): semaphore(VK_NULL_HANDLE){
}

_Semaphore::_Semaphore(_SemaphoreCreateInfo createInfo): semaphore(VK_NULL_HANDLE){
    if(!_Device::IsInitialized()){
        return;
    }

    VkSemaphoreCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if(vkCreateSemaphore(_Device::GetDevice(), &fenceInfo, nullptr, &semaphore) != VK_SUCCESS){
        throw std::runtime_error("Failed to create fence");
    }

    useCount = std::make_shared<uint32_t>(1);
}

VkSemaphore _Semaphore::GetSemaphore() const{
    if(semaphore == VK_NULL_HANDLE){
        throw std::runtime_error("Tried to return an uninitialized semaphore");
    }

    return semaphore;
}

bool _Semaphore::IsInitialized() const{
    return semaphore != VK_NULL_HANDLE;
}

_Semaphore::_Semaphore(const _Semaphore& other){
    if(other.useCount.get() == nullptr){
        return;
    }

    semaphore = other.semaphore;
    useCount = other.useCount;
    (*useCount.get())++;
}

_Semaphore& _Semaphore::operator=(const _Semaphore& other){
    if(this == &other){
        return *this;
    }

    if(other.useCount.get() == nullptr){
        return *this;
    }

    Destruct();

    semaphore = other.semaphore;
    useCount = other.useCount;
    (*useCount.get())++;
    return *this;
}

void _Semaphore::Destruct(){
    if(useCount.get() == nullptr){
        return;
    }

    if(*useCount <= 1){
        vkDestroySemaphore(_Device::GetDevice(), semaphore, nullptr);
        useCount.reset();
    }else{
        (*useCount.get())--;
    }   
}

_Semaphore::~_Semaphore(){
    Destruct();
}

}