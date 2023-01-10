cmake_minimum_required(VERSION 3.22.1)

project(simpleJsonApp)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} httpServer)