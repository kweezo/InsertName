#include "PhysicalDevice.hpp"

namespace renderer{

    void i_PhysicalDevice::Create(){
        device.reset(new i_PhysicalDevice());
    }
    
    void i_PhysicalDevice::Destroy(){
        device.reset();
    }

    VkPhysicalDevice i_PhysicalDevice::GetDevice(){
        return device->physicalDevice;
    }


    i_PhysicalDevice::i_PhysicalDevice(){

    }


    void i_PhysicalDevice::PickDevice(){
        uint32_t deviceCount;
        vkEnumeratePhysicalDevices(i_Instance::GetInstance(), &deviceCount, nullptr);

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(i_Instance::GetInstance(), &deviceCount, devices.data());

        for(VkPhysicalDevice device : devices){
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);

            VkPhysicalDeviceFeatures features;
            vkGetPhysicalDeviceFeatures(device, &features);
        }
    }

}