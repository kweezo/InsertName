#include "DataBuffer.hpp"

namespace renderer {
    const uint32_t TARGET_STAGING_BUFFER_COUNT_PER_THREAD = 5;

    const size_t FREE_STAGING_MEMORY_TRESHOLD = 1024;

    std::vector<std::vector<i_DataBufferStagingCommandBuferData> > i_DataBuffer::stagingCommandBuffers = {};
    std::list<std::pair<VkBuffer, VkDeviceMemory> > i_DataBuffer::stagingBufferAndMemoryDeleteQueue = {};
    i_Fence i_DataBuffer::finishedCopyingFence = {};
    std::set<uint32_t> i_DataBuffer::resetPoolIndexes = {};
    bool i_DataBuffer::anyCommandBuffersRecorded = false;
    std::vector<std::pair<std::shared_ptr<std::thread>, std::shared_ptr<bool>>> i_DataBuffer::queueWaitThreads = {};

    void i_DataBuffer::Init() {
        CreateCommandBuffers();

        finishedCopyingFence = i_Fence(false);
    }

    void i_DataBuffer::CreateCommandBuffers() {
        uint32_t i = 0;
        stagingCommandBuffers.resize(std::thread::hardware_concurrency());
        for (std::vector<i_DataBufferStagingCommandBuferData> &commandBuffers: stagingCommandBuffers) {
            i_CommandBufferCreateInfo stagingCommandBufferInfo;
            stagingCommandBufferInfo.type = i_CommandBufferType::DATA;
            stagingCommandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            stagingCommandBufferInfo.flags = COMMAND_POOL_TYPE_TRANSFER | COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG;
            stagingCommandBufferInfo.threadIndex = i;

            commandBuffers.resize(TARGET_STAGING_BUFFER_COUNT_PER_THREAD);

            for (i_DataBufferStagingCommandBuferData &data: commandBuffers) {
                data = {i_CommandBuffer(stagingCommandBufferInfo), {}, true, i_Semaphore(i_SemaphoreCreateInfo{})};
            }

            i = (i++) % std::thread::hardware_concurrency();
        }
    }

    void i_DataBuffer::Update() {
        std::list<std::unique_ptr<std::lock_guard<std::mutex> > > lockGuards;
        for (std::vector<i_DataBufferStagingCommandBuferData> &commandBuffers : stagingCommandBuffers) {
            for (i_DataBufferStagingCommandBuferData &data : commandBuffers) {
                if (!data.free) {
                    if (data.mutex.expired()) {
                        throw std::runtime_error(
                            "Tried to update an already destroyed data buffer; vkCommandBuffer " + std::format(
                                "0x{:X}", (size_t) data.commandBuffer.GetCommandBuffer()));
                    }
                    lockGuards.emplace_back(std::make_unique<std::lock_guard<std::mutex> >(*data.mutex.lock().get()));
                }
            }
        }


        SubmitCommandBuffers();
        UpdateCleanup();
    }

    void i_DataBuffer::Cleanup() {
        stagingCommandBuffers.clear();
        finishedCopyingFence.~i_Fence();
    }


    i_DataBuffer::i_DataBuffer() : buffer(VK_NULL_HANDLE), memory(VK_NULL_HANDLE) {
    }

