add_executable( streamLegacyPhasorGen "" )
target_sources( streamLegacyPhasorGen PRIVATE streamLegacyPhasorGen.cpp)
target_include_directories( streamLegacyPhasorGen PUBLIC ../src ../testUtilities )
target_link_libraries( streamLegacyPhasorGen ReiserRT_FlyingPhasor TestUtilities )
target_compile_options( streamLegacyPhasorGen PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

add_executable( streamFlyingPhasorGen "" )
target_sources( streamFlyingPhasorGen PRIVATE streamFlyingPhasorGen.cpp )
target_include_directories( streamFlyingPhasorGen PUBLIC ../src ../testUtilities )
target_link_libraries( streamFlyingPhasorGen ReiserRT_FlyingPhasor TestUtilities )
target_compile_options( streamFlyingPhasorGen PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

add_executable( twoToneTest "" )
target_sources( twoToneTest PRIVATE twoToneTest.cpp)
target_include_directories( twoToneTest PUBLIC ../src ../testUtilities )
target_link_libraries( twoToneTest ReiserRT_FlyingPhasor )
target_compile_options( twoToneTest PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

# What this does is set up a relative path where we expect our custom libraries to be
# It will be used to patch the installation to find libraries relative to the binary.
file( RELATIVE_PATH _rel ${CMAKE_INSTALL_PREFIX}/${INSTALL_BINDIR} ${CMAKE_INSTALL_PREFIX})
set( _rpath "\$ORIGIN/${_rel}" )
file( TO_NATIVE_PATH "${_rpath}/${INSTALL_LIBDIR}" MY_LIBRARY_RPATH )
message( STATUS "MY_LIBRARY_RPATH: ${MY_LIBRARY_RPATH}" )

# Install streamFlyingPhasorGen and streamLegacyPhasorGen with RPATH setting
set_target_properties( streamFlyingPhasorGen
        PROPERTIES
        SKIP_BUILD_RPATH OFF
        BUILD_WITH_INSTALL_RPATH OFF
        INSTALL_RPATH "${MY_LIBRARY_RPATH}"
        INSTALL_RPATH_USE_LINK_PATH ON
)
set_target_properties( streamLegacyPhasorGen
        PROPERTIES
        SKIP_BUILD_RPATH OFF
        BUILD_WITH_INSTALL_RPATH OFF
        INSTALL_RPATH "${MY_LIBRARY_RPATH}"
        INSTALL_RPATH_USE_LINK_PATH ON
)

# Installation of our binary executable component.
install(
        TARGETS streamFlyingPhasorGen streamLegacyPhasorGen
        RUNTIME DESTINATION ${INSTALL_BINDIR} COMPONENT bin
)

