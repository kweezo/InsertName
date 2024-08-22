#pragma once

#include <memory>
#include <string>
#include <mutex>
#include <exception>

#include <enkiTS/TaskScheduler.h>

#include <vulkan/vulkan.h>

#include "renderer/base/window.hpp"
#include "renderer/base/instance.hpp"

#define APP_VERSION(major, minor, patch) VK_MAKE_VERSION(major, minor, patch)

namespace renderer{


    struct AppCreateInfo{
        std::string name;
        uint32_t version;

        WindowCreateInfo windowCreateInfo;
    };

    class App{
    public:
        static void Create(AppCreateInfo& createInfo);
        static bool ShouldQuit();

    private:
        App(AppCreateInfo& createInfo);
        App(const App& other) = delete;
        App& operator=(const App& other) = delete;


        static void AppCreationCheck();

        static std::unique_ptr<App> app;


        enki::TaskScheduler scheduler;

    };


};