cmake_minimum_required(VERSION 3.20)

project(arm2mips)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CORE_SOURCES
    src/arm_decoder.cpp
    src/arm_interpreter.cpp
    src/arm_memory.cpp
    src/arm_engine.cpp
    src/arm_cpu.cpp
    src/arm_cfg.cpp
    src/arm_block_builder.cpp
)

add_executable(arm2mips
    src/main.cpp
    ${CORE_SOURCES}
    src/mips.cpp
)

add_executable(arm_tests
    src/tests.cpp
    ${CORE_SOURCES}
)

add_executable(analyze_eboot
    src/analyze_eboot.cpp
)

add_executable(vita_self
    src/vita_self.cpp
)