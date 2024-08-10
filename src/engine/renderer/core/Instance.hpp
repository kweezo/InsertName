#pragma once

#include <vector>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <fstream>

#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

#ifdef NDEBUG
/*#include <NVIDIA_Nsight_Aftermath/GFSDK_Aftermath.h>
#include <NVIDIA_Nsight_Aftermath/GFSDK_Aftermath_GpuCrashDump.h>
#include <NVIDIA_Nsight_Aftermath/GFSDK_Aftermath_Defines.h>*/
#endif

#include "../window/Window.hpp"

#define GPU_CRASH_LOG_FILE "./client_data/log/gpu_dump.nv-gpudmp"

namespace renderer {
    class i_Instance {
    public:
        static void Init();

        static void Cleanup();

        static VkInstance GetInstance();

    private:
        static void SetupDebugMessenger();

        static VkInstance instance;
        static VkDebugUtilsMessengerEXT debugMessenger;
    };
}
