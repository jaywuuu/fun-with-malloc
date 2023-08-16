cmake_minimum_required(VERSION 3.10)

# set the project name
project(MemoryAlloc)
set (CMAKE_CXX_STANDARD 20)

# add lib
add_library(MemoryAlloc
    MemoryAlloc.cpp
)
