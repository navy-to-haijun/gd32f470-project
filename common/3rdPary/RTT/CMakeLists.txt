cmake_minimum_required(VERSION 3.22)

project(RTT)
add_library(RTT INTERFACE)
# Enable CMake support for ASM and C languages
enable_language(C ASM)

# 宏定义
target_compile_definitions(RTT INTERFACE 

)
# 头文件
target_include_directories(RTT INTERFACE
    ${CMAKE_SOURCE_DIR}/3rdPary/RTT
)
# 递归查找RTT文件夹下所有C文件
file(GLOB_RECURSE SRC_DIR_LIST ${CMAKE_SOURCE_DIR}/3rdPary/RTT/*.c)
# 源文件
target_sources(RTT INTERFACE
    ${SRC_DIR_LIST}
    ${CMAKE_SOURCE_DIR}/3rdPary/RTT/SEGGER_RTT_ASM_ARMv7M.S
)

target_link_directories(RTT INTERFACE
)

target_link_libraries(RTT INTERFACE
)

# Validate that RTT code is compatible with C standard
if(CMAKE_C_STANDARD LESS 11)
    message(ERROR "Generated code requires C11 or higher")
endif()


