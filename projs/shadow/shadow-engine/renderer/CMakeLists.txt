

FILE(GLOB_RECURSE SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/base/src/*.cpp
        ${CMAKE_CURRENT_LIST_DIR}/vulkan/src/*.cpp
        )

target_shadow_module(shadow-engine
        SOURCES ${SOURCES}
        INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/base/inc/
        INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/vulkan/inc
        )