cmake_minimum_required(VERSION 3.2)
project(termux-auth VERSION 1.3)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
include_directories(${PROJECT_SOURCE_DIR})

find_library(LIB_CRYPTO crypto)

add_executable(plogin plogin.c)
target_link_libraries(plogin termux-auth)
