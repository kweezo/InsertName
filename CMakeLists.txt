cmake_minimum_required(VERSION 3.20)

project(InsertName)
if(WIN32)
    include_directories("lib\\win")
    include_directories("C:\\Program Files (x86)\\libpqxx\\include")
    include_directories("C:\\Strawberry\\c\\include")
    include_directories("lib\\win\\g-truc-glm-0af55cc")

    link_directories("lib\\win\\GLFW\\lib-mingw-w64")
    link_directories("lib\\win\\VulkanSDK\\1.3.275.0\\Lib")
    link_directories("C:\\Program Files (x86)\\libpqxx\\lib")
    find_library(SSL_LIBRARY
        NAMES libssl.a
        PATHS C:\\Strawberry\\c\\lib
    )
    find_library(CRYPTO_LIBRARY
        NAMES libcrypto.a
        PATHS C:\\Strawberry\\c\\lib
    )
    find_library(PQ_LIBRARY
        NAMES libpq.lib
        PATHS "C:\\Program Files\\PostgreSQL\\16\\lib"
    )
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

option(BUILD_CLIENT "Build the client executable" OFF)
option(BUILD_INSTALLER "Build the installer executable" OFF)
option(BUILD_SERVER "Build the server executable" OFF)
option(DEBUG "Build with debug flags" OFF)
option(GFX_RECONSTRUCT "Build with graphics reconstruction" OFF)

if(DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -DNDEBUG")
endif()

if(GFX_RECONSTRUCT)
    add_definitions(-DGFX_RECONSTRUCT)
endif()

if(BUILD_INSTALLER)
    file(GLOB installersrc src/installer/**/*.cpp src/settings.cpp )
    add_executable(installer ${installersrc} src/installer/main.cpp)
endif()

if(BUILD_SERVER)
    file(GLOB serversrc src/server/*.cpp)
    add_executable(server ${serversrc} src/server/main.cpp)

    if(WIN32)
        target_link_libraries(server PUBLIC
            pqxx
            ${PQ_LIBRARY}
            ws2_32
            ${SSL_LIBRARY}
            ${CRYPTO_LIBRARY}
        )
    endif()
endif()

if(BUILD_CLIENT)    
    file(GLOB clientsrc src/client/renderer/core/*.cpp src/client/renderer/ext/*.cpp src/client/renderer/window/*.cpp src/client/account/*.cpp)
    add_executable(client ${clientsrc} src/client/main.cpp src/settings.cpp)
    
    if(WIN32)
        target_link_libraries(client PRIVATE
            glfw3
            glfw3dll
            gdi32
            vulkan-1
            ws2_32
            ${SSL_LIBRARY}
            ${CRYPTO_LIBRARY}
        )
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(client PUBLIC glfw)
        target_link_libraries(client PUBLIC assimp)
        target_link_libraries(client PUBLIC vulkan)
        target_link_libraries(client PUBLIC ssl)
        target_link_libraries(client PUBLIC crypto)
    endif()
endif()
set_target_properties(client PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
