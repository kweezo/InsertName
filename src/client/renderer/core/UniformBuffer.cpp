#include "UniformBuffer.hpp"

namespace renderer{

bool UniformBuffer::creationLock = false;
std::vector<UniformBuffer*> UniformBuffer::instanceList = {};
    
template<typename T>
UniformBuffer::UniformBuffer(T& data, uint32_t index){
    if(creationLock){
        throw std::runtime_error("Tried to create a buffer after enabling them, \n which is thus far not allowed.");
    }

    dataBuffer = DataBuffer(DATA_BUFFER_UNIFORM_BIT, sizeof(T), &data, true, DATA_BUFFER_UNIFORM_BIT);
    
    dataSize = sizeof(T);
    instanceList.push_back(this);
}

template<typename T>
void UniformBuffer::UpdateData(T& data){
    if(dataSize != sizeof(T)){
        throw std::runtime_error("Data size mismatch when trying to update uniform buffer data, expected size: " + std::to_string(dataSize) + " actual size: " + std::to_string(sizeof(T)));
    }

    dataBuffer.UpdateData(&data, sizeof(T));
}

void UniformBuffer::EnableBuffers(){
    DescriptorBatchInfo batchInfo{};

//TODO lmao    batchInfo.layoutIndex = 
    batchInfo.setCount = instanceList.size();

    std::vector<DescriptorHandle> descriptorBatch = DescriptorManager::CreateDescriptors({batchInfo});
}

}