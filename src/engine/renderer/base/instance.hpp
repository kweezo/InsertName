#pragma once

#include <string>
#include <exception>
#include <memory>
#include <vector>
#include <fstream>
#include <iostream>
#include <list>

#include <jsoncpp/json/json.h>

#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>


namespace renderer{
    class i_Instance{
        public:
            static void Create(std::string appName, uint32_t version);
            static void Destroy();

            static VkInstance GetInstance();

            ~i_Instance();
        private:
            i_Instance(std::string appName, uint32_t version);
            i_Instance& operator=(const i_Instance& other) = delete;
            i_Instance(const i_Instance& other) = delete;


            static std::unique_ptr<i_Instance> instance;

            void GetRequiredExtensions();
            void LoadConfig();
            void CreateVulkanInstance(std::string appName, uint32_t version);
            void SetupDebugMessenger();

            std::list<uint32_t> CheckExtensionSupport(const std::vector<char*>& extensions);

            VkInstance vulkanInstance;

            uint32_t glfwExtensionCount;
            std::vector<char*> extensions;
            std::vector<char*> supportedOptionalExtensions;

            std::vector<char*> layers;

            VkDebugUtilsMessengerEXT debugMessenger;
    };
}
