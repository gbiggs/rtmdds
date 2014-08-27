set(PKG_DESC "DDS transport for OpenRTM-aist")
set(PKG_EXTERNAL_DEPS "openrtm-aist")
set(PKG_DDS_LIBS)
foreach(lib ${DDS_LIBRARIES})
    set(PKG_DDS_LIBS "-l${lib} ${PKG_DDS_LIBS}")
endforeach(lib)
set(PKG_DDS_INC_DIRS)
foreach(incdir ${DDS_INCLUDE_DIRS})
    set(PKG_DDS_INC_DIRS "-I${incdir} ${PKG_DDS_INC_DIRS}")
endforeach(incdir)
set(pkg_conf_file ${CMAKE_CURRENT_BINARY_DIR}/rtmdds.pc)
configure_file(rtmdds.pc.in ${pkg_conf_file} @ONLY)
install(FILES ${pkg_conf_file}
    DESTINATION ${LIB_INSTALL_DIR}/pkgconfig/ COMPONENT build)

# Install CMake modules
set(cmake_config ${CMAKE_CURRENT_BINARY_DIR}/rtmdds-config.cmake)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/rtmdds-config.cmake.in
    ${cmake_config} @ONLY)
set(cmake_version_config ${CMAKE_CURRENT_BINARY_DIR}/rtmdds-config-version.cmake)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/rtmdds-config-version.cmake.in
    ${cmake_version_config} @ONLY)
set(cmake_mods ${CMAKE_CURRENT_SOURCE_DIR}/Modules/FindOpenRTM.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/Modules/FindDDS.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/Findrtmdds.cmake
    ${cmake_config} ${cmake_version_config})
install(FILES ${cmake_mods} DESTINATION ${SHARE_INSTALL_DIR} COMPONENT build)

