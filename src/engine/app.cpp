#include "app.hpp"


namespace renderer{
    std::unique_ptr<App> App::app;

    void App::Create(AppCreateInfo& createInfo){
        if(app != nullptr){
            throw std::runtime_error("ERROR: App already exists");
        }

        app.reset(new App(createInfo));
    }


    App::App(AppCreateInfo& createInfo){
        i_Window::Create(createInfo.windowCreateInfo);
        i_Instance::Create(createInfo.name, createInfo.version);


        scheduler.Initialize();
    }


    bool App::ShouldQuit(){
        AppCreationCheck();

        return glfwWindowShouldClose(i_Window::GetGLFWWindow());
    }


   void App::AppCreationCheck(){
        if(app == nullptr){
            throw std::runtime_error("ERROR: Tried to use the app before creating it");
        }
   }
}