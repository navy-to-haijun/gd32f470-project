cmake_minimum_required(VERSION 3.22)

# 导出 compile_commands.json，方便 VSCode 自动解析头文件
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
# 强制输出 ANSI 颜色
add_compile_options(-fdiagnostics-color=always)

# 编译模式：Debug 或 Release
set(CMAKE_BUILD_TYPE "Debug")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# 设置 common 路径
get_filename_component(COMMON_DIR "${CMAKE_SOURCE_DIR}/../../common" REALPATH)
message(STATUS "COMMON_DIR = ${COMMON_DIR}")

# 包含交叉编译链
include("${COMMON_DIR}/gcc-arm-none-eabi.cmake")


# 可执行文件名
set(EXECUTABLE_NAME "led_printf_demo")


# 工程名
project(led_printf_demo C  ASM)



# 添加子目录
add_subdirectory(${COMMON_DIR} ${CMAKE_BINARY_DIR}/common)

# 获取common的源文件列表（假设common中设置了INTERFACE_SOURCES属性）
get_target_property(COMMON_SOURCES common_sources INTERFACE_SOURCES)
if(NOT COMMON_SOURCES)
    message(FATAL_ERROR  "common_sources INTERFACE_SOURCES property not found, using fallback")
endif()

# 主项目的源文件
file(GLOB_RECURSE APP_SOURCES
    ${CMAKE_SOURCE_DIR}/../source/*.c
)

# list(FILTER ALL_SOURCES EXCLUDE REGEX "CMakeFiles")
# list(FILTER ALL_SOURCES EXCLUDE REGEX "CompilerId")

# 创建可执行文件（包含所有源文件）
add_executable(${EXECUTABLE_NAME}
    ${APP_SOURCES}
    ${COMMON_SOURCES}  # 添加common的源文件
)

# 添加头文件
target_include_directories(${EXECUTABLE_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/../include
)

# link common接口库（获取头文件目录和宏定义）
target_link_libraries(${EXECUTABLE_NAME} PRIVATE COMMON)

# 额外的宏定义（如果需要）
target_compile_definitions(${EXECUTABLE_NAME} PRIVATE
    # 这里可以添加app特有的宏定义
)

# 添加编译选项
target_compile_options(${EXECUTABLE_NAME} PRIVATE
    -Wno-unused-parameter    # 屏蔽未使用参数警告
    -Wno-return-type         # 屏蔽返回值警告
)

# 设置输出目录
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/../output)
file(MAKE_DIRECTORY ${OUTPUT_DIR})
# 添加转换bin 文件
add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${EXECUTABLE_NAME}> ${OUTPUT_DIR}/${EXECUTABLE_NAME}.bin
    COMMENT "Converting elf to bin: $<TARGET_FILE:${EXECUTABLE_NAME}> -> ${OUTPUT_DIR}/${EXECUTABLE_NAME}.bin"
)

