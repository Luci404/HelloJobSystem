cmake_minimum_required(VERSION 3.10)

project(HelloJobSystem)

file(GLOB_RECURSE HELLOJOBSYSTEM_DEMO_HEADERS "${CMAKE_SOURCE_DIR}/src/*.h")
file(GLOB_RECURSE HELLOJOBSYSTEM_DEMO_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_executable(HelloJobSystem "${HELLOJOBSYSTEM_DEMO_HEADERS}" "${HELLOJOBSYSTEM_DEMO_SOURCES}")

target_include_directories(HelloJobSystem PUBLIC "${CMAKE_SOURCE_DIR}/src")
