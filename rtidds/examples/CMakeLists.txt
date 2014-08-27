# Build the sample components
set(subcomp_srcs sub_rtc.cpp)
set(subcomp_standalone_srcs sub_standalone.cpp)
set(subcomp_hdrs sub_rtc.h)
set(pubcomp_srcs pub_rtc.cpp)
set(pubcomp_standalone_srcs pub_standalone.cpp)
set(pubcomp_hdrs pub_rtc.h)

include_directories(${OPENRTM_INCLUDE_DIRS} ${DDS_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include)
add_definitions(${OPENRTM_DEFINITIONS} ${DDS_DEFINITIONS})

set(idls ${CMAKE_CURRENT_SOURCE_DIR}/test.idl)
COMPILE_IDL_FILES(${idls})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(rtmdds_subcomp SHARED
    ${subcomp_srcs} ${ALL_IDL_SRCS})
add_executable(rtmdds_subcomp_standalone ${subcomp_standalone_srcs})
target_link_libraries(rtmdds_subcomp ${OPENRTM_LIBRARIES} ${DDS_LIBRARIES}
    rtmdds)
target_link_libraries(rtmdds_subcomp_standalone rtmdds_subcomp)

add_library(rtmdds_pubcomp SHARED
    ${pubcomp_srcs} ${ALL_IDL_SRCS})
add_executable(rtmdds_pubcomp_standalone ${pubcomp_standalone_srcs})
target_link_libraries(rtmdds_pubcomp ${OPENRTM_LIBRARIES} ${DDS_LIBARIES}
    rtmdds)
target_link_libraries(rtmdds_pubcomp_standalone rtmdds_pubcomp)

# Install the compiled sample components
install(TARGETS rtmdds_subcomp rtmdds_subcomp_standalone
    rtmdds_pubcomp rtmdds_pubcomp_standalone
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
    COMPONENT examples)

# Install the sample files
set(eg_cmakelists ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt.example.in
    ${eg_cmakelists} @ONLY)
install(FILES ${eg_cmakelists} DESTINATION ${SHARE_INSTALL_DIR}/examples
    COMPONENT examples)
install(FILES ${subcomp_srcs} ${subcomp_standalone_srcs} ${subcomp_hdrs}
    ${pubcomp_srcs} ${pubcomp_standalone_srcs} ${pubcomp_hdrs}
    ${idls} README.txt rtc.conf pubcomp.conf subcomp.conf
    DESTINATION ${SHARE_INSTALL_DIR}/examples
    COMPONENT examples)

