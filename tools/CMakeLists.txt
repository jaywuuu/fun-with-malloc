cmake_minimum_required(VERSION 3.10)

# set the project name
project(Tools)
set (CMAKE_CXX_STANDARD 20)

# add exec
add_executable(profiler
    profiler.cpp
)
