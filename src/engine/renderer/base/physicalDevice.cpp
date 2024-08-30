#include "physicalDevice.hpp"

namespace renderer{


    std::unique_ptr<i_PhysicalDevice> i_PhysicalDevice::device;

    void i_PhysicalDevice::Create(){
        device.reset(new i_PhysicalDevice());
    }
    
    void i_PhysicalDevice::Destroy(){
        device.reset();
    }

    VkPhysicalDevice i_PhysicalDevice::GetDevice(){
        return device->physicalDevice;
    }


    i_PhysicalDevice::i_PhysicalDevice(): physicalDevice(VK_NULL_HANDLE){
        PickDevice();
    }


    void i_PhysicalDevice::PickDevice(){
        uint32_t deviceCount;
        vkEnumeratePhysicalDevices(i_Instance::GetInstance(), &deviceCount, nullptr);

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(i_Instance::GetInstance(), &deviceCount, devices.data());


        VkPhysicalDevice fallbackDevice = VK_NULL_HANDLE;


        for(VkPhysicalDevice device : devices){
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);

            VkPhysicalDeviceFeatures features;
            vkGetPhysicalDeviceFeatures(device, &features);


            uint32_t queueFamilyPropertyCount;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyPropertyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyPropertyCount, queueFamilyProperties.data());




            bool graphicsQueueFamilyFound = false;

            for(const VkQueueFamilyProperties& properties : queueFamilyProperties){
                if(properties.queueFlags | VK_QUEUE_GRAPHICS_BIT){
                    graphicsQueueFamilyFound = true;
                    break;
                }
            }


           if(properties.apiVersion < VK_VERSION_1_3 || !graphicsQueueFamilyFound){
                continue;
           } 

            if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
                this->physicalDevice = device;
            } else if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU){
                fallbackDevice = device;
            }
        }

        if(physicalDevice == VK_NULL_HANDLE){
            physicalDevice = fallbackDevice;
        }

        if(physicalDevice == VK_NULL_HANDLE){
            throw std::runtime_error("ERROR: No suitable GPU found");
        }
    }
    
}
