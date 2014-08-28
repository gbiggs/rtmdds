# Find rtmdds.
#
# The following directories are searched:
# RTMDDS_ROOT (CMake variable)
# RTMDDS_ROOT (Environment variable)
#
# This sets the following variables:
# RTMDDS_FOUND - True if rtmdds was found.
# RTMDDS_INCLUDE_DIRS - Directories containing the rtmdds include files.
# RTMDDS_LIBRARIES - Libraries needed to use rtmdds.
# RTMDDS_DEFINITIONS - Compiler flags for rtmdds.
# RTMDDS_VERSION - The version of rtmdds found.
# RTMDDS_VERSION_MAJOR - The major version of rtmdds found.
# RTMDDS_VERSION_MINOR - The minor version of rtmdds found.
# RTMDDS_VERSION_PATCH - The revision version of rtmdds found.
# RTMDDS_VERSION_CAN - The candidate version of rtmdds found.
#
# This module also defines one macro usable in your CMakeLists.txt files:
# RTMDDS_COMPILE_IDL_FILES(file1 file2 ...)
#   Compiles the specified IDL files, placing the generated C++ source files in
#   ${CMAKE_CURRENT_BINARY_DIR}. The source files can be found in file1_SRCS,
#   file2_SRCS, etc., and all source files for all IDL files are available in
#   ALL_IDL_SRCS.

find_package(OpenRTM 1 REQUIRED)
find_package(DDS 4.5 REQUIRED)

find_package(PkgConfig)
pkg_check_modules(PC_RTMDDS QUIET rtmdds)

find_path(RTMDDS_INCLUDE_DIR rtmdds/ddsportmgmt.h
    HINTS ${RTMDDS_ROOT}/include $ENV{RTMDDS_ROOT}/include
    ${PC_RTMDDS_INCLUDE_DIRS})
find_library(RTMDDS_LIBRARY rtmdds
    HINTS ${RTMDDS_ROOT}/lib $ENV{RTMDDS_ROOT}/lib
    ${PC_RTMDDS_LIBRARY_DIRS})

set(RTMDDS_DEFINITIONS ${OPENRTM_DEFINITIONS} ${DDS_DEFINITIONS})
set(RTMDDS_INCLUDE_DIRS ${RTMDDS_INCLUDE_DIR} ${OPENRTM_INCLUDE_DIRS}
    ${DDS_INCLUDE_DIRS})
set(RTMDDS_LIBRARIES ${RTMDDS_LIBRARY} ${OPENRTM_LIBRARIES} ${DDS_LIBRARIES})

message(STATUS "Version from pkgconfig: ${PC_RTMDDS_VERSION}")
set(RTMDDS_VERSION ${PC_RTMDDS_VERSION})
string(REGEX REPLACE "^([0-9]+).*" "\\1"
    RTMDDS_VERSION_MAJOR "${RTMDDS_VERSION}")
string(REGEX REPLACE "^[0-9]+\\.([0-9]+).*" "\\1"
    RTMDDS_VERSION_MINOR "${RTMDDS_VERSION}")
string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1"
    RTMDDS_VERSION_PATCH ${RTMDDS_VERSION})
string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.[0-9]+(.*)" "\\1"
    RTMDDS_VERSION_CAN ${RTMDDS_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenRTM
    REQUIRED_VARS RTMDDS_INCLUDE_DIR RTMDDS_LIBRARY
    VERSION_VAR ${RTMDDS_VERSION})


macro(IDL_OUTPUTS _idl _dir _result)
    set(${_result} ${_dir}/${_idl}.h ${_dir}/${_idl}Dcps.h
        ${_dir}/${_idl}SacDcps.c ${_dir}/${_idl}SacDc ps.h
        ${_dir}/${_idl}SplDcps.c ${_dir}/${_idl}SplDcps.h)
endmacro(IDL_OUTPUTS)


macro(COMPILE_IDL _idl_file)
    get_filename_component(_idl ${_idl_file} NAME_WE)
    set(_idl_srcs_var ${_idl}_SRCS)
    IDL_OUTPUTS(${_idl} ${CMAKE_CURRENT_BINARY_DIR} ${_idl_srcs_var})
    add_custom_command(OUTPUT ${${_idl_srcs_var}}
        COMMAND idlpp -I $ENV{OSPL_HOME}/etc/idl -S -l c ${_idl_file}
        -d ${CMAKE_CURRENT_BINARY_DIR}
	WORKING_DIRECTORY ${CURRENT_BINARY_DIR}
        DEPENDS ${_idl_file}
        COMMENT "Compiling ${_idl_file}" VERBATIM)
    set(ALL_IDL_SRCS ${_idl_srcs} ${${_idl_srcs_var}})
endmacro(COMPILE_IDL)


# Module exposed to the user
macro(RTMDDS_COMPILE_IDL_FILES)
    set(idl_srcs)
    foreach(idl ${ARGN})
        COMPILE_IDL(${idl})
    endforeach(idl)
endmacro(RTMDDS_COMPILE_IDL_FILES)

