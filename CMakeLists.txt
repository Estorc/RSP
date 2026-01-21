cmake_minimum_required(VERSION 3.10)
project(RSP C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)


file(GLOB_RECURSE SOURCES "src/*.c")

# Create executable
add_executable(rsp ${SOURCES})

# Include directories
target_include_directories(rsp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Optional: Enable compiler warnings
if(MSVC)
    target_compile_options(rsp PRIVATE /W4)
else()
    target_compile_options(rsp PRIVATE -Wall -Wextra -Wpedantic)
endif()