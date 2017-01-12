cmake_minimum_required(VERSION 3.6)

project(tst CXX)

set(CMAKE_CXX_STANDARD 14)

enable_testing()

add_subdirectory(include)
add_subdirectory(external)
add_subdirectory(test)
