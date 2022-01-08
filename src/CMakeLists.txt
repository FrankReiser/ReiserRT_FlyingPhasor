# Public Header files
set( _publicHeaders
        FlyingPhasorToneGenerator.h
        )

# Private Header files (none until otherwise determined)
set( _privateHeaders
        ""
        )

# Private Source files.
set( _privateSources
        FlyingPhasorToneGenerator.cpp
        )

add_library( TSG_ComplexToneGen SHARED "" )
target_sources( TSG_ComplexToneGen
        PRIVATE
        ${_privateSources}
        #            ${_privateHeaders}
        #        PUBLIC
        #            ${_publicHeaders}
        )

set_target_properties( TSG_ComplexToneGen
        PROPERTIES
        POSITION_INDEPENDENT_CODE 1
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN 1
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        DEBUG_POSTFIX "_d"
        PUBLIC_HEADER "${_publicHeaders};${CMAKE_BINARY_DIR}/${INSTALL_INCLUDEDIR}/TSG_ComplexToneGenExport.h"
        )

# Specify our target interfaces for external clients
target_include_directories( TSG_ComplexToneGen
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_BINARY_DIR}/${INSTALL_INCLUDEDIR}>"
        "$<INSTALL_INTERFACE:${INSTALL_INCLUDEDIR}>"
        )

include(GenerateExportHeader)
generate_export_header( TSG_ComplexToneGen
        BASE_NAME "TSG_ComplexToneGen"
        EXPORT_MACRO_NAME "TSG_ComplexToneGen_EXPORT"
        EXPORT_FILE_NAME "${CMAKE_BINARY_DIR}/${INSTALL_INCLUDEDIR}/TSG_ComplexToneGenExport.h"
        DEPRECATED_MACRO_NAME "TSG_ComplexToneGen_DEPRECATED"
        NO_EXPORT_MACRO_NAME "TSG_ComplexToneGen_NO_EXPORT"
        STATIC_DEFINE "TSG_ComplexToneGen_STATIC_DEFINE"
        NO_DEPRECATED_MACRO_NAME "TSG_ComplexToneGen_NO_DEPRECATED"
        DEFINE_NO_DEPRECATED
        )

install(
        TARGETS
        TSG_ComplexToneGen
        EXPORT
        TSG_ComplexToneGenTargets
        ARCHIVE
        DESTINATION ${INSTALL_LIBDIR}
        COMPONENT lib
        RUNTIME
        DESTINATION ${INSTALL_BINDIR}
        COMPONENT bin
        LIBRARY
        DESTINATION ${INSTALL_LIBDIR}
        COMPONENT lib
        PUBLIC_HEADER
        DESTINATION "${INSTALL_INCLUDEDIR}"
        COMPONENT dev
)

install (
        EXPORT
        TSG_ComplexToneGenTargets
        NAMESPACE
        "TSG_ComplexToneGen::"
        DESTINATION
        ${INSTALL_CMAKEDIR}
        COMPONENT
        dev
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/TSG_ComplexToneGenConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)
configure_package_config_file(
        ${PROJECT_SOURCE_DIR}/cmake/TSG_ComplexToneGenConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/TSG_ComplexToneGenConfig.cmake
        INSTALL_DESTINATION ${INSTALL_CMAKEDIR}
)

install(
        FILES
        ${CMAKE_CURRENT_BINARY_DIR}/TSG_ComplexToneGenConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/TSG_ComplexToneGenVersion.cmake
        DESTINATION
        ${INSTALL_CMAKEDIR}
)
