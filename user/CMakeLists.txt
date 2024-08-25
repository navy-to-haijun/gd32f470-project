cmake_minimum_required(VERSION 3.22)

project(user)
add_library(user INTERFACE)
# Enable CMake support for ASM and C languages
enable_language(C ASM)

# 宏定义
target_compile_definitions(user INTERFACE 

)
# 头文件
target_include_directories(user INTERFACE
    ${CMAKE_SOURCE_DIR}/user/inc
)
# 递归查找user文件夹下所有C文件
file(GLOB_RECURSE SRC_DIR_LIST ${CMAKE_SOURCE_DIR}/user/src/*.c)

# 源文件
target_sources(user INTERFACE
    ${SRC_DIR_LIST}
)

target_link_directories(user INTERFACE
)

target_link_libraries(user INTERFACE
)

# Validate that user code is compatible with C standard
if(CMAKE_C_STANDARD LESS 11)
    message(ERROR "Generated code requires C11 or higher")
endif()


