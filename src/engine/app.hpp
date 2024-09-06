#pragma once

#include <memory>
#include <string>
#include <mutex>
#include <exception>
#include <functional>

#include <vulkan/vulkan.h>

#include "renderer/base/window.hpp"
#include "renderer/base/instance.hpp"
#include "renderer/base/physicalDevice.hpp"
#include "renderer/base/logicalDevice.hpp"
#include "renderer/base/swapchain.hpp"
#include "renderer/base/scheduler.hpp"
#include "renderer/ui/modelManager.hpp"
#include "renderer/util/mem/dataBuffer.hpp"


#define APP_VERSION(major, minor, patch) VK_MAKE_VERSION(major, minor, patch)

namespace renderer{


    struct AppCreateInfo{
        std::string name;
        uint32_t version;

        WindowCreateInfo windowCreateInfo;
    };

    enum UpdateInvokeTime{
        START,
        EARLY,
        MID,
        LATE,
        END
    };

    class App{
    public:
        static void Create(AppCreateInfo& createInfo);
        static bool ShouldQuit();
        static void Update();
        static void Destroy();

//        static void AddUpdateFunc(std::function<void(void)> func, UpdateInvokeTime invokeTime); needed?

        ~App();
    private:
        App(AppCreateInfo& createInfo);
        App(const App& other) = delete;
        App& operator=(const App& other) = delete;


        static void AppCreationCheck();

        static std::unique_ptr<App> app;
    };


};