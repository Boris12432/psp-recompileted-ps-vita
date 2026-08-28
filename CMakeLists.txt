cmake_minimum_required(VERSION 3.20)

project(arm2mips)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(arm2mips
    src/main.cpp
    src/arm_decoder.cpp
    src/arm_interpreter.cpp
    src/arm_memory.cpp
    src/arm_engine.cpp
    src/mips.cpp
)

add_executable(arm_tests
    src/tests.cpp
    src/arm_decoder.cpp
    src/arm_interpreter.cpp
    src/arm_memory.cpp
    src/arm_engine.cpp
)