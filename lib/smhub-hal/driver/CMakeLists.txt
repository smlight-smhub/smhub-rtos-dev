cmake_minimum_required(VERSION 3.16.5)

project(driver C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_INSTALL_PREFIX ${TOP_DIR}/install)
set(CMAKE_INSTALL_INC_PREFIX ${TOP_DIR}/install/include)
set(CMAKE_DRIVER_DIR ${TOP_DIR}/driver)

set(SAFETY_FLAGS "-Wall -Wextra -fno-strict-aliasing")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SAFETY_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SAFETY_FLAGS}")

add_compile_definitions(F18)

include_directories(include)

file(GLOB _SOURCES "src/*.c")
file(GLOB _HEADERS "include/*.h")

add_library(driver STATIC ${_SOURCES})
install(TARGETS driver DESTINATION lib)
install(FILES ${_HEADERS} DESTINATION include/driver)
