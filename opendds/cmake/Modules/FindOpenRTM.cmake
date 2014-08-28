# Find OpenRTM-aist
#
# The following directories are searched:
# OPENRTM_ROOT (CMake variable)
# OPENRTM_ROOT (Environment variable)
#
# This sets the following variables:
# OPENRTM_FOUND - True if OpenRTM-aist was found.
# OPENRTM_INCLUDE_DIRS - Directories containing the OpenRTM-aist include files.
# OPENRTM_LIBRARIES - Libraries needed to use OpenRTM-aist.
# OPENRTM_DEFINITIONS - Compiler flags for OpenRTM-aist.
# OPENRTM_VERSION - The version of OpenRTM-aist found.
# OPENRTM_VERSION_MAJOR - The major version of OpenRTM-aist found.
# OPENRTM_VERSION_MINOR - The minor version of OpenRTM-aist found.
# OPENRTM_VERSION_PATCH - The revision version of OpenRTM-aist found.
# OPENRTM_VERSION_CAN - The candidate version of OpenRTM-aist found.
# OPENRTM_IDL_WRAPPER - rtm-skelwrapper command
# OPENRTM_IDL_WRAPPER_FLAGS - rtm-skelwrapper flag
# OPENRTM_IDLC - IDL command
# OPENRTM_IDLFLAGS - IDL optins

find_package(PkgConfig)
pkg_check_modules(PC_OPENRTM QUIET openrtm-aist)
pkg_check_modules(PC_COIL QUIET libcoil)

find_path(OPENRTM_INCLUDE_DIR rtm/RTC.h
    HINTS ${OPENRTM_ROOT}/include $ENV{OPENRTM_ROOT}/include
    ${PC_OPENRTM_INCLUDE_DIRS})
find_path(COIL_INCLUDE_DIR coil/config_coil.h
    HINTS ${OPENRTM_ROOT}/include $ENV{OPENRTM_ROOT}/include
    ${PC_COIL_INCLUDE_DIRS})
find_library(OPENRTM_LIBRARY RTC
    HINTS ${OPENRTM_ROOT}/lib $ENV{OPENRTM_ROOT}/lib
    ${PC_OPENRTM_LIBRARY_DIRS})
find_library(COIL_LIBRARY coil
    HINTS ${OPENRTM_ROOT}/lib $ENV{OPENRTM_ROOT}/lib
    ${PC_OPENRTM_LIBRARY_DIRS})

set(OPENRTM_DEFINITIONS ${PC_COIL_CFLAGS_OTHER} ${PC_OPENRTM_CFLAGS_OTHER})
set(OPENRTM_INCLUDE_DIRS ${COIL_INCLUDE_DIR} ${OPENRTM_INCLUDE_DIR} ${OPENRTM_INCLUDE_DIR}/rtm/idl)
set(OPENRTM_LIBRARIES ${OPENRTM_LIBRARY} ${COIL_LIBRARY} uuid dl pthread
    omniORB4 omnithread omniDynamic4)


#file(STRINGS ${OPENRTM_INCLUDE_DIR}/rtm/version.h OPENRTM_VERSION
#    NEWLINE_CONSUME)
set(OPENRTM_VERSION "1.1.0")
string(REGEX MATCH "version = \"([0-9]+)\\.([0-9]+)\\.([0-9]+)-?([a-zA-Z0-9]*)\""
    OPENRTM_VERSION "${OPENRTM_VERSION}")
set(OPENRTM_VERSION_MAJOR ${CMAKE_MATCH_1})
set(OPENRTM_VERSION_MINOR ${CMAKE_MATCH_2})
set(OPENRTM_VERSION_PATCH ${CMAKE_MATCH_3})
set(OPENRTM_VERSION_CAN ${CMAKE_MATCH_4})


execute_process(COMMAND rtm-config --idlc OUTPUT_VARIABLE OPENRTM_IDLC
    OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND rtm-config --idlflags OUTPUT_VARIABLE OPENRTM_IDLFLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE)
separate_arguments(OPENRTM_IDLFLAGS)
execute_process(COMMAND rtm-config --prefix OUTPUT_VARIABLE _rtm_prefix
    OUTPUT_STRIP_TRAILING_WHITESPACE)
set(OPENRTM_IDL_DIR
    ${_rtm_prefix}/include/openrtm-${OPENRTM_VERSION_MAJOR}.${OPENRTM_VERSION_MINOR}/rtm/idl
    CACHE STRING "Directory containing the OpenRTM-aist IDL files.")
set(OPENRTM_IDL_WRAPPER rtm-skelwrapper)
set(OPENRTM_IDL_WRAPPER_FLAGS --include-dir= --skel-suffix=Skel --stub-suffix=Stub)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenRTM
    REQUIRED_VARS OPENRTM_INCLUDE_DIR OPENRTM_LIBRARY COIL_LIBRARY
    VERSION_VAR ${OPENRTM_VERSION})


macro(OPENRTM_IDL_OUTPUTS _idl _dir _result)
    set(${_result} ${_dir}/${_idl}Skel.cpp ${_dir}/${_idl}Skel.h)
endmacro(OPENRTM_IDL_OUTPUTS)


macro(OPENRTM_COMPILE_IDL _idl_file)
    get_filename_component(_idl ${_idl_file} NAME_WE)
    set(_idl_srcs_var ${_idl}_SRCS)
    OPENRTM_IDL_OUTPUTS(${_idl} ${CMAKE_CURRENT_BINARY_DIR} ${_idl_srcs_var})

    add_custom_command(OUTPUT ${${_idl_srcs_var}}
        COMMAND ${OPENRTM_IDLC} ${OPENRTM_IDLFLAGS} -I${OPENRTM_IDL_DIR} ${_idl_file}
		COMMAND ${OPENRTM_IDL_WRAPPER} ${OEPNRTM_IDL_WRAPPER_FLAGS} --idl-file=${_idl_file}
        WORKING_DIRECTORY ${CURRENT_BINARY_DIR}
        DEPENDS ${_idl_file}
        COMMENT "Compiling ${_idl_file}" VERBATIM)
    add_custom_target(${_idl}_TGT DEPENDS ${${_idl_srcs_var}})
    set(ALL_IDL_SRCS ${ALL_IDL_SRCS} ${${_idl_srcs_var}})
    if(NOT TARGET ALL_IDL_TGT)
        add_custom_target(ALL_IDL_TGT)
    endif(NOT TARGET ALL_IDL_TGT)
    add_dependencies(ALL_IDL_TGT ${_idl}_TGT)
endmacro(OPENRTM_COMPILE_IDL)

# Module exposed to the user
macro(OPENRTM_COMPILE_IDL_FILES)
    foreach(idl ${ARGN})
        OPENRTM_COMPILE_IDL(${idl})
    endforeach(idl)
endmacro(OPENRTM_COMPILE_IDL_FILES)
