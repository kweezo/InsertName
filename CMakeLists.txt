cmake_minimum_required(VERSION 3.10)

project(InsertName)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

option(BUILD_CLIENT "Build the client executable" ON)
option(BUILD_INSTALLER "Build the installer executable" OFF)
option(BUILD_SERVER "Build the installer executable" OFF)
option(DEBUG "Build with debug flags" OFF)

if(DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -DNDEBUG")
endif()

if(BUILD_INSTALLER)
    file(GLOB installersrc src/installer/**/*.cpp src/settings.cpp)
    add_executable(installer ${installersrc} src/installer/main.cpp)
endif()

if(BUILD_SERVER)
    file(GLOB serversrc src/server/*.cpp src/settings.cpp)
    add_executable(server ${serversrc})
endif()

#Linux libs
if(BUILD_CLIENT)    
    file(GLOB clientsrc src/client/renderer/core/*.cpp src/client/renderer/ext/*.cpp src/client/renderer/window/*.cpp)
    add_executable(client ${clientsrc} src/client/main.cpp src/settings.cpp)
    
    target_link_libraries(client PUBLIC glfw)
    target_link_libraries(client PUBLIC assimp)
    target_link_libraries(client PUBLIC vulkan)
endif()