#-------------------------------------------------------------------------------------------
# Copyright (C) Electronic Arts Inc.  All rights reserved.
#-------------------------------------------------------------------------------------------

#-------------------------------------------------------------------------------------------
# CMake info
#-------------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.11)
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

# Parts of the test suite fail to compile if char8_t is enabled, so we disable it
if (EATHREAD_NO_CHAR8T_FLAG)
    add_compile_options(${EATHREAD_NO_CHAR8T_FLAG})
endif()

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
FetchContent_Declare(
  EABase
  GIT_REPOSITORY https://github.com/electronicarts/EABase.git
  GIT_TAG        521cb053d9320636f53226ffc616216cf532f0ef
  GIT_SUBMODULES "" # This should be temporary until we update the cyclic submodule dependencies in EABase.
)

FetchContent_MakeAvailable(EABase)
target_link_libraries(EAThreadTest EABase)

FetchContent_Declare(
  EAAssert
  GIT_REPOSITORY https://github.com/electronicarts/EAAssert.git
  GIT_TAG        e5e181255de2e883dd1f987c78ccc42ac81d3bca
  GIT_SUBMODULES "" # This should be temporary until we update the cyclic submodule dependencies in EAAssert.
)
FetchContent_MakeAvailable(EAAssert)
target_link_libraries(EAThreadTest EAAssert)

FetchContent_Declare(
  EAStdC
  GIT_REPOSITORY https://github.com/electronicarts/EAStdC.git
  GIT_TAG        fbcc34e89c63636054334888f3a5bd7ac2fd4b76
  GIT_SUBMODULES "" # This should be temporary until we update the cyclic submodule dependencies in EAStdC.
)
FetchContent_MakeAvailable(EAStdC)
target_link_libraries(EAThreadTest EAStdC)

FetchContent_Declare(
  EAMain
  GIT_REPOSITORY https://github.com/electronicarts/EAMain.git
  GIT_TAG        24ca8bf09e6b47b860286fc2f4c832f4009273d1
  GIT_SUBMODULES "" # This should be temporary until we update the cyclic submodule dependencies in EAMain.
)
FetchContent_MakeAvailable(EAMain)
target_link_libraries(EAThreadTest EAMain)

FetchContent_Declare(
  EATest
  GIT_REPOSITORY https://github.com/electronicarts/EATest.git
  GIT_TAG        a59b372fc9cba517283ad6d060d2ab96e0ba34ac
  GIT_SUBMODULES "" # This should be temporary until we update the cyclic submodule dependencies in EATest.
)
FetchContent_MakeAvailable(EATest)
target_link_libraries(EAThreadTest EATest)

FetchContent_Declare(
  EASTL
  GIT_REPOSITORY https://github.com/electronicarts/EASTL.git
  GIT_TAG        05f4b4aef33f2f3ded08f19fa97f5a27ff35ff9f
  GIT_SUBMODULES "" # This should be temporary until we update the cyclic submodule dependencies in EASTL.
)
FetchContent_MakeAvailable(EASTL)
target_link_libraries(EAThreadTest EASTL)

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

