#-------------------------------------------------------------------------------------------
# Copyright (C) Electronic Arts Inc.  All rights reserved.
#-------------------------------------------------------------------------------------------

#-------------------------------------------------------------------------------------------
# CMake info
#-------------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.1)
project(EAThreadTest CXX)
include(CTest)

#-------------------------------------------------------------------------------------------
# Defines
#-------------------------------------------------------------------------------------------
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D_SCL_SECURE_NO_WARNINGS)
add_definitions(-D_CHAR16T)
add_definitions(-DEA_OPENSOURCE)

#-------------------------------------------------------------------------------------------
# Compiler Flags
#-------------------------------------------------------------------------------------------
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/packages/EASTL/scripts/CMake")
include(CommonCppFlags)

#-------------------------------------------------------------------------------------------
# Source files
#-------------------------------------------------------------------------------------------
file(GLOB EATHREADTEST_SOURCES "thread/source/*.cpp")
set(SOURCES ${EATHREADTEST_SOURCES})


#-------------------------------------------------------------------------------------------
# Executable definition
#-------------------------------------------------------------------------------------------
add_executable(EAThreadTest ${SOURCES})

#-------------------------------------------------------------------------------------------
# Include directories
#-------------------------------------------------------------------------------------------
target_include_directories(EAThreadTest PUBLIC include)

#-------------------------------------------------------------------------------------------
# Dependencies 
#-------------------------------------------------------------------------------------------
add_subdirectory(packages/EAAssert)
add_subdirectory(packages/EABase)
add_subdirectory(packages/EAMain)
add_subdirectory(packages/EASTL)
add_subdirectory(packages/EAStdC)
add_subdirectory(packages/EATest)

target_link_libraries(EAThreadTest EAAssert)
target_link_libraries(EAThreadTest EABase)
target_link_libraries(EAThreadTest EAMain)
target_link_libraries(EAThreadTest EASTL)
target_link_libraries(EAThreadTest EAStdC)
target_link_libraries(EAThreadTest EATest)
target_link_libraries(EAThreadTest EAThread)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if((NOT APPLE) AND (NOT WIN32))
    target_link_libraries(EAThreadTest ${EASTLTest_Libraries} Threads::Threads rt)
else()
    target_link_libraries(EAThreadTest ${EASTLTest_Libraries} Threads::Threads)
endif()

#-------------------------------------------------------------------------------------------
# Run Unit tests and verify the results.
#-------------------------------------------------------------------------------------------
add_test(EAThreadTestRuns EAThreadTest)
set_tests_properties (EAThreadTestRuns PROPERTIES PASS_REGULAR_EXPRESSION "RETURNCODE=0")

