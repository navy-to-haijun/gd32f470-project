cmake_minimum_required(VERSION 3.22)

project(hardware)
add_library(hardware INTERFACE)
# Enable CMake support for ASM and C languages
enable_language(C ASM)

# 宏定义
target_compile_definitions(hardware INTERFACE 

)
# 头文件
target_include_directories(hardware INTERFACE
    ${CMAKE_SOURCE_DIR}/Hardware/LED
    # ${CMAKE_SOURCE_DIR}/Hardware/SDRAM
    # ${CMAKE_SOURCE_DIR}/Hardware/RGB
)
# 递归查找Hardware文件夹下所有C文件
file(GLOB_RECURSE SRC_DIR_LIST 
    ${CMAKE_SOURCE_DIR}/Hardware/LED/*.c
    ${CMAKE_SOURCE_DIR}/Hardware/USB/*.c
)


# 源文件
target_sources(hardware INTERFACE
    ${SRC_DIR_LIST}
    
    # ${CMAKE_SOURCE_DIR}/Hardware/LED/Led.c
    # ${CMAKE_SOURCE_DIR}/Hardware/SDRAM/exmc_sdram.c
)

target_link_directories(hardware INTERFACE
)

target_link_libraries(hardware INTERFACE
)

# Validate that hardware code is compatible with C standard
if(CMAKE_C_STANDARD LESS 11)
    message(ERROR "Generated code requires C11 or higher")
endif()


