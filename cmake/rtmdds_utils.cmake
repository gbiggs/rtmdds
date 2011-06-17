macro(LIST_TO_STRING _string _list)
    set(${_string})
    foreach(_item ${_list})
        set(${_string} "${${_string}} ${_item}")
    endforeach(_item)
endmacro(LIST_TO_STRING)


macro(FILTER_LIST _list _pattern _output)
    set(${_output})
    foreach(_item ${_list})
        if("${_item}" MATCHES ${_pattern})
            set(${_output} ${${_output}} ${_item})
        endif("${_item}" MATCHES ${_pattern})
    endforeach(_item)
endmacro(FILTER_LIST)


###############################################################################
# This macro processes a list of arguments into separate lists based on
# keywords found in the argument stream. For example:
# BUILDBLAG (misc_arg INCLUDEDIRS /usr/include LIBDIRS /usr/local/lib
#            LINKFLAGS -lthatawesomelib CFLAGS -DUSEAWESOMELIB SOURCES blag.c)
# Any other args found at the start of the stream will go into the variable
# specified in _other_args. Typically, you would take arguments to your macro
# as normal, then pass ${ARGN} to this macro to parse the dynamic-length
# arguments (so if ${_otherArgs} comes back non-empty, you've ignored something
# or the user has passed in some arguments without a keyword).
macro(PROCESS_ARGUMENTS _sources_args _include_dirs_args _lib_dirs_args _link_libs_args _link_flags_args _cflags_args _idl_args _other_args)
    set(${_sources_args})
    set(${_include_dirs_args})
    set(${_lib_dirs_args})
    set(${_link_libs_args})
    set(${_link_flags_args})
    set(${_cflags_args})
    set(${_idl_args})
    set(${_other_args})
    set(_current_dest ${_other_args})
    foreach(_arg ${ARGN})
        if(_arg STREQUAL "SOURCES")
            set(_current_dest ${_sources_args})
        elseif(_arg STREQUAL "INCLUDEDIRS")
            set(_current_dest ${_include_dirs_args})
        elseif(_arg STREQUAL "LIBDIRS")
            set(_current_dest ${_lib_dirs_args})
        elseif(_arg STREQUAL "LINKLIBS")
            set(_current_dest ${_link_libs_args})
        elseif(_arg STREQUAL "LINKFLAGS")
            set(_current_dest ${_link_flags_args})
        elseif(_arg STREQUAL "CFLAGS")
            set(_current_dest ${_cflags_args})
        elseif(_arg STREQUAL "IDL")
            set(_current_dest ${_idl_args})
        else(_arg STREQUAL "SOURCES")
            list(APPEND ${_current_dest} ${_arg})
        endif(_arg STREQUAL "SOURCES")
    endforeach(_arg)
endmacro(PROCESS_ARGUMENTS)


include(FindPkgConfig)
macro(GET_PKG_CONFIG_INFO _pkg _required)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(${_pkg}_PKG ${_required} ${_pkg})
        if(${_pkg}_PKG_CFLAGS_OTHER)
            LIST_TO_STRING(${_pkg}_CFLAGS "${${_pkg}_PKG_CFLAGS_OTHER}")
        else(${_pkg}_PKG_CFLAGS_OTHER)
            set(${_pkg}_CFLAGS "")
        endif(${_pkg}_PKG_CFLAGS_OTHER)
        set(${_pkg}_INCLUDE_DIRS ${${_pkg}_PKG_INCLUDE_DIRS})
        set(${_pkg}_LINK_LIBS ${${_pkg}_PKG_LIBRARIES})
        set(${_pkg}_LIBRARY_DIRS ${${_pkg}_PKG_LIBRARY_DIRS})
        if(${_pkg}_PKG_LDFLAGS_OTHER)
            LIST_TO_STRING(${_pkg}_LINK_FLAGS ${${_pkg}_PKG_LDFLAGS_OTHER})
        else(${_pkg}_PKG_LDFLAGS_OTHER)
            set(${_pkg}_LINK_FLAGS "")
        endif(${_pkg}_PKG_LDFLAGS_OTHER)
    else(PKG_CONFIG_FOUND)
        message(STATUS "Could not find pkg-config.")
        message(STATUS
            "You will need to set the following variables manually:")
        message(STATUS "${_pkg}_INCLUDE_DIRS ${_pkg}_CFLAGS_OTHER ${_pkg}_LINK_LIBS ${_pkg}_LIBRARY_DIRS ${_pkg}_LINK_FLAGS")
    endif(PKG_CONFIG_FOUND)
