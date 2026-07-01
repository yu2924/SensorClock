cmake_minimum_required(VERSION 3.5)

set(PROJECT_VER "1.0.0")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

add_compile_options(-Wno-format)
add_compile_options(-Wno-ignored-qualifiers)
add_compile_options(-Wno-attributes)
add_compile_options(-Wno-unused-function)
project(SensorClock)

# FlogFS for assets
set(FROGFS_NAME frogfs)
target_add_frogfs(${PROJECT_NAME}.elf CONFIG ${FROGFS_NAME}.yaml NAME ${FROGFS_NAME})

# BSP DMA2D & PPA support
add_compile_definitions(CONFIG_LV_ATTRIBUTE_LARGE_RAM_ARRAY=EXT_RAM_BSS_ATTR)
