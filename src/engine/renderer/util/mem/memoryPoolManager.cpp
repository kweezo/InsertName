#include "memoryPoolManager.hpp"

namespace renderer{


    std::unique_ptr<i_MemoryPoolManager> i_MemoryPoolManager::poolManager;

    void i_MemoryPoolManager::Create(){
        poolManager.reset(new i_MemoryPoolManager);
    }

    void i_MemoryPoolManager::Destroy(){
        poolManager.reset();
    }

    i_MemoryPoolManager::i_MemoryPoolManager(){

    }

    i_MemoryPoolManager::~i_MemoryPoolManager(){
        for(std::list<VmaPool> poolList : pools){
            for(VmaPool pool : poolList){
                vmaDestroyPool(i_LogicalDevice::GetAllocator(), pool);
            }
        }
    }

}
