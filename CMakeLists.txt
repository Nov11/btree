cmake_minimum_required(VERSION 3.7)
project(btree)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp BTree.cpp BTree.h)
add_executable(btree ${SOURCE_FILES})