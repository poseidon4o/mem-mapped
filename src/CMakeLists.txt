cmake_minimum_required(VERSION 2.8)

project(mem_mapped)

file(GLOB HEADERS "*.h")
file(GLOB SOURCES "*.cpp")

include_directories(.)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})
