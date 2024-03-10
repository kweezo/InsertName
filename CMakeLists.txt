cmake_minimum_required(VERSION 3.10)

project(InsertName)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "-Wall -g")
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

file(GLOB clientsrc src/client/**/*.cpp)
add_executable(client ${clientsrc} src/client/main.cpp)

#Windows libs
if(WIN32)

endif (WIN32)


#Linux libs
if (UNIX)
    target_link_libraries(client PUBLIC glfw)
    target_link_libraries(client PUBLIC assimp)
    target_link_libraries(client PUBLIC vulkan)
endif(UNIX)