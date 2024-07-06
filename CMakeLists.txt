cmake_minimum_required(VERSION 3.20)

project(InsertName)

include_directories("src")

if(WIN32)
    set(BOOST_LIB_SUFFIX "-mgw13-mt-x64-1_85.a")

    include_directories("lib\\win\\include")

    link_directories(lib\\win\\github_repos\\GLFW\\lib-mingw-w64)
    link_directories(lib\\win\\github_repos\\VulkanSDK\\1.3.275.0\\Lib)
    link_directories(lib\\win\\github_repos\\PDCurses-3.9\\wincon)
    link_directories(lib\\win\\github_repos\\libpqxx-master\\build\\libpqxx\\lib)
    link_directories(lib\\win\\github_repos\\assimp-master\\build\\Assimp\\lib)
    link_directories(lib\\win\\github_repos\\jsoncpp-master\\build\\jsoncpp\\lib)

    find_library(BOOST_CONTAINER
        NAMES libboost_container${BOOST_LIB_SUFFIX}
        PATHS lib\\win\\github_repos\\boost_1_85_0\\stage\\lib
    )
    find_library(SSL_LIBRARY
        NAMES libssl.a
        PATHS lib\\win\\github_repos\\c\\lib
    )
    find_library(CRYPTO_LIBRARY
        NAMES libcrypto.a
        PATHS lib\\win\\github_repos\\c\\lib
    )
    find_library(PQ_LIBRARY
        NAMES libpq.a
        PATHS lib\\win\\github_repos\\c\\lib
    )
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY lib)

option(BUILD_SERVER "Build the server executable" OFF)
option(BUILD_SERVER_AUTH "Build the server authentication executable" OFF)
option(BUILD_SERVER_WORLD "Build the server world executable" OFF)
option(BUILD_SERVER_CHAT "Build the server chat executable" OFF)
option(BUILD_SERVER_VOICE "Build the server voice executable" OFF)
option(BUILD_SERVER_CONTROL "Build the server control executable" OFF)

option(BUILD_CLIENT "Build the client executable" OFF)
option(BUILD_INSTALLER "Build the installer executable" OFF)
option(BUILD_ENGINE "Build the engine library" OFF)
option(DEBUG "Build with debug flags" OFF)
option(RELEASE "Build with release flags" OFF)
option(GFX_RECONSTRUCT "Build with graphics reconstruction" OFF)

if(BUILD_SERVER)
    set(BUILD_SERVER_AUTH ON)
    set(BUILD_SERVER_WORLD ON)
    set(BUILD_SERVER_CHAT ON)
    set(BUILD_SERVER_VOICE ON)
    set(BUILD_SERVER_CONTROL ON)
endif()

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

if(BUILD_SERVER_AUTH)
    file(GLOB serverauthsrc src/server/auth/*.cpp)
    add_executable(serverauth ${serverauthsrc} src/server/auth/main.cpp)

    if(WIN32)
        target_link_libraries(serverauth PUBLIC

        )
    else()
        target_link_libraries(serverauth PUBLIC
        
        )
    endif()
endif()

if(BUILD_SERVER_WORLD)
    file(GLOB serverworldsrc src/server/world/*.cpp)
    add_executable(serverworld ${serverworldsrc} src/server/world/main.cpp)

    if(WIN32)
        target_link_libraries(serverworld PUBLIC
            
        )
    else()
        target_link_libraries(serverworld PUBLIC

        )
    endif()
endif()

if(BUILD_SERVER_CHAT)
    file(GLOB serverchatsrc src/server/chat/*.cpp)
    add_executable(serverchat ${serverchatsrc} src/server/chat/main.cpp)

    if(WIN32)
        target_link_libraries(serverchat PUBLIC
            
        )
    else()
        target_link_libraries(serverchat PUBLIC
        
        )
    endif()
endif()

if(BUILD_SERVER_VOICE)
    file(GLOB servervoicesrc src/server/voice/*.cpp)
    add_executable(servervoice ${servervoicesrc} src/server/voice/main.cpp)

    if(WIN32)
        target_link_libraries(servervoice PUBLIC
            
        )
    else()
        target_link_libraries(servervoice PUBLIC

        )
    endif()
endif()

if(BUILD_SERVER_CONTROL)
    file(GLOB servercontrolsrc src/server/control/*.cpp)
    add_executable(servercontrol ${servercontrolsrc} src/server/control/main.cpp)

    if(WIN32)
        target_link_libraries(servercontrol PUBLIC
            
        )
    else()
        target_link_libraries(servercontrol PUBLIC

        )
    endif()
endif()

# if(BUILD_SERVER)
#     file(GLOB serversrc src/server/*.cpp)
#     add_executable(server ${serversrc} src/server/main.cpp)

#     if(WIN32)
#         target_link_libraries(server PUBLIC
#             pqxx
#             ${PQ_LIBRARY}
#             ws2_32
#             ${SSL_LIBRARY}
#             ${CRYPTO_LIBRARY}
#             -l:pdcurses.a
#         )
#     else()
#         target_link_libraries(server PUBLIC pqxx)
#         target_link_libraries(server PUBLIC ssl)
#         target_link_libraries(server PUBLIC crypto)
#         target_link_libraries(server PUBLIC SDL2)
#         target_link_libraries(server PUBLIC ncurses)
#     endif()
# endif()

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
            ${BOOST_CONTAINER}
        )
    else()
        target_link_libraries(engine PUBLIC glfw)
        target_link_libraries(engine PUBLIC assimp)
        target_link_libraries(engine PUBLIC vulkan)
        target_link_libraries(engine PUBLIC boost_container)
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
