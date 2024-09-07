#include "logicalDevice.hpp"

const std::string deviceConfigPath = "engine_data/config/device.json";

namespace renderer{
    std::unique_ptr<i_LogicalDevice> i_LogicalDevice::device;
    
    void i_LogicalDevice::Create(){
        device.reset(new i_LogicalDevice());
    }

    void i_LogicalDevice::Destroy(){
        device.reset();
    }


    i_LogicalDevice::i_LogicalDevice(){
        LoadConfig();
        CreateQueueCreateInfos();
        CreateDevice();
        RetrieveQueues();
        CreateVulkanAllocator();
    }


    void i_LogicalDevice::LoadConfig(){
        std::ifstream stream(deviceConfigPath);
        
        Json::Value root;
        Json::CharReaderBuilder builder;
        std::string error;

        if(!Json::parseFromStream(builder, stream, &root, &error)){
            throw std::runtime_error("ERROR: Failed to parse shaders.json: " + error);
        }


        const Json::Value& requiredExtensions = root["extensions_required"];

        for(const Json::Value& extension : requiredExtensions){
            const char* buff = extension.asCString();
            this->extensions.push_back(new char[strlen(buff)+1]);

            strcpy(this->extensions.back(), buff);
        }
        
        
        std::list<uint32_t> unsupportedExtensionIndexes = CheckExtensionSupport(extensions);
        if(!unsupportedExtensionIndexes.empty()){
            std::cerr << "ERROR: Not all required extensions are supported: \n";
            for(uint32_t i : unsupportedExtensionIndexes){
                std::cerr << "\t" << extensions[i] << "\n"; 
            }
            throw std::runtime_error("Program is terminating");
        }



        const Json::Value& optionalExtensions = root["extensions_optional"];

        for(const Json::Value& extension : optionalExtensions){
            const char* buff = extension.asCString();
            this->supportedOptionalExtensions.push_back(new char[strlen(buff)+1]);

            strcpy(this->supportedOptionalExtensions.back(), buff);
        }

        unsupportedExtensionIndexes = CheckExtensionSupport(supportedOptionalExtensions);
        if(!unsupportedExtensionIndexes.empty()){
            uint32_t erasedExtCount = 0;
            std::cout << "INFO: Not all optional extensions are supported: \n";
            for(uint32_t i : unsupportedExtensionIndexes){
                std::cout << "\t" << supportedOptionalExtensions[i-erasedExtCount] << "\n"; 
                supportedOptionalExtensions.erase(supportedOptionalExtensions.begin() + i - erasedExtCount);

                erasedExtCount++;
            }
        } 

        extensions.insert(extensions.begin(), supportedOptionalExtensions.begin(), supportedOptionalExtensions.end());

        stream.close();
    }

    void i_LogicalDevice::CreateDevice(){

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = queueCreateInfos.size();

        createInfo.enabledExtensionCount = extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();
        

        if(vkCreateDevice(i_PhysicalDevice::GetDevice(), &createInfo, nullptr, &logicalDevice) != VK_SUCCESS){
            throw std::runtime_error("ERROR: Failed to create a logical device");
        }

        queueBatches.clear();
        queuePriorities.clear();
    }


