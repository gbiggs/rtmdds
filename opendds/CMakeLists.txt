cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(rtmdds)
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)
include(${PROJECT_SOURCE_DIR}/cmake/rtmdds_utils.cmake)

# Set version and OS-specific settings
set(RTMDDS_VERSION 1.0.0-RC1 CACHE STRING "RTM DDS version")
DISSECT_VERSION()
GET_OS_INFO()

if(WIN32)
    set(RTMDDS_DLLVERSION
        "${RTMDDS_VERSION_MAJOR}${RTMDDS_VERSION_MINOR}${RTMDDS_VERSION_PATCH}")
    set(RTMDDS_DEBUG_POSTFIX d)
else(WIN32)
    set(RTMDDS_DLLVERSION)
    set(RTMDDS_DEBUG_POSTFIX)
endif(WIN32)

# Add an "uninstall" target
configure_file("${PROJECT_SOURCE_DIR}/cmake/uninstall_target.cmake.in"
    "${PROJECT_BINARY_DIR}/uninstall_target.cmake" IMMEDIATE @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}" -P
    "${PROJECT_BINARY_DIR}/uninstall_target.cmake")

# Get necessary dependency information
find_package(OpenRTM 1.1 REQUIRED)
find_package(DDS 3.5 REQUIRED)

# Subdirectories
add_subdirectory(cmake)
add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(examples)

# Package creation
include(InstallRequiredSystemLibraries)
set(PKG_DESCRIPTION "DDS transport for OpenRTM-aist")
set(PKG_VENDOR "Geoffrey Biggs, AIST")
configure_file("${PROJECT_SOURCE_DIR}/cmake/cpack_options.cmake.in"
    "${PROJECT_BINARY_DIR}/cpack_options.cmake" @ONLY)
set(CPACK_PROJECT_CONFIG_FILE "${PROJECT_BINARY_DIR}/cpack_options.cmake")
include(CPack)

