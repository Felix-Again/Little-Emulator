cmake_minimum_required(VERSION 3.10)

project(DMG-Emulator VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED)

set(SOURCES
    src/main.cpp
    src/core/cpu.cpp
    src/core/memorybus.cpp
    src/core/gpu.cpp
    src/core/cyclecounter.cpp
    src/core/systembus.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE src/core)

if(WIN32)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wpedantic -Wextra -Werror)
endif()


if (TARGET SDL3::SDL3-shared)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3-shared)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
endif()