endmacro(GET_PKG_CONFIG_INFO)


macro(APPLY_PKG_CONFIG_DIRS _pkg)
    if(${_pkg}_INCLUDE_DIRS)
        include_directories(${${_pkg}_INCLUDE_DIRS})
    endif(${_pkg}_INCLUDE_DIRS)
    if(${_pkg}_LIBRARY_DIRS)
        link_directories(${${_pkg}_LIBRARY_DIRS})
    endif(${_pkg}_LIBRARY_DIRS)
endmacro(APPLY_PKG_CONFIG_DIRS)


macro(APPLY_PKG_CONFIG_TO_TGTS _pkg)
    if(${_pkg}_LINK_FLAGS)
        foreach(_tgt ${ARGN})
            set_target_properties(${_tgt} PROPERTIES
                LINK_FLAGS "${${_pkg}_LINK_FLAGS}")
        endforeach(_tgt)
    endif(${_pkg}_LINK_FLAGS)
    if(${_pkg}_LINK_LIBS)
        foreach(_tgt ${ARGN})
            target_link_libraries(${_tgt} ${${_pkg}_LINK_LIBS})
        endforeach(_tgt)
    endif(${_pkg}_LINK_LIBS)
endmacro(APPLY_PKG_CONFIG_TO_TGTS)


macro(APPLY_PKG_CONFIG_TO_SRCS _pkg)
    if(${_pkg}_CFLAGS)
        set_source_files_properties(${ARGN}
            PROPERTIES COMPILE_FLAGS "${${_pkg}_CFLAGS}")
    endif(${_pkg}_CFLAGS)
endmacro(APPLY_PKG_CONFIG_TO_SRCS)


macro(GET_OS_INFO)
    string(REGEX MATCH "Linux" OS_IS_LINUX ${CMAKE_SYSTEM_NAME})
    if(OS_IS_LINUX)
        if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
            set(LIB_INSTALL_DIR "lib64")
        else(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
            set(LIB_INSTALL_DIR "lib")
        endif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    else(OS_IS_LINUX)
        set(LIB_INSTALL_DIR "lib")
    endif(OS_IS_LINUX)
    set(INCLUDE_INSTALL_DIR "include/${PROJECT_NAME_LOWER}")
    set(BIN_INSTALL_DIR "bin")
    set(SHARE_INSTALL_DIR "share/${PROJECT_NAME_LOWER}")
endmacro(GET_OS_INFO)


macro(DISSECT_VERSION)
    # Find version components
    string(REGEX REPLACE "^([0-9]+).*" "\\1"
        RTMDDS_VERSION_MAJOR "${RTMDDS_VERSION}")
    string(REGEX REPLACE "^[0-9]+\\.([0-9]+).*" "\\1"
        RTMDDS_VERSION_MINOR "${RTMDDS_VERSION}")
    string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1"
        RTMDDS_VERSION_PATCH ${RTMDDS_VERSION})
    string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.[0-9]+(.*)" "\\1"
        RTMDDS_VERSION_CAN ${RTMDDS_VERSION})
endmacro(DISSECT_VERSION)


macro(IDL_OUTPUTS _idl _dir _result)
    set(${_result} ${_dir}/${_idl}.cxx ${_dir}/${_idl}.h
        ${_dir}/${_idl}Plugin.cxx ${_dir}/${_idl}Plugin.h
        ${_dir}/${_idl}Support.cxx ${_dir}/${_idl}Support.h)
endmacro(IDL_OUTPUTS)


macro(COMPILE_IDL _idl_file)
    get_filename_component(_idl ${_idl_file} NAME_WE)
    set(_idl_srcs_var ${_idl}_SRCS)
    IDL_OUTPUTS(${_idl} ${CMAKE_CURRENT_BINARY_DIR} ${_idl_srcs_var})
    add_custom_command(OUTPUT ${${_idl_srcs_var}}
        COMMAND rtiddsgen -language C++ -d ${CMAKE_CURRENT_BINARY_DIR}
        ${_idl_file} WORKING_DIRECTORY ${CURRENT_BINARY_DIR}
        DEPENDS ${_idl_file}
        COMMENT "Compiling ${_idl_file}" VERBATIM)
    set(ALL_IDL_SRCS ${_idl_srcs} ${${_idl_srcs_var}})
endmacro(COMPILE_IDL)


macro(COMPILE_IDL_FILES)
    set(idl_srcs)
    foreach(idl ${ARGN})
        COMPILE_IDL(${idl})
    endforeach(idl)
endmacro(COMPILE_IDL_FILES)

