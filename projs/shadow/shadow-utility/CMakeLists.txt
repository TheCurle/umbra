set(CMAKE_CXX_STANDARD 20)

FILE(GLOB_RECURSE SOURCES src/*.cpp inc/*.h)

add_library(shadow-utils ${SOURCES})
target_include_directories(shadow-utils PUBLIC inc)
