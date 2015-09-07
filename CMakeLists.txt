cmake_minimum_required(VERSION 2.8)
cmake_policy (SET CMP0020 NEW)

project("InterfaceMonitor")
message ( STATUS "InterfaceMonitor" )

SET(CMAKE_BUILD_TYPE Debug)

set (CMAKE_FIND_ROOT_PATH ${ENV_PATH})
set (CMAKE_PREFIX_PATH ${ENV_PATH})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/BIN/)

find_package(Boost 1.55.0 REQUIRED system thread chrono unit_test_framework)

find_package(PkgConfig REQUIRED)

#DBUS
pkg_check_modules(DBUS dbus-1)

find_library(DBUS_LIBRARY
    NAMES dbus-1
    HINTS ${PC_DBUS_LIBDIR}
          ${PC_DBUS_LIBRARY_DIRS}
)

#GLIB
pkg_check_modules (GLIB2 glib-2.0)

set (GLIB2_LIBRARIES  libglib-2.0.so
                      libgio-2.0.so
                      libgmodule-2.0.so
                      libgobject-2.0.so
                      libgthread-2.0.so)

include_directories(${DBUS_INCLUDE_DIRS})
include_directories(${GLIB2_INCLUDE_DIRS})
include_directories("./")
include_directories("./SRC/InterfaceManager")
include_directories("./SRC/InterfaceMonitor")
include_directories("${CMAKE_BINARY_DIR}")

add_subdirectory(SRC)
