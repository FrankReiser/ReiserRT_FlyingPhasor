add_executable( testInitialization "" )
target_sources( testInitialization PRIVATE testInitialization.cpp )
target_include_directories( testInitialization PUBLIC ../src ../testUtilities )
target_link_libraries( testInitialization ReiserRT_FlyingPhasor TestUtilities )
target_compile_options( testInitialization PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)
add_test( NAME runInitializationTest COMMAND $<TARGET_FILE:testInitialization> )

add_executable( testPurity "" )
target_sources( testPurity PRIVATE testPurity.cpp)
target_include_directories( testPurity PUBLIC ../src ../testUtilities )
target_link_libraries( testPurity ReiserRT_FlyingPhasor TestUtilities )
target_compile_options( testPurity PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)
add_test( NAME runPurityTest1 COMMAND $<TARGET_FILE:testPurity> --radsPerSample=2.0 --phase=-1.5 )
add_test( NAME runPurityTest2 COMMAND $<TARGET_FILE:testPurity> --radsPerSample=-2.0 --phase=1.5 )
add_test( NAME runPurityTest3 COMMAND $<TARGET_FILE:testPurity> --radsPerSample=0.25 --phase=0.0 )
add_test( NAME runPurityTest4 COMMAND $<TARGET_FILE:testPurity> --radsPerSample=0.001 --phase=0.0 )

add_executable( testScalingAndAccumulating "" )
target_sources( testScalingAndAccumulating PRIVATE testScalingAndAccumulating.cpp)
target_include_directories( testScalingAndAccumulating PUBLIC ../src )
target_link_libraries( testScalingAndAccumulating ReiserRT_FlyingPhasor )
target_compile_options( testScalingAndAccumulating PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)
add_test( NAME runScalingAndAccumulatingTest COMMAND $<TARGET_FILE:testScalingAndAccumulating> )
