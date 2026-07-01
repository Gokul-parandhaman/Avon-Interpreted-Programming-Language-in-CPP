cmake_minimum_required(VERSION 3.10)
project(Nova VERSION 2.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

add_executable(nova
    src/Lexer.cpp
    src/Parser.cpp
    src/bytecode/BytecodeCompiler.cpp
    src/bytecode/NovaVM.cpp
    src/main.cpp
)

target_include_directories(nova PRIVATE src)

install(TARGETS nova DESTINATION bin)