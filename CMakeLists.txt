cmake_minimum_required(VERSION 3.10)

project(InsertName)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)


add_executable(InsertName src/main.cpp)

#Windows libs
if(WIN32)

endif (WIN32)


#Linux libs
if (UNIX)
    target_link_libraries(InsertName PUBLIC glfw)
    target_link_libraries(InsertName PUBLIC assimp)
    target_link_libraries(InsertName PUBLIC vulkan)
endif(UNIX)