    i_DataBuffer::i_DataBuffer(i_DataBufferCreateInfo createInfo) : createInfo(createInfo),
                                                                    stagingBuffer(VK_NULL_HANDLE),
                                                                    stagingMemory(VK_NULL_HANDLE),
                                                                    buffer(VK_NULL_HANDLE), memory(VK_NULL_HANDLE){
        if (!i_Device::DeviceMemoryFree()) {
            createInfo.transferToLocalDeviceMemory = false;
        }

        bufferMutex.reset(new std::mutex);
    
        if (createInfo.transferToLocalDeviceMemory) {
            {
                std::lock_guard<std::mutex> lock(bufferCreationMutex);
                CreateBuffer(buffer, createInfo.usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, createInfo.size);
            }
        
            AllocateMemory(memory, buffer, createInfo.size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        
            CreateBuffer(stagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, createInfo.size);
            AllocateMemory(stagingMemory, stagingBuffer, createInfo.size,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                    
            UploadDataToMemory(stagingMemory, createInfo.data, createInfo.size);
        
            RecordCopyCommandBuffer(createInfo.threadIndex, createInfo.size);
        } else {
            {
                std::lock_guard<std::mutex> lock(bufferCreationMutex);
                CreateBuffer(buffer, createInfo.usage, createInfo.size);
            }
        
            AllocateMemory(memory, buffer, createInfo.size,
                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
                    
            UploadDataToMemory(memory, createInfo.data, createInfo.size);
        }
    
        if (!createInfo.signalSemaphore.IsInitialized()) {
            createInfo.signalSemaphore = i_Semaphore(i_SemaphoreCreateInfo{});
        }
    
        useCount = std::make_shared<uint32_t>(1);
    }

    void i_DataBuffer::UpdateData(void *data, size_t size, uint32_t threadIndex) {
        if (createInfo.size != size) {
            throw std::runtime_error(
                "Size of data does not match size of buffer, you need to create a new buffer for this");
        }
        if (!createInfo.isDynamic) {
            throw std::runtime_error("This buffer isn't dynamic, so it can't be changed");
        }
        if (!i_Device::DeviceMemoryFree()) {
            createInfo.transferToLocalDeviceMemory = false;
        }
        if (createInfo.transferToLocalDeviceMemory) {
            if (stagingMemory == VK_NULL_HANDLE) {
                AllocateMemory(stagingMemory, stagingBuffer, size,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            }
            UploadDataToMemory(stagingMemory, data, size);
            RecordCopyCommandBuffer(threadIndex, size);
            return;
        } 
        
        UploadDataToMemory(memory, data, size);
        

    }
    VkBuffer i_DataBuffer::GetBuffer() {
        std::lock_guard<std::mutex> lock(bufferCreationMutex);
        if (buffer == VK_NULL_HANDLE) {
            throw std::runtime_error("Tried to return an uninitialized data buffer");
        }

        return buffer;
    }

    i_DataBuffer::i_DataBuffer(const i_DataBuffer &other) {
        if (other.useCount.get() == nullptr) {
            return;
        }

        buffer = other.buffer;
        memory = other.memory;
        stagingBuffer = other.stagingBuffer;
        stagingMemory = other.stagingMemory;
        createInfo = other.createInfo;
        bufferMutex = other.bufferMutex;
        useCount = other.useCount;

        (*useCount.get())++;
    }

    i_DataBuffer &i_DataBuffer::operator=(const i_DataBuffer &other) {
        if (this == &other) {
            return *this;
        }

        if (other.useCount.get() == nullptr) {
            return *this;
        }

        Destruct();

        buffer = other.buffer;
        memory = other.memory;
        stagingBuffer = other.stagingBuffer;
        stagingMemory = other.stagingMemory;
        createInfo = other.createInfo;
        bufferMutex = other.bufferMutex;
        useCount = other.useCount;

        (*useCount.get())++;

        return *this;
    }

    void i_DataBuffer::Destruct() {
        if (useCount.get() == nullptr) {
            return;
        }

        if (*useCount.get() == 1) {
            vkFreeMemory(i_Device::GetDevice(), memory, nullptr);
            vkDestroyBuffer(i_Device::GetDevice(), buffer, nullptr);
            if (stagingBuffer != VK_NULL_HANDLE) {
                vkDestroyBuffer(i_Device::GetDevice(), stagingBuffer, nullptr);
            }
            if (stagingMemory != VK_NULL_HANDLE) {
                vkFreeMemory(i_Device::GetDevice(), stagingMemory, nullptr);
            }

            useCount.reset();
            return;
        }

        (*useCount.get())--;
    }

    i_DataBuffer::~i_DataBuffer() {
        Destruct();
    }

    void i_DataBuffer::CreateBuffer(VkBuffer &buffer, VkBufferUsageFlags usage, VkDeviceSize size) {
        VkBufferCreateInfo bufferInfo{};

        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.usage = usage;
        bufferInfo.size = size;

        if (i_Device::GetQueueFamilyInfo().transferFamilyFound) {
            bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
            bufferInfo.queueFamilyIndexCount = 2;

            uint32_t queueFamilyIndices[] = {
                i_Device::GetQueueFamilyInfo().graphicsQueueCreateInfo.queueFamilyIndex,
                i_Device::GetQueueFamilyInfo().transferQueueCreateInfo.queueFamilyIndex
            };

            bufferInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        if (vkCreateBuffer(i_Device::GetDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create buffer");
        }
    }

    void i_DataBuffer::AllocateMemory(VkDeviceMemory &memory, VkBuffer buffer, size_t size,
                                      VkMemoryPropertyFlags properties) {
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(i_Device::GetDevice(), buffer, &memRequirements);

        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(i_Device::GetPhysicalDevice(), &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((memRequirements.memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties)
                == properties) {
                VkMemoryAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = memRequirements.size;
                allocInfo.memoryTypeIndex = i;

                if (vkAllocateMemory(i_Device::GetDevice(), &allocInfo, nullptr, &memory) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to allocate vertex buffer memory");
                }

                break;
            }
        }

        vkBindBufferMemory(i_Device::GetDevice(), buffer, memory, 0);
    }

    void i_DataBuffer::UploadDataToMemory(VkDeviceMemory memory, void *data, size_t size) {
        void *mappedMem;

        if (vkMapMemory(i_Device::GetDevice(), memory, 0, size, 0, &mappedMem) != VK_SUCCESS) {
            throw std::runtime_error("Failed to map vertex buffer memory, out of RAM?");
        }
        memcpy(mappedMem, data, size);
        vkUnmapMemory(i_Device::GetDevice(), memory);
    }

    i_CommandBuffer i_DataBuffer::RetrieveFreeStagingCommandBuffer(uint32_t threadIndex, i_Semaphore signalSemaphore, uint64_t signalSemaphoreValue,
                                                                   std::weak_ptr<std::mutex> mutex) {
        anyCommandBuffersRecorded = true;

        auto listFront = stagingCommandBuffers.begin();
        std::advance(listFront, threadIndex);

        for (i_DataBufferStagingCommandBuferData &commandBuffer: *listFront) {
            if (commandBuffer.free) {
                commandBuffer.free = false;
                commandBuffer.signalSemaphore = signalSemaphore;
                commandBuffer.mutex = mutex;
                return commandBuffer.commandBuffer;
            }
        }


        i_CommandBufferCreateInfo stagingCommandBufferInfo;
        stagingCommandBufferInfo.type = i_CommandBufferType::DATA;
        stagingCommandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        stagingCommandBufferInfo.flags = COMMAND_POOL_TYPE_TRANSFER | COMMAND_BUFFER_ONE_TIME_SUBMIT_FLAG;
        stagingCommandBufferInfo.threadIndex = threadIndex;


        (*listFront).emplace_back(i_CommandBuffer(stagingCommandBufferInfo), mutex, false, signalSemaphore, signalSemaphoreValue);
        return (*listFront).back().commandBuffer;
    }

    void i_DataBuffer::RecordCopyCommandBuffer(uint32_t threadIndex, size_t size) {
        i_CommandBuffer commandBuffer = RetrieveFreeStagingCommandBuffer(
            createInfo.threadIndex, createInfo.signalSemaphore, createInfo.signalSemaphoreValue, bufferMutex);

        commandBuffer.BeginCommandBuffer(nullptr, false);

        VkBufferCopy copyRegion{};
        copyRegion.size = size;

        vkCmdCopyBuffer(commandBuffer.GetCommandBuffer(), stagingBuffer, buffer, 1, &copyRegion);

        commandBuffer.EndCommandBuffer();

        if (createInfo.isDynamic && size > FREE_STAGING_MEMORY_TRESHOLD) {
            stagingBufferAndMemoryDeleteQueue.push_front(std::make_pair(stagingBuffer, stagingMemory));
            stagingMemory = VK_NULL_HANDLE;
            stagingBuffer = VK_NULL_HANDLE;
        }
    }

    void i_DataBuffer::SubmitCommandBuffers() {
        if (!anyCommandBuffersRecorded) {
            return;
        }

        std::vector<VkSubmitInfo> submitInfos{};

        std::list<VkTimelineSemaphoreSubmitInfo> timelineSubmitInfos{};
        std::list<uint64_t> timelineSignalSemaphoreValues{};


        std::vector<VkCommandBuffer> recordedCommandBuffers{};

        std::vector<i_Fence> fences{};

        std::list<std::pair<VkCommandBuffer, VkSemaphore> > submitInfoReferenceList{};

        std::list<std::pair<uint32_t, uint32_t>> stagingCommandBufferIndexes;


        uint32_t i = 0;
        uint32_t y = 0;
        for (std::vector<i_DataBufferStagingCommandBuferData> &commandBuffers: stagingCommandBuffers) {

            i = 0;
            for (i_DataBufferStagingCommandBuferData &commandBuffer: commandBuffers) {

                if (commandBuffer.free) {
                    continue;
                }

                stagingCommandBufferIndexes.push_back(std::make_pair<>(y, i));
                i++;


                if (!commandBuffer.signalSemaphore.IsInitialized()) {
                    recordedCommandBuffers.push_back(commandBuffer.commandBuffer.GetCommandBuffer());
                    continue;
                }


                submitInfoReferenceList.push_front({
                    commandBuffer.commandBuffer.GetCommandBuffer(),
                    commandBuffer.signalSemaphore.GetSemaphore()
                });

                timelineSignalSemaphoreValues.push_back(commandBuffer.signalSemaphoreValue);

                VkTimelineSemaphoreSubmitInfo timelineSubmitInfo{};

                timelineSubmitInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
                timelineSubmitInfo.pSignalSemaphoreValues = &timelineSignalSemaphoreValues.back();
                timelineSubmitInfo.signalSemaphoreValueCount = 1;

                timelineSubmitInfos.push_back(timelineSubmitInfo);

                VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.pNext = &submitInfos.back();

                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &submitInfoReferenceList.front().first;

                submitInfo.signalSemaphoreCount = 1;
                submitInfo.pSignalSemaphores = &submitInfoReferenceList.front().second;

                submitInfos.push_back(submitInfo);
            }

            y++;
        }

        if (!recordedCommandBuffers.empty()) {
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            submitInfo.commandBufferCount = recordedCommandBuffers.size();
            submitInfo.pCommandBuffers = recordedCommandBuffers.data();

            submitInfos.push_back(submitInfo);
        }

        VkQueue queue = i_Device::GetTransferQueue();

        if (vkQueueSubmit(queue, submitInfos.size(), submitInfos.data(),
                          VK_NULL_HANDLE) != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit data buffer command buffer");
        }

        std::shared_ptr<bool> finished = std::make_shared<bool>(false);
        std::shared_ptr<std::thread> thread = std::make_shared<std::thread>(std::thread(WaitForQueue, queue, stagingCommandBufferIndexes, finished));

        queueWaitThreads.push_back(std::make_pair<>(thread, finished));
    }

    void i_DataBuffer::WaitForQueue(VkQueue queue, std::list<std::pair<uint32_t, uint32_t>> buffersInQueue, std::shared_ptr<bool> finished){
        vkQueueWaitIdle(queue);

        for(std::pair<uint32_t, uint32_t>& i : buffersInQueue){
            if(i.second > stagingCommandBuffers[i.first].size()){
                continue;
            }

            stagingCommandBuffers[i.first][i.second].free = true;
        }

        *finished = true;
    }

    void i_DataBuffer::UpdateCleanup() {
        for (uint32_t i: resetPoolIndexes) {
            i_CommandBuffer::ResetPools(i_CommandBufferType::DATA, i);
        }

        anyCommandBuffersRecorded = false;

        for(uint32_t i = 0; i < queueWaitThreads.size(); i++){
            if(!*queueWaitThreads[i].second){
                continue;
            }

            queueWaitThreads.erase(queueWaitThreads.begin() + i);
            i--;
        }

        for (std::vector<i_DataBufferStagingCommandBuferData> &commandBuffers: stagingCommandBuffers) {
            commandBuffers.resize(TARGET_STAGING_BUFFER_COUNT_PER_THREAD);
        }

        for (std::pair<VkBuffer, VkDeviceMemory> stagingBufferAndMemory: stagingBufferAndMemoryDeleteQueue) {
            vkDestroyBuffer(i_Device::GetDevice(), stagingBufferAndMemory.first, nullptr);
            vkFreeMemory(i_Device::GetDevice(), stagingBufferAndMemory.second, nullptr);
        }
        stagingBufferAndMemoryDeleteQueue.clear();
    }

    void i_DataBuffer::SetSignalSemaphore(i_Semaphore signalSemaphore) {
        createInfo.signalSemaphore = signalSemaphore;
    }

    void i_DataBuffer::SetSignalSemapnoreValue(uint64_t value){
        createInfo.signalSemaphoreValue = value;
    }
}
