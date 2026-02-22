cmake_minimum_required(VERSION 3.10)
project(RSP C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)


# Source files
set(RSP_SOURCES
    src/rsp.c
)

set(SOURCES
    src/main.c
)

# Create executable
add_library(rsp STATIC ${RSP_SOURCES})
add_executable(rsp_executable ${SOURCES})

# Link the rsp library to the executable
target_link_libraries(rsp_executable PRIVATE rsp)

# Include directories
target_include_directories(rsp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(rsp_executable PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Optional: Enable compiler warnings
if(MSVC)
    target_compile_options(rsp PRIVATE /W4)
else()
    target_compile_options(rsp PRIVATE -Wall -Wextra -Wpedantic)
endif()