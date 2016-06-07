project(hide)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

find_package(Threads)

find_package(PythonLibs REQUIRED)
message(STATUS "PythonLibs: ${PYTHON_LIBRARIES}")

find_package(Boost REQUIRED COMPONENTS
	program_options
	regex
	filesystem
	system
	exception
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")

include_directories(${PYTHON_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${LibClang_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/rethread)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/wigwag/include)

if (MSVC)
	add_definitions(-DBOOST_ALL_NO_LIB)
	add_definitions(-DMS_NO_COREDLL)
	add_definitions(-DPy_ENABLE_SHARED)
	add_definitions(-DSWIG_PYTHON_INTERPRETER_NO_DEBUG)
	add_definitions(-DHIDE_PLATFORM_WINDOWS=1)
	add_definitions(-D_SCL_SECURE_NO_WARNINGS)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4250")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
	add_definitions(-std=c++11)
	add_definitions(-Wall)
	add_definitions(-DHIDE_PLATFORM_POSIX=1)
endif()

######################################################################

set(hide_SRCS
	hide/Buffer.cpp
	hide/ContextUnawareSyntaxHighlighter.cpp
	hide/Indexer.cpp
	hide/Project.cpp
	hide/ProjectFiles.cpp
	hide/buildsystems/BuildProcessBase.cpp
	hide/buildsystems/DefaultBuildProcess.cpp
	hide/buildsystems/cmake/CMakeBuildSystem.cpp
	hide/buildsystems/cmake/NinjaCMakeBackend.cpp
	hide/lang_plugins/CTagsInvoker.cpp
	hide/lang_plugins/GenericCTagsIndexer.cpp
	hide/lang_plugins/cpp/CompileCommandsJsonCompilationInfo.cpp
	hide/lang_plugins/cpp/CppCTagsIndexer.cpp
	hide/lang_plugins/cpp/File.cpp
	hide/lang_plugins/cpp/LanguagePlugin.cpp
	hide/utils/Executable.cpp
	hide/utils/FileSystemNotifier.cpp
	hide/utils/FileSystemUtils.cpp
	hide/utils/Logger.cpp
	hide/utils/LoggerMessage.cpp
	hide/utils/PipeLinesReader.cpp
	hide/utils/Thread.cpp
)

add_library(hide-core STATIC ${hide_SRCS})

target_link_libraries(hide-core
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	${Boost_REGEX_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_EXCEPTION_LIBRARY}
	${LibClang_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
)

######################################################################

set(hide_test_SRCS test.cpp)

add_executable(hide-test ${hide_test_SRCS})

target_link_libraries(hide-test
	hide-core
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	${Boost_REGEX_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_EXCEPTION_LIBRARY}
	${LibClang_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
)

######################################################################

set(swig_SRCS swig/hide.i)

SET(SWIG_MODULE_hide_EXTRA_DEPS ${hide_SRCS})

set(CMAKE_SWIG_FLAGS "-threads")
set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/bin)

set_source_files_properties(${swig_SRCS} PROPERTIES CPLUSPLUS ON)
if (NOT MSVC)
	set_source_files_properties("bin/hidePYTHON_wrap.cxx" PROPERTIES COMPILE_FLAGS "-Wno-maybe-uninitialized")
endif()

swig_add_module(hide python ${swig_SRCS})
swig_link_libraries(hide hide-core ${PYTHON_LIBRARIES})
