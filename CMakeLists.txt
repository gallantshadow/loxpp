
# Version requirement
cmake_minimum_required(VERSION 4.1.2)

# Project name
project(Lox)

# C++23 verison
set(CMAKE_CXX_STANDARD 23)

# generate .json file for clangd lsp
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Source directory
add_subdirectory(src)

