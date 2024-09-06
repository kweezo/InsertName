#include "app.hpp"


namespace renderer{
    std::unique_ptr<App> App::app;

    void App::Create(AppCreateInfo& createInfo){
        if(app != nullptr){
            throw std::runtime_error("ERROR: App already exists");
        }

        app.reset(new App(createInfo));
    }
    
    void App::Destroy(){
        app.reset();
    }


    App::App(AppCreateInfo& createInfo){
        i_Instance::Create(createInfo.name, createInfo.version);
        i_Window::Create(createInfo.windowCreateInfo);
        i_PhysicalDevice::Create();
        i_LogicalDevice::Create();
        i_Swapchain::Create();
        i_Scheduler::Create();
        ModelManager::Create();


        std::vector<float> whoops = {1.0f};
        i_DataBufferCreateInfo info{};

        info.data = (void*)whoops.data();
        info.size = 4;
        info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        i_DataBuffer buffer = i_DataBuffer(info);

        i_Scheduler::GetScheduler().WaitforAll();
    }

    bool App::ShouldQuit(){
        AppCreationCheck();

        return glfwWindowShouldClose(i_Window::GetGLFWWindow());
    }
    
    void App::Update(){
        glfwPollEvents();
    }

   void App::AppCreationCheck(){
        if(app == nullptr){
            throw std::runtime_error("ERROR: Tried to use the app before creating it");
        }
   }

    App::~App(){
        ModelManager::Destroy();
        i_Scheduler::Destroy();
        i_Swapchain::Destroy();
        i_LogicalDevice::Destroy();
        i_PhysicalDevice::Destroy();
        i_Window::Destroy();
        i_Instance::Destroy();
    }

}