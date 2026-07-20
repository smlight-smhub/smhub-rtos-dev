cmake_minimum_required(VERSION 3.16.5)

project(common C ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_INSTALL_PREFIX ${TOP_DIR}/install)
set(CMAKE_INSTALL_INC_PREFIX ${TOP_DIR}/install/include)

set(SAFETY_FLAGS "-Wall -Wextra -fno-strict-aliasing")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SAFETY_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SAFETY_FLAGS}")

file(GLOB _SOURCES "src/riscv64/*.c" "src/riscv64/riscv_optimized/*.S")
file(GLOB _HEADERS "include/riscv64/*.h")

include_directories(include/riscv64)
include_directories(${CMAKE_INSTALL_INC_PREFIX}/arch)
include_directories(${CMAKE_INSTALL_INC_PREFIX}/kernel)

add_library(common STATIC ${_SOURCES})

install(TARGETS common DESTINATION lib)
install(FILES ${_HEADERS} DESTINATION include/common)
