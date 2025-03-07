cmake_minimum_required(VERSION 3.19)

# PATH OF THE MAX SDK
set (C74_SDK "" CACHE PATH "Path to the Max SDK")
if(NOT C74_SDK)
  message(FATAL_ERROR "Max SDK Path is not set")
endif()

# Detect operating system and configure accordingly
if(CMAKE_SYSTEM_NAME STREQUAL "Windows" OR WIN32)
    message(STATUS "Configuring for Windows cross-compilation")
    set(CMAKE_SYSTEM_NAME Windows)
    set(CMAKE_SYSTEM_PROCESSOR x86_64)
    set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
    set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
    set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)
    set(CMAKE_FIND_ROOT_PATH /opt/local/x86_64-w64-mingw32 /usr/local/x86_64-w64-mingw32 /usr/x86_64-w64-mingw32)
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
else()
    message(STATUS "Configuring for macOS")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14")
endif()

# Define project name
string(REGEX REPLACE "(.*)/" "" THIS_FOLDER_NAME "${CMAKE_CURRENT_SOURCE_DIR}")
project(${THIS_FOLDER_NAME})

# Set output directory for compiled binaries
set(C74_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../externals)

# Add subdirectories for compilation
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/bufGranul~)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/liveGranul~)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/synGranul~)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/mc.bufGranul~)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/mc.liveGranul~)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/mc.synGranul~)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/bargraf~)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/idleubutton)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/rand_dist_list)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/rand_dist_list~)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/multiouts~)

