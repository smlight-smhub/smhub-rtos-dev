#
# Copyright (c) 2026 SMLIGHT
# All rights reserved.
#
# Core CMake build list for framework-sg2000-rtos.
#
cmake_minimum_required(VERSION 3.16)
project(framework-sg2000-rtos C CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


file(GLOB_RECURSE HAL_SOURCES
    "lib/smhub-hal/hal/cv181x/src/*.c"
    "lib/smhub-hal/driver/src/*.c"
)

add_library(smhub-hal STATIC ${HAL_SOURCES})
target_include_directories(smhub-hal PUBLIC
    "lib/smhub-hal/hal/cv181x/include"
    "lib/smhub-hal/driver/include"
    "lib/smhub-hal/common/include"
    "lib/smhub-hal/arch/include"
)


file(GLOB FREERTOS_SOURCES
    "lib/FreeRTOS-Kernel/*.c"
    "lib/FreeRTOS-Kernel/portable/GCC/RISC-V/*.c"
    "lib/FreeRTOS-Kernel/portable/MemMang/heap_4.c"
)

add_library(freertos STATIC ${FREERTOS_SOURCES})
target_include_directories(freertos PUBLIC
    "lib/FreeRTOS-Kernel/include"
    "lib/FreeRTOS-Kernel/portable/GCC/RISC-V"
    "include" # usually for FreeRTOSConfig.h
)


file(GLOB_RECURSE METAL_SOURCES
    "lib/libmetal/lib/*.c"
    "lib/libmetal/lib/system/generic/*.c"
    "lib/libmetal/lib/processor/riscv/*.c"
)

add_library(metal STATIC ${METAL_SOURCES})
target_compile_definitions(metal PRIVATE METAL_INTERNAL DEFAULT_LOGGER_ON=0)
target_include_directories(metal PUBLIC "lib/libmetal/lib" "lib/libmetal")


file(GLOB_RECURSE OPENAMP_SOURCES "lib/open-amp/lib/*.c")

add_library(openamp STATIC ${OPENAMP_SOURCES})
target_include_directories(openamp PUBLIC "lib/open-amp/lib/include")
target_link_libraries(openamp PUBLIC metal)


add_library(sg2000_sdk INTERFACE)
target_link_libraries(sg2000_sdk INTERFACE smhub-hal freertos openamp)
target_link_options(sg2000_sdk INTERFACE
    -T ${CMAKE_CURRENT_SOURCE_DIR}/smhub_lscript.ld
)
