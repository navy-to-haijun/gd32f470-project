cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# 工程名字
set(CMAKE_PROJECT_NAME gd32f470BaseProject)

# Include toolchain file
include("gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# 添加子目录
add_subdirectory(Firmware)
add_subdirectory(user)
add_subdirectory(Hardware)
add_subdirectory(3rdPary/RTT)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE

    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    gd32f470HAL
    user
    hardware
    RTT

    # Add user defined libraries
)
# 自定义清除build文件命令
add_custom_target( clc
    COMMAND echo " clean ${CMAKE_BINARY_DIR} file!"
    COMMAND rm -rf ${CMAKE_BINARY_DIR}/*
)

# 自定义下载程序命令
add_custom_target( d 
    COMMAND echo "download ${CMAKE_PROJECT_NAME}.elf by pyocd!"
    # COMMAND pyocd load ${CMAKE_PROJECT_NAME}.elf --target gd32f470zg --pack ${CMAKE_SOURCE_DIR}/pyocd/GigaDevice.GD32F4xx_DFP.3.2.0.pack
    COMMAND pyocd load ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.elf
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/pyocd
)

# 自定义打印指令
add_custom_target( p 
    COMMAND echo "USe RTT by pyocd!"
    COMMAND pyocd rtt 
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/pyocd
)