    void i_LogicalDevice::RetrieveQueues(){
        uint32_t y = 0;
        for(VkDeviceQueueCreateInfo& createInfo : queueCreateInfos){
            for(uint32_t i = 0; i < createInfo.queueCount; i++){
                queueBatches[y].queues.push_back(0);
                vkGetDeviceQueue(logicalDevice, createInfo.queueFamilyIndex, i, &queueBatches[y].queues.back());
            }
            y++;
        }
    }

        
    void i_LogicalDevice::CreateQueueCreateInfos(){
        uint32_t queueFamilyPropertyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(i_PhysicalDevice::GetDevice(), &queueFamilyPropertyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(i_PhysicalDevice::GetDevice(), &queueFamilyPropertyCount, queueFamilyProperties.data());

        queueCreateInfos.reserve(queueFamilyPropertyCount);

        uint32_t i = 0;
        for(const VkQueueFamilyProperties& properties : queueFamilyProperties){
            if(!(properties.queueFlags & VK_QUEUE_GRAPHICS_BIT || properties.queueFlags & VK_QUEUE_COMPUTE_BIT || properties.queueFlags &
            VK_QUEUE_TRANSFER_BIT)){
                i++;
                continue;//in case there are some weird ass encoding queues, too lazy to check if thats possible
            }

            queuePriorities.emplace_back(properties.queueCount, 1.0f);

            VkDeviceQueueCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            createInfo.queueCount = properties.queueCount;
            createInfo.queueFamilyIndex = i;
            createInfo.pQueuePriorities = queuePriorities.back().data();

            queueCreateInfos.push_back(createInfo);


            i_QueueBatch queueBatch{};
            queueBatch.flags = createInfo.flags;
            queueBatch.queueFamilyIndex = i;

            queueBatches.push_back(queueBatch);


            i++;
        }
    }

    void i_LogicalDevice::CreateVulkanAllocator(){
        const std::vector<std::pair<std::string, VmaAllocatorCreateFlagBits>> optionalExtensions = {
            {"VK_KHR_dedicated_allocation", VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT},
            {"VK_KHR_bind_memory2", VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT},
            {"VK_KHR_maintenance4", VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT},
            {"VK_KHR_maintenance5", VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT},
            {"VK_EXT_memory_budget", VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT},
            {"VK_KHR_buffer_device_address", VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT},
            {"VK_EXT_memory_priority", VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT},
            {"VK_AMD_device_coherent_memory", VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT},
            //{"VK_KHR_external_memory_win32", VMA_ALLOCATOR_CREATE_KHR_EXTERNAL_MEMORY_WIN32_BIT}//WHY NO WORKY???'
        };

        VmaVulkanFunctions vulkanFunctions = {};
        vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
        vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;
        VmaAllocatorCreateFlags flags{};

        for(std::pair<std::string, VmaAllocatorCreateFlagBits> extension : optionalExtensions){
            bool match = false;
            for(char* supportedExtension : supportedOptionalExtensions){
                if(strcmp(extension.first.c_str(), supportedExtension) == 0){
                    match = true;
                    break;
                }
           } 

            if(match){
                flags |= extension.second;
            }
        }

        VmaAllocatorCreateInfo createInfo{};
        createInfo.flags = flags;
        createInfo.vulkanApiVersion = VK_API_VERSION_1_3;
        createInfo.physicalDevice = i_PhysicalDevice::GetDevice();
        createInfo.device = logicalDevice;
        createInfo.instance = i_Instance::GetInstance();
        createInfo.pVulkanFunctions = &vulkanFunctions;

        if(vmaCreateAllocator(&createInfo, &vmaAllocator) != VK_SUCCESS){
            throw std::runtime_error("Failed to create the VMA allocator");
        }
    }
    
    VkDevice i_LogicalDevice::GetDevice(){
        return device->logicalDevice;
    }

    VmaAllocator i_LogicalDevice::GetAllocator(){
        return device->vmaAllocator;
    }

    std::vector<uint32_t> i_LogicalDevice::GetQueueFamilyIndices(VkQueueFlags flags){//TODO fix
        VkQueueFlags remainingFlags = flags;
        std::vector<uint32_t> indices;

        //unceremoniously written by chatgpt
        for (uint32_t i = 0; i < device->queueBatches.size() && remainingFlags != 0; i++) {
            VkQueueFlags queueFlags = device->queueBatches[i].flags;

            VkQueueFlags intersection = remainingFlags & queueFlags;
            if (intersection != 0 && intersection != queueFlags) {
                indices.push_back(i);
                remainingFlags &= ~queueFlags;    
            }
        }

        for (uint32_t i = 0; i < device->queueBatches.size() && remainingFlags != 0; i++) {
            VkQueueFlags queueFlags = device->queueBatches[i].flags;

            if (queueFlags & remainingFlags) {
                indices.push_back(i);
                remainingFlags &= ~queueFlags;     
            }
        }

        if (remainingFlags != 0) {
            throw std::runtime_error("Failed to find a suitable queue family(bruh)");
        }

        return indices;
    }

    std::list<uint32_t> i_LogicalDevice::CheckExtensionSupport(const std::vector<char*>& extensions){
        std::list<uint32_t> unsupportedExtensionIndexes;

        uint32_t propertyCount;
        vkEnumerateDeviceExtensionProperties(i_PhysicalDevice::GetDevice(), nullptr, &propertyCount, nullptr);

        std::vector<VkExtensionProperties> properties(propertyCount);
        vkEnumerateDeviceExtensionProperties(i_PhysicalDevice::GetDevice(), nullptr, &propertyCount, properties.data());


        for(uint32_t i = 0; i < extensions.size(); i++){
            bool match = false;
            for(uint32_t y = 0; y < properties.size(); y++){
                if(strcmp(extensions[i], properties[y].extensionName) == 0){
                    match = true;
                    break;
                }
            }

            if(match){
                continue;
            }

            unsupportedExtensionIndexes.push_back(i);
        }
        
        return unsupportedExtensionIndexes;
    }

    i_LogicalDevice::~i_LogicalDevice(){
        vmaDestroyAllocator(vmaAllocator);
        vkDestroyDevice(logicalDevice, nullptr);
    }
}