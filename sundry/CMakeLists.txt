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

# TODO Install the two streaming utilities.

