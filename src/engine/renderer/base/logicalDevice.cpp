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


        const Json::Value& extensions = root["extensions"];

        for(const Json::Value& extension : extensions){
            const char* buff = extension.asCString();
            this->extensions.push_back(new char[strlen(buff)+1]);

            strcpy(this->extensions.back(), buff);
        }

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

        const std::vector<char*> supportedOptionalExtensions = i_Instance::GetSupportedOptionalExtensions();

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

    i_LogicalDevice::~i_LogicalDevice(){
        vmaDestroyAllocator(vmaAllocator);
        vkDestroyDevice(logicalDevice, nullptr);
    }
}