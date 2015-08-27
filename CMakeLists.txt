cmake_minimum_required(VERSION 2.8)
cmake_policy (SET CMP0020 NEW)

project("InterfaceMonitor")
message ( STATUS "INterfaceMonitor" )

SET(CMAKE_BUILD_TYPE Debug)

set (CMAKE_FIND_ROOT_PATH ${ENV_PATH})
set (CMAKE_PREFIX_PATH ${ENV_PATH})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/BIN/)

include_directories("./")
include_directories("./SRC/InterfaceManager")
include_directories("./SRC/InterfaceMonitor")
include_directories("${CMAKE_BINARY_DIR}")

add_subdirectory(SRC)
