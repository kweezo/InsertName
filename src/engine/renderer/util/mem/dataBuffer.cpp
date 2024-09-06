#include "dataBuffer.hpp"

namespace renderer{

    i_DataBuffer::i_DataBuffer():
     buffer(VK_NULL_HANDLE), allocation(VK_NULL_HANDLE), stagingBuffer(VK_NULL_HANDLE), stagingAllocation(VK_NULL_HANDLE){

    }

    i_DataBuffer::i_DataBuffer(i_DataBufferCreateInfo createInfo): createInfo(createInfo),
     buffer(VK_NULL_HANDLE), allocation(VK_NULL_HANDLE), stagingBuffer(VK_NULL_HANDLE), stagingAllocation(VK_NULL_HANDLE){
        state = i_DataBufferState::INIT;

        i_Scheduler::GetScheduler().AddTaskSetToPipe(this);
    }


    void i_DataBuffer::ExecuteRange(enki::TaskSetPartition range, uint32_t threadNum){
        switch(state){
            case INIT:
                Init();
                break;
        }
    }

    void i_DataBuffer::Init(){
        CreateBuffer();
        CreateStagingBuffer();
        UploadDataToStagingMemory();
    }

    
    void i_DataBuffer::CreateBuffer(){
    


        VkBufferUsageFlags additionalUsage = 0;

        if(i_PhysicalDevice::DeviceMemoryAvailable()){
            additionalUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        }

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

        bufferInfo.usage = createInfo.usage | additionalUsage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
/*
        if(queueFamilyIndices.size() > 1){
            std::vector<uint32_t> queueFamilyIndices
             = i_LogicalDevice::GetQueueFamilyIndices(createInfo.flags);

            bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
            bufferInfo.pQueueFamilyIndices = queueFamilyIndices.data()u
            bufferInfo.queueFamilyIndexCount = queueFamilyIndices.size();
        }        
*/
        bufferInfo.size = createInfo.size;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.requiredFlags = i_PhysicalDevice::DeviceMemoryAvailable() ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

        

        if(vmaCreateBuffer(i_LogicalDevice::GetAllocator(), &bufferInfo, &allocInfo, &buffer,
         &allocation, nullptr) != VK_SUCCESS){
            throw std::runtime_error("ERROR: Command vmaCreateBuffer returned failure");
        }
    }

    void i_DataBuffer::CreateStagingBuffer(){
        if(!i_PhysicalDevice::DeviceMemoryAvailable()){
            return;
        }

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

        if(vmaCreateBuffer(i_LogicalDevice::GetAllocator(), &bufferInfo, &allocInfo, &stagingBuffer,
         &stagingAllocation, nullptr) != VK_SUCCESS){
            throw std::runtime_error("ERROR: Command vmaCreateBuffer returned failure when trying to create the staging buffer");
        }

    }
    
    void i_DataBuffer::UploadDataToStagingMemory(){
        if(!i_PhysicalDevice::DeviceMemoryAvailable()){
            return;
        }


        void* mappedMem;

        vmaMapMemory(i_LogicalDevice::GetAllocator(), stagingAllocation, &mappedMem);

        memcpy(mappedMem, createInfo.data, createInfo.size);

        vmaUnmapMemory(i_LogicalDevice::GetAllocator(), stagingAllocation);
    }


    void i_DataBuffer::CopyFrom(const i_DataBuffer& other){
        buffer = other.buffer;
        allocation = other.allocation;
        stagingBuffer = other.stagingBuffer;
        stagingAllocation = other.stagingAllocation;

        state = other.state;
        createInfo = other.createInfo;

        useCount = other.useCount;

        (*useCount)++;
    }
    
    i_DataBuffer::i_DataBuffer(const i_DataBuffer& other){
        CopyFrom(other);

        Destruct();
    }

    i_DataBuffer& i_DataBuffer::operator=(const i_DataBuffer& other){
        if(this == &other || useCount == nullptr){
            return *this;
        }

        CopyFrom(other);

        Destruct();

        return *this;
    }
    
    void i_DataBuffer::Destruct(){
        if(buffer == VK_NULL_HANDLE || *useCount == 1){
            return;
        }

        vmaDestroyBuffer(i_LogicalDevice::GetAllocator(), buffer, allocation);

        if(stagingBuffer == VK_NULL_HANDLE){
            vmaDestroyBuffer(i_LogicalDevice::GetAllocator(), stagingBuffer, stagingAllocation);
        } 

        (*useCount)--;
    }

    i_DataBuffer::~i_DataBuffer(){
        Destruct();
    }
}