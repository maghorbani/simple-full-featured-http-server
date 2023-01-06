cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_CXX_STANDARD 17)
project(httpServer)
set(target ${PROJECT_NAME})
find_package(Threads REQUIRED)


include_directories(
    ${CMAKE_SOURCE_DIR}/src
    )

file(GLOB src_cpp_files "src/*.cpp")
file(GLOB src_h_files "src/*.h")

add_library(${target} ${src_cpp_files} ${src_h_files})

target_link_libraries(${target} PRIVATE Threads::Threads)

add_subdirectory(examples)