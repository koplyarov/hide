project(hide)
cmake_minimum_required(VERSION 2.6)

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

find_package(PythonLibs REQUIRED)

find_package(Boost REQUIRED COMPONENTS
	program_options
	regex
	filesystem
	system
	exception
)

find_path(LibClang_INCLUDE_DIRS clang-c/Index.h HINTS /usr/lib/llvm-3.4/include /usr/lib/llvm-3.5/include)
find_library(LibClang_LIBRARIES NAMES clang HINTS /usr/lib/llvm-3.4/lib /usr/lib/llvm-3.5/lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

include_directories(${PYTHON_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${LibClang_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR})

add_definitions(-std=c++11)

add_definitions(-DHIDE_PLATFORM_POSIX=1) # TODO: implement platform detection

######################################################################

set(hide_SRCS
	hide/Buffer.cpp
	hide/Indexer.cpp
	hide/Project.cpp
	hide/buildsystems/BuildProcessBase.cpp
	hide/buildsystems/DefaultBuildProcess.cpp
	hide/buildsystems/cmake/CMakeBuildSystem.cpp
	hide/buildsystems/cmake/NinjaCMakeBackend.cpp
	hide/lang_plugins/cpp/File.cpp
	hide/lang_plugins/cpp/LanguagePlugin.cpp
	hide/lang_plugins/cpp/clang/Index.cpp
	hide/utils/Executable.cpp
	hide/utils/Logger.cpp
	hide/utils/LoggerMessage.cpp
	hide/utils/ReadBufferLinesListener.cpp
	hide/utils/Thread.cpp
)

add_library(hide SHARED ${hide_SRCS})

target_link_libraries(hide
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	${Boost_REGEX_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_EXCEPTION_LIBRARY}
	${LibClang_LIBRARIES}
)

######################################################################

set(swig_SRCS swig/hide.i)

SET(SWIG_MODULE_hide_EXTRA_DEPS ${hide_SRCS})

set(CMAKE_SWIG_FLAGS "-threads")
set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/bin)

set_source_files_properties(${swig_SRCS} PROPERTIES CPLUSPLUS ON)
swig_add_module(hide python ${swig_SRCS})
swig_link_libraries(hide hide ${PYTHON_LIBRARIES})

######################################################################

set(test_SRCS
	test.cpp
)

add_executable(hide-test WIN32 ${test_SRCS})

target_link_libraries(hide-test hide)
