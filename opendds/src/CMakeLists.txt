# cmake_minimum_required(VERSION 2.8)
# list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
# find_package(rtmdds REQUIRED)
include(${PROJECT_SOURCE_DIR}/cmake/rtmdds_utils.cmake)

set(srcs ddsportmgmt.cpp
    ddssubportbase.cpp
    ddspubportbase.cpp)

include_directories(${OPENRTM_INCLUDE_DIRS} ${DDS_INCLUDE_DIRS})
add_definitions(${OPENRTM_DEFINITIONS} ${DDS_DEFINITIONS} -DACE_HAS_VERSIONED_NAMESPACE=1)

# RTMDDS
set(lib_name rtmdds)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/include)
add_library(${lib_name} SHARED ${srcs})
target_link_libraries(${lib_name} ${OPENRTM_LIBRARIES} ddswrapperforrtm)
install(TARGETS ${lib_name} LIBRARY DESTINATION
    ${LIB_INSTALL_DIR} COMPONENT library)

# DDS Wrapper
set(srcs_dds dcpsFuncPubImpl.cpp dcpsFuncPubWrapper.cpp
  dcpsFuncSubImpl.cpp dcpsFuncSubWrapper.cpp)
set(lib_dds_name ddswrapperforrtm)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/include)

# Compile the IDL files used in the DDS Function
set(idls ${CMAKE_CURRENT_SOURCE_DIR}/dds_datatype.idl)
COMPILE_IDL_FILES(${idls})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(${lib_dds_name} SHARED ${srcs_dds} ${ALL_IDL_SRCS})
target_link_libraries(${lib_dds_name} ${DDS_LIBRARIES})
install(TARGETS ${lib_dds_name} LIBRARY DESTINATION ${LIB_INSTALL_DIR} COMPONENT library)
