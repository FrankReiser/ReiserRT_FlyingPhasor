add_executable( streamLegacyGen "" )
target_sources( streamLegacyGen PRIVATE streamLegacyGen.cpp )
target_include_directories( streamLegacyGen PUBLIC ../src )
target_link_libraries( streamLegacyGen TSG_FlyingPhasorToneGen )

add_executable( streamFlyingPhasorGen "" )
target_sources( streamFlyingPhasorGen PRIVATE streamFlyingPhasorGen.cpp )
target_include_directories( streamFlyingPhasorGen PUBLIC ../src )
target_link_libraries( streamFlyingPhasorGen TSG_FlyingPhasorToneGen )
