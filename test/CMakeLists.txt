cmake_minimum_required(VERSION 3.10)

# set the project name
project(Test)
set (CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(test_memalloc
  test_memalloc.cpp
)

target_include_directories(test_memalloc PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../MemoryAlloc
)

target_link_libraries(
  test_memalloc
  GTest::gtest_main
  MemoryAlloc
)

include(GoogleTest)
gtest_discover_tests(test_memalloc)