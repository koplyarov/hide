project(hide)
cmake_minimum_required(VERSION 2.6)
file(GLOB hide_SRCS ./*.cpp ./clang/*.cpp)

find_path(LibClang_INCLUDE_DIRS clang-c/Index.h HINTS /usr/lib/llvm-3.4/include)
find_library(LibClang_LIBRARIES NAMES clang HINTS /usr/lib/llvm-3.4/lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

include_directories(./)
include_directories(${LibClang_INCLUDE_DIRS})

add_definitions(-std=c++11)

add_executable(hide WIN32 ${hide_SRCS})

target_link_libraries(hide ${LibClang_LIBRARIES})