# Find a DDS implementation.
# Currently only RTI's DDS implementation is searched for.
#
# The following directories are searched:
# DDS_ROOT (CMake variable)
# DDS_ROOT (Environment variable)
# OSPL_HOME (Environment variable)
#
# Prior to calling this script, you may set the DDS_HOST variable. This is used
# when searching for some implementations as a name for the directory
# containing the library files. For example, it could be set to
# "x64Linux2.6gcc4.1.1".
#
# This sets the following variables:
# DDS_FOUND - True if DDS was found.
# DDS_VENDOR - Name of the DDS vendor found (e.g. "RTI")
# DDS_INCLUDE_DIRS - Directories containing the DDS include files.
# DDS_LIBRARIES - Libraries needed to use DDS.
# DDS_DEFINITIONS - Compiler flags for DDS.
# DDS_VERSION - The version of DDS found.
# DDS_VERSION_MAJOR - The major version of DDS found.
# DDS_VERSION_MINOR - The minor version of DDS found.
# DDS_VERSION_PATCH - The revision version of DDS found.
# DDS_VERSION_CAN - The candidate version of DDS found.

find_path(DDS_INCLUDE_DIR dds_dcps.h
    HINTS ${DDS_ROOT}/include $ENV{DDS_ROOT}/include
    $ENV{OSPL_HOME}/include
    PATH_SUFFIXES dcps/C/SAC)

if(NOT DDS_HOST)
    set(DDS_HOST "x86.linux2.6")
endif(NOT DDS_HOST)
find_library(DDS_C_LIBRARY dcpssac
    HINTS ${DDS_ROOT}/lib $ENV{DDS_ROOT}/lib $ENV{OSPL_HOME}/lib)
find_library(DDS_CPP_LIBRARY dcpssacpp
    HINTS ${DDS_ROOT}/lib $ENV{DDS_ROOT}/lib $ENV{OSPL_HOME}/lib)
find_library(DDS_CORE_LIBRARY ddskernel
    HINTS ${DDS_ROOT}/lib $ENV{DDS_ROOT}/lib $ENV{OSPL_HOME}/lib)
find_library(DDS_UTIL_LIBRARY ddsutil
    HINTS ${DDS_ROOT}/lib $ENV{DDS_ROOT}/lib $ENV{OSPL_HOME}/lib)
find_library(DDS_MISC_LIBRARY
    NAMES ddsuser ddsserialization ddsconf ddsconfparser ddsdatabase ddsos
    HINTS ${DDS_ROOT}/lib $ENV{DDS_ROOT}/lib $ENV{OSPL_HOME}/lib)

if(WIN32)
    set(DDS_EXTRA_LIBRARIES netapi32 advapi32 user32 ws2_32)
    set(DDS_DEFINITIONS -DRTI_WIN32 -DNDDS_DLL_VARIABLE /MD)
else(WIN32)
    set(DDS_EXTRA_LIBRARIES dl nsl m pthread rt)
    if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
        set(bits_flag -m64)
	set(DDS_DEFINITIONS -DRTI_UNIX -m64)
    else(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
        set(bits_flag -m32)
	set(DDS_DEFINITIONS -DRTI_UNIX -D__i386__=1 -m32)
    endif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
endif(WIN32)

set(DDS_INCLUDE_DIRS ${DDS_INCLUDE_DIR} $ENV{OSPL_HOME}/include $ENV{OSPL_HOME}/include/sys)
set(DDS_LIBRARIES ${DDS_C_LIBRARY} ${DDS_CPP_LIBRARY} ${DDS_CORE_LIBRARY}
    ${DDS_EXTRA_LIBRARIES})

file(GLOB DDS_VERSION_FILE ${DDS_ROOT}/etc/RELEASEINFO
    $ENV{DDS_ROOT}/etc/RELEASEINFO
    $ENV{OSPL_HOME}/etc/RELEASEINFO)
if(DDS_VERSION_FILE)
    string(REGEX MATCH "PACKAGE_VERSION=V([0-9]+).([0-9]+).([A-Z0-9]*)"
        DDS_VERSION "${DDS_VERSION_FILE}")
    set(DDS_VERSION_MAJOR ${CMAKE_MATCH_1})
    set(DDS_VERSION_MINOR ${CMAKE_MATCH_2})
    set(DDS_VERSION_PATCH ${CMAKE_MATCH_3})
    set(DDS_VERSION_CAN)
endif(DDS_VERSION_FILE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DDS
    REQUIRED_VARS DDS_INCLUDE_DIR DDS_C_LIBRARY DDS_CPP_LIBRARY
    DDS_CORE_LIBRARY DDS_UTIL_LIBRARY DDS_MISC_LIBRARY
    VERSION_VAR ${DDS_VERSION})

