#-------------------------------------------------------------------------------------------
# Copyright (C) Electronic Arts Inc.  All rights reserved.
#-------------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.18)
project(EASTL CXX)

#-------------------------------------------------------------------------------------------
# Compiler Flags
#-------------------------------------------------------------------------------------------
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/scripts/CMake")
include(CommonCppFlags)

#-------------------------------------------------------------------------------------------
# Library definition
#-------------------------------------------------------------------------------------------
file(GLOB EASTL_SOURCES "source/*.cpp")
add_library(EASTL SHARED ${EASTL_SOURCES})

target_compile_definitions(EASTL PUBLIC
        EA_HAVE_CPP11_CONTAINERS=1
        EA_HAVE_CPP11_ATOMIC=1
        EA_HAVE_CPP11_CONDITION_VARIABLE=1
        EA_HAVE_CPP11_MUTEX=1
        EA_HAVE_CPP11_THREAD=1
        EA_HAVE_CPP11_FUTURE=1
        EA_HAVE_CPP11_TYPE_TRAITS=1
        EA_HAVE_CPP11_TUPLES=1
        EA_HAVE_CPP11_REGEX=1
        EA_HAVE_CPP11_RANDOM=1
        EA_HAVE_CPP11_CHRONO=1
        EA_HAVE_CPP11_SCOPED_ALLOCATOR=1
        EA_HAVE_CPP11_INITIALIZER_LIST=1
        EA_HAVE_CPP11_SYSTEM_ERROR=1
        EA_HAVE_CPP11_TYPEINDEX=1
        EASTL_STD_TYPE_TRAITS_AVAILABLE=1
        EASTL_MOVE_SEMANTICS_ENABLED=1
        EASTL_VARIADIC_TEMPLATES_ENABLED=1
        EASTL_VARIABLE_TEMPLATES_ENABLED=1
        EASTL_INLINE_VARIABLE_ENABLED=1
        EASTL_HAVE_CPP11_TYPE_TRAITS=1
        EASTL_INLINE_NAMESPACES_ENABLED=1)

set_target_properties(EASTL PROPERTIES
        UNITY_BUILD ON
        WINDOWS_EXPORT_ALL_SYMBOLS ON)

#-------------------------------------------------------------------------------------------
# Defines
#-------------------------------------------------------------------------------------------
add_definitions(-D_CHAR16T)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D_SCL_SECURE_NO_WARNINGS)
add_definitions(-DEASTL_OPENSOURCE=1)

#-------------------------------------------------------------------------------------------
# Include dirs
#-------------------------------------------------------------------------------------------
target_include_directories(EASTL PUBLIC include)

#-------------------------------------------------------------------------------------------
# Dependencies
#-------------------------------------------------------------------------------------------
add_subdirectory(packages/EABase)
target_link_libraries(EASTL EABase)
