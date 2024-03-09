cmake_minimum_required(VERSION 3.10)

project(InsertName)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


add_executable(main "src/main.cpp")

#Windows libs
if(WIN32)

endif (WIN32)


#Linux libs
if (UNIX)
    find_package(glfw3 3.3 REQUIRED)
    target_link_libraries(main glfw3)

endif(UNIX)