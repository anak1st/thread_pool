cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(test_thread_pool)

enable_testing()

file(GLOB SRC_FILES
  "${PROJECT_SOURCE_DIR}/src/test.cpp")
add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES})

find_package(fmt CONFIG REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE fmt::fmt)

find_package(GTest CONFIG REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(${CMAKE_PROJECT_NAME})

# copy compile_commands.json to build directory
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${CMAKE_BINARY_DIR}/compile_commands.json
  ${CMAKE_SOURCE_DIR}/build/compile_commands.json)