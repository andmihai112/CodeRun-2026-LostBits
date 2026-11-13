cmake_minimum_required(VERSION 3.20)
project(SDL3_Project_SelfContained)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(glm)

add_executable(SDL3_Project_SelfContained
        main.cpp
        circle.cpp
        circle.h
        rectangle.cpp
        rectangle.h
)

target_include_directories(SDL3_Project_SelfContained PRIVATE SDL3/include)

target_link_directories(SDL3_Project_SelfContained PRIVATE SDL3/lib)

target_link_libraries(SDL3_Project_SelfContained PRIVATE
        SDL3
        glm::glm
)