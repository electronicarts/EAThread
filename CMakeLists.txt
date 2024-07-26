#-------------------------------------------------------------------------------------------
# Copyright (C) Electronic Arts Inc.  All rights reserved.
#-------------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.11)
include(FetchContent)
project(EAThread CXX)

#-------------------------------------------------------------------------------------------
# Options
#-------------------------------------------------------------------------------------------
option(EATHREAD_BUILD_TESTS "Enable generation of build files for tests" OFF)

#-------------------------------------------------------------------------------------------
# Compiler Flags
#-------------------------------------------------------------------------------------------
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/scripts/CMake")
include(CommonCppFlags)

#-------------------------------------------------------------------------------------------
# Library definition
#-------------------------------------------------------------------------------------------
file(GLOB EATHREAD_SOURCES "source/*.cpp")
add_library(EAThread ${EATHREAD_SOURCES})

#-------------------------------------------------------------------------------------------
# Defines
#-------------------------------------------------------------------------------------------
add_definitions(-D_CHAR16T)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

#-------------------------------------------------------------------------------------------
# Export Include Directories
#-------------------------------------------------------------------------------------------
target_include_directories(EAThread PUBLIC include)

#-------------------------------------------------------------------------------------------
# Package Dependencies
#-------------------------------------------------------------------------------------------
FetchContent_Declare(
  EABase
  GIT_REPOSITORY https://github.com/electronicarts/EABase.git
  GIT_TAG        521cb053d9320636f53226ffc616216cf532f0ef
  GIT_SUBMODULES "" # This should be temporary until we update the cyclic submodule dependencies in EABase.
)

FetchContent_MakeAvailable(EABase)

target_link_libraries(EAThread EABase)

if(EATHREAD_BUILD_TESTS)
    add_subdirectory(test)
endif()
