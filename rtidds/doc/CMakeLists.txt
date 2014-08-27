# English
if(EXISTS manual.pdf)
    install(FILES manual.pdf
        DESTINATION
        share/doc/${PROJECT_NAME_LOWER}-${RTMDDS_VERSION_MAJOR}.${RTMDDS_VERSION_MINOR}
        COMPONENT doc)
endif(EXISTS manual.pdf)
# Japanese
if(EXISTS manual_jp.pdf)
    install(FILES manual_jp.pdf
        DESTINATION
        share/doc/${PROJECT_NAME_LOWER}-${RTMDDS_VERSION_MAJOR}.${RTMDDS_VERSION_MINOR}
        COMPONENT doc)
endif(EXISTS manual_jp.pdf)

