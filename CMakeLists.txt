CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
project(SPL)

set(CMAKE_CXX_STANDARD 11)

aux_source_directory(./src/ DIR_SRCS)
aux_source_directory(./src/astnode DIR_SRCS)

add_executable(splc ${DIR_SRCS} src/parse/syntax.cc src/parse/deliver.cpp)
target_link_libraries(splc -ly -lfl)
