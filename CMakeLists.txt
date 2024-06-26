cmake_minimum_required(VERSION 3.20)

project(InsertName)

include_directories("src")

if(WIN32)
    include_directories("lib\\win")
    include_directories("lib\\win\\g-truc-glm-0af55cc")
    include_directories("lib\\win\\PDCurses-3.9")
    include_directories("lib\\win\\libpqxx-master\\build\\libpqxx\\include")
    include_directories("lib\\win\\c\\include") # openssl
    include_directories("lib\\win\\assimp-master\\build\\Assimp\\include")
    include_directories("lib\\win\\jsoncpp-master\\build\\jsoncpp\\include")

    link_directories("lib\\win\\GLFW\\lib-mingw-w64")
    link_directories("lib\\win\\VulkanSDK\\1.3.275.0\\Lib")
    link_directories("lib\\win\\PDCurses-3.9\\wincon")
    link_directories("lib\\win\\libpqxx-master\\build\\libpqxx\\lib")
    link_directories("lib\\win\\assimp-master\\build\\Assimp\\lib")
    link_directories("lib\\win\\jsoncpp-master\\build\\jsoncpp\\lib")
    find_library(SSL_LIBRARY
        NAMES libssl.a
        PATHS lib\\win\\c\\lib
    )
    find_library(CRYPTO_LIBRARY
        NAMES libcrypto.a
        PATHS lib\\win\\c\\lib
    )
    find_library(PQ_LIBRARY
        NAMES libpq.a
        PATHS lib\\win\\c\\lib
    )
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY lib)

option(BUILD_CLIENT "Build the client executable" OFF)
option(BUILD_INSTALLER "Build the installer executable" OFF)
option(BUILD_SERVER "Build the server executable" OFF)
option(BUILD_ENGINE "Build the engine library" OFF)
option(DEBUG "Build with debug flags" OFF)
option(RELEASE "Build with release flags" OFF)
option(GFX_RECONSTRUCT "Build with graphics reconstruction" OFF)

if(DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -DNDEBUG ")
endif()

if(RELEASE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

if(GFX_RECONSTRUCT)
    add_definitions(-DGFX_RECONSTRUCT)
endif()

if(BUILD_INSTALLER)
    file(GLOB installersrc src/installer/**/*.cpp)
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
            -l:pdcurses.a
        )
    else()
        target_link_libraries(server PUBLIC pqxx)
        target_link_libraries(server PUBLIC ssl)
        target_link_libraries(server PUBLIC crypto)
        target_link_libraries(server PUBLIC SDL2)
        target_link_libraries(server PUBLIC ncurses)
    endif()
endif()

if(BUILD_ENGINE)
    file(GLOB enginesrc src/engine/renderer/core/*.cpp src/engine/renderer/ext/*.cpp
     src/engine/renderer/window/*.cpp src/client/account/Settings.cpp src/engine/renderer/ext/**/*.cpp)
    add_library(engine SHARED ${enginesrc})

    if(WIN32)
        target_link_libraries(engine PUBLIC
            glfw3
            glfw3dll
            assimp
            gdi32
            vulkan-1
            jsoncpp
        )
    else()
        target_link_libraries(engine PUBLIC glfw)
        target_link_libraries(engine PUBLIC assimp)
        target_link_libraries(engine PUBLIC vulkan)
    endif()
endif()

if(BUILD_CLIENT)    
    file(GLOB clientsrc src/client/account/*.cpp)
    add_executable(client ${clientsrc} src/client/main.cpp)

    link_directories(${CMAKE_SOURCE_DIR}/lib)
    include_directories(${CMAKE_SOURCE_DIR}/src)
    
    if(WIN32)
        target_link_libraries(client PUBLIC
            ws2_32
            ${SSL_LIBRARY}
            ${CRYPTO_LIBRARY}
            engine
        )
    else()
        target_link_libraries(client PUBLIC ssl)
        target_link_libraries(client PUBLIC crypto)
        target_link_libraries(client PUBLIC engine)
        target_link_libraries(client PUBLIC jsoncpp)
    endif()
endif()
