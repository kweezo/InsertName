#pragma once


#include <vector>
#include <stdexcept>

#include <vulkan/vulkan.h>

#include "DataBuffer.hpp"
#include "DescriptorManager.hpp"

namespace renderer{
class UniformBuffer{
public:
    template<typename T>//templates are a kind of mental retardation and thats why I love them
    UniformBuffer(T& data, uint32_t index);

    template<typename T>
    void UpdateData(T& data);

    static void EnableBuffers();

    void AssignDescriptorHandle(DescriptorHandle handle);// DO NOT TOUCH THIS UNLESS YOU KNOW WHY ITS LIKE THIS,
    // NO HARAM
private:
    DataBuffer dataBuffer;
    DescriptorHandle descriptorHandle;

    size_t dataSize;

    static std::vector<UniformBuffer*> instanceList;
    static bool creationLock; //i mean we have to do a bunch of shit if we do it after creating descriptor sets, so we ain't gonna allow it (for now)
    //maybe implement like a soft reintialization for when loading new maps or shit that just resets all of this?
};

}