set(hdrs rtmdds/ddsportmgmt.h
    rtmdds/ddssubportbase.h
    rtmdds/ddssubport.h
    rtmdds/ddspubportbase.h
    rtmdds/ddspubport.h)
install(FILES ${hdrs} DESTINATION ${INCLUDE_INSTALL_DIR}
    COMPONENT headers)

