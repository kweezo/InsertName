#pragma once

#include <string>
#include <exception>
#include <memory>
#include <vector>
#include <fstream>

#include <jsoncpp/json/json.h>

#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

namespace renderer{
    class i_Instance{
        public:
            static void Create(std::string appName, uint32_t version);
            static void Destroy();

            ~i_Instance();
        private:
            i_Instance(std::string appName, uint32_t version);
            i_Instance& operator=(const i_Instance& other) = delete;
            i_Instance(const i_Instance& other) = delete;


            static std::unique_ptr<i_Instance> instance;

            void GetRequiredExtensions();
            void LoadConfig();
            void CreateVulkanInstance(std::string appName, uint32_t version);

            VkInstance vulkanInstance;

            std::vector<const char*> extensions;
            std::vector<const char*> layers;
    };
}
