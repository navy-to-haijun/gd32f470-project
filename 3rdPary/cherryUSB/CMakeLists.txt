# cmake_minimum_required(VERSION 3.22)

# project(lession)
# add_library(lession INTERFACE)
# # Enable CMake support for ASM and C languages
# enable_language(C ASM)

# # 宏定义
# target_compile_definitions(lession INTERFACE 

# )
# # 头文件
# target_include_directories( INTERFACE
#     ${CMAKE_SOURCE_DIR}/lession/lession1
# )
# # 递归查找lession文件夹下所有C文件
# file(GLOB_RECURSE SRC_DIR_LIST ${CMAKE_SOURCE_DIR}/lession/*.c)

# # 源文件
# target_sources(lession INTERFACE
#     ${SRC_DIR_LIST}
    
#     # ${CMAKE_SOURCE_DIR}/lession/LED/Led.c
#     # ${CMAKE_SOURCE_DIR}/lession/SDRAM/exmc_sdram.c
# )

# target_link_directories(lession INTERFACE
# )

# target_link_libraries(lession INTERFACE
# )

# # Validate that lession code is compatible with C standard
# if(CMAKE_C_STANDARD LESS 11)
#     message(ERROR "Generated code requires C11 or higher")
# endif()


