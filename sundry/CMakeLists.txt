add_executable( streamLegacyGen "" )
target_sources( streamLegacyGen PRIVATE streamLegacyGen.cpp )
target_include_directories( streamLegacyGen PUBLIC ../src ../testUtilities )
target_link_libraries( streamLegacyGen ReiserRT_FlyingPhasor TestUtilities )

add_executable( streamFlyingPhasorGen "" )
target_sources( streamFlyingPhasorGen PRIVATE streamFlyingPhasorGen.cpp )
target_include_directories( streamFlyingPhasorGen PUBLIC ../src ../testUtilities )
target_link_libraries( streamFlyingPhasorGen ReiserRT_FlyingPhasor TestUtilities )

add_executable( twoToneTest "" )
target_sources( twoToneTest PRIVATE twoToneTest.cpp)
target_include_directories( twoToneTest PUBLIC ../src ../testUtilities )
target_link_libraries( twoToneTest ReiserRT_FlyingPhasor )
