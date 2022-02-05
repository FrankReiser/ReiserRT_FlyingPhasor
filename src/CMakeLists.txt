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

add_library( ReiserRT_FlyingPhasor SHARED "" )
target_sources( ReiserRT_FlyingPhasor
        PRIVATE
        ${_privateSources}
        #            ${_privateHeaders}
        #        PUBLIC
        #            ${_publicHeaders}
        )

set_target_properties( ReiserRT_FlyingPhasor
        PROPERTIES
        POSITION_INDEPENDENT_CODE 1
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN 1
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        DEBUG_POSTFIX "_d"
        PUBLIC_HEADER "${_publicHeaders};${CMAKE_BINARY_DIR}/${INSTALL_INCLUDEDIR}/ReiserRT_FlyingPhasorExport.h"
        )

# Specify our target interfaces for external clients
target_include_directories( ReiserRT_FlyingPhasor
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_BINARY_DIR}/${INSTALL_INCLUDEDIR}>"
        "$<INSTALL_INTERFACE:${INSTALL_INCLUDEDIR}>"
        )

include(GenerateExportHeader)
generate_export_header( ReiserRT_FlyingPhasor
        BASE_NAME "ReiserRT_FlyingPhasor"
        EXPORT_MACRO_NAME "ReiserRT_FlyingPhasor_EXPORT"
        EXPORT_FILE_NAME "${CMAKE_BINARY_DIR}/${INSTALL_INCLUDEDIR}/ReiserRT_FlyingPhasorExport.h"
        DEPRECATED_MACRO_NAME "ReiserRT_FlyingPhasor_DEPRECATED"
        NO_EXPORT_MACRO_NAME "ReiserRT_FlyingPhasor_NO_EXPORT"
        STATIC_DEFINE "ReiserRT_FlyingPhasor_STATIC_DEFINE"
        NO_DEPRECATED_MACRO_NAME "ReiserRT_FlyingPhasor_NO_DEPRECATED"
        DEFINE_NO_DEPRECATED
        )

install(
        TARGETS
        ReiserRT_FlyingPhasor
        EXPORT
        ReiserRT_FlyingPhasorTargets
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
        ReiserRT_FlyingPhasorTargets
        NAMESPACE
        "ReiserRT_FlyingPhasor::"
        DESTINATION
        ${INSTALL_CMAKEDIR}
        COMPONENT
        dev
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/ReiserRT_FlyingPhasorConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)
configure_package_config_file(
        ${PROJECT_SOURCE_DIR}/cmake/ReiserRT_FlyingPhasorConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/ReiserRT_FlyingPhasorConfig.cmake
        INSTALL_DESTINATION ${INSTALL_CMAKEDIR}
)

install(
        FILES
        ${CMAKE_CURRENT_BINARY_DIR}/ReiserRT_FlyingPhasorConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/ReiserRT_FlyingPhasorConfigVersion.cmake
        DESTINATION
        ${INSTALL_CMAKEDIR}
)
