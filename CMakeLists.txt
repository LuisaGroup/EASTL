#-------------------------------------------------------------------------------------------
# Copyright (C) Electronic Arts Inc.  All rights reserved.
#-------------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.20)
project(EASTL CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

#-------------------------------------------------------------------------------------------
# Compiler Flags
#-------------------------------------------------------------------------------------------
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/scripts/CMake")
include(CommonCppFlags)

# mimalloc
option(EASTL_ENABLE_MIMALLOC "Enable mimalloc" ${WIN32})

#-------------------------------------------------------------------------------------------
# Library definition
#-------------------------------------------------------------------------------------------
file(GLOB EASTL_SOURCES "source/*.cpp")
add_library(EASTL-object OBJECT ${EASTL_SOURCES})

add_library(EASTL-interface INTERFACE)
target_compile_definitions(EASTL-interface INTERFACE
        EA_PRAGMA_ONCE_SUPPORTED=1
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
        EASTL_USER_LITERALS_ENABLED=0
        EASTL_STD_ITERATOR_CATEGORY_ENABLED=1
        EASTL_STD_TYPE_TRAITS_AVAILABLE=1
        EASTL_MOVE_SEMANTICS_ENABLED=1
        EASTL_VARIADIC_TEMPLATES_ENABLED=1
        EASTL_VARIABLE_TEMPLATES_ENABLED=1
        EASTL_INLINE_VARIABLE_ENABLED=1
        EASTL_HAVE_CPP11_TYPE_TRAITS=1
        EASTL_INLINE_NAMESPACES_ENABLED=1
        EASTL_ALLOCATOR_EXPLICIT_ENABLED=1
        EA_DLL=1
        EASTL_USER_DEFINED_ALLOCATOR=1)

if (WIN32 OR EASTL_ENABLE_MIMALLOC)
    set(MI_OVERRIDE OFF CACHE BOOL "" FORCE)
    set(MI_XMALLOC ON CACHE BOOL "" FORCE)
    set(MI_USE_CXX ON CACHE BOOL "" FORCE)
    set(MI_OSX_ZONE ON CACHE BOOL "" FORCE)
    set(MI_BUILD_STATIC ON CACHE BOOL "" FORCE)
    set(MI_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    set(MI_BUILD_OBJECT OFF CACHE BOOL "" FORCE)
    set(MI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    add_subdirectory(packages/mimalloc)
    if (MSVC)
        target_compile_definitions(mimalloc-static PUBLIC _CRT_SECURE_NO_WARNINGS=1)
    endif ()
    set_target_properties(mimalloc-static PROPERTIES UNITY_BUILD ON)
    target_compile_definitions(EASTL-object PRIVATE EASTL_MIMALLOC_ENABLED=1 EASTL_PROJECT=1)
    target_link_libraries(EASTL-object PUBLIC mimalloc-static)
endif ()

if (WIN32)
    target_link_options(EASTL-object PRIVATE "LINKER:/NATVIS:${CMAKE_CURRENT_SOURCE_DIR}/doc/EASTL.natvis")
endif ()

set_target_properties(EASTL-object PROPERTIES UNITY_BUILD ON POSITION_INDEPENDENT_CODE ON)
target_link_libraries(EASTL-object PUBLIC EASTL-interface)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND
        CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15.0)
    target_compile_options(EASTL-interface INTERFACE -Wno-deprecated-builtins)
endif ()

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
target_include_directories(EASTL-interface INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

#-------------------------------------------------------------------------------------------
# Dependencies
#-------------------------------------------------------------------------------------------
add_subdirectory(packages/EABase)
target_link_libraries(EASTL-interface INTERFACE EABase)
