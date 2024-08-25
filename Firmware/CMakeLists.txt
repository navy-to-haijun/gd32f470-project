cmake_minimum_required(VERSION 3.22)

project(gd32f470HAL)
add_library(gd32f470HAL INTERFACE)
# Enable CMake support for ASM and C languages
enable_language(C ASM)

target_compile_definitions(gd32f470HAL INTERFACE 
    USE_STDPERIPH_DRIVER
    GD32F470
    $<$<CONFIG:Debug>:DEBUG>
)
# 头文件
target_include_directories(gd32f470HAL INTERFACE
    ${CMAKE_SOURCE_DIR}/Firmware/GD32F4xx_standard_peripheral/Include
    ${CMAKE_SOURCE_DIR}/Firmware/CMSIS
    ${CMAKE_SOURCE_DIR}/Firmware/CMSIS/GD/GD32F4xx/Include
    ${CMAKE_SOURCE_DIR}/Firmware/GD32F4xx_usb_library/device/class/cdc/Include
    ${CMAKE_SOURCE_DIR}/Firmware/GD32F4xx_usb_library/device/core/Include
    ${CMAKE_SOURCE_DIR}/Firmware/GD32F4xx_usb_library/driver/Include
    ${CMAKE_SOURCE_DIR}/Firmware/GD32F4xx_usb_library/ustd/common
    ${CMAKE_SOURCE_DIR}/Firmware/GD32F4xx_usb_library/ustd/class/cdc
)
# 递归查找Firmware文件夹下所有C文件
file(GLOB_RECURSE SRC_DIR_LIST 
    ${CMAKE_SOURCE_DIR}/Firmware/CMSIS/*.c
    ${CMAKE_SOURCE_DIR}/Firmware/GD32F4xx_standard_peripheral/*.c
    ${CMAKE_SOURCE_DIR}/Firmware/GD32F4xx_usb_library/device/core/Source/*.c
    ${CMAKE_SOURCE_DIR}/Firmware/GD32F4xx_usb_library/driver/Source/*.c
    ${CMAKE_SOURCE_DIR}/Firmware/GD32F4xx_usb_library/device/class/cdc/Source/*.c
)

# 源文件
target_sources(gd32f470HAL INTERFACE
    ${SRC_DIR_LIST}
    ${CMAKE_SOURCE_DIR}/Firmware/startup_gd32f4xx.s
)

target_link_directories(gd32f470HAL INTERFACE
)

target_link_libraries(gd32f470HAL INTERFACE
)

# Validate that gd32f470HAL code is compatible with C standard
if(CMAKE_C_STANDARD LESS 11)
    message(ERROR "Generated code requires C11 or higher")
endif()


