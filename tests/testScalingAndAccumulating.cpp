/**
 * @file testScalingAndAccumulating.cpp
 * @brief Test Scaling And Accumulating Functionality
 *
 * @authors Frank Reiser
 * @date Initiated on Nov 26, 2023
 */

#include "FlyingPhasorToneGenerator.h"

#include <iostream>
#include <memory>


using namespace ReiserRT::Signal;

int runScalingWithConstantTest()
{
    // This test assumes the basic functionality has been well tested as it will rely on that
    // as a basis for comparison.
    constexpr size_t NUM_SAMPLES = 1024;

    // Create some buffers for comparing test results to gold standard.
    std::unique_ptr< FlyingPhasorElementType[] > goldenElementBuf{new FlyingPhasorElementType[NUM_SAMPLES] };
    std::unique_ptr< FlyingPhasorElementType[] > testElementBuf{new FlyingPhasorElementType[NUM_SAMPLES] };
    std::unique_ptr< FlyingPhasorElementType[] > scratchElementBuf{new FlyingPhasorElementType[NUM_SAMPLES] };

    // Both the Gold Standard and the Test will utilize FlyingPhasor instances. We will just use them differently.
    FlyingPhasorToneGenerator goldenGen{ 1.0  };
    FlyingPhasorToneGenerator testGen{ 1.0 };

    // We are going to use a fixed magnitude of 3 for the first samples we retrieve to test
    // the getSamplesScaled operation that takes a const scalar value. We will build up
    // our golden buffer by fetching unscaled samples and manually scaling them.
    constexpr double toneOneMag = 3.0;  // Something other than default of 1.
    goldenGen.getSamples( goldenElementBuf.get(), NUM_SAMPLES );
    for ( size_t i = 0; NUM_SAMPLES != i; ++i )
        goldenElementBuf[i] *= toneOneMag;

    // Invoke getSamplesScaled on our generator under test using the same scalar value as golden buffer.
    testGen.getSamplesScaled( testElementBuf.get(), NUM_SAMPLES, toneOneMag );

    // The Samples should compare 100%
    for ( size_t i = 0; NUM_SAMPLES != i; ++i )
    {
        if ( goldenElementBuf[i] != testElementBuf[i] )
        {
            std::cout << "Failed getSamplesScaled with fixed scalar test at index " << i << std::endl;
            return 1;
        }
    }

    // If we made it here, we want to test accumSamplesScaled with a fixed scalar.
    // We also want to use a different tone to accumulate and a different scalar.
    // We will be making temporary use of our testElementBuffer to assist us in
    // building up our golden tone
    constexpr double toneTwoMag = 2.0;  // Something other than default of 1.
    goldenGen.reset( 2.0 );
    goldenGen.getSamplesScaled( scratchElementBuf.get(), NUM_SAMPLES, toneTwoMag );
    for ( size_t i = 0; NUM_SAMPLES != i; ++i )
        goldenElementBuf[i] += scratchElementBuf[i];


    // Now test the accumSamplesScaled with the second scalar value.
    testGen.reset( 2.0 );
    testGen.accumSamplesScaled( testElementBuf.get(), NUM_SAMPLES, toneTwoMag );
    for ( size_t i = 0; NUM_SAMPLES != i; ++i )
    {
        if ( goldenElementBuf[i] != testElementBuf[i] )
        {
            std::cout << "Failed accumSamplesScaled with fixed scalar test at index " << i << std::endl;
            return 2;
        }
    }

    return 0;
}

int runScalingWithVectorTest()
{
    // This test assumes the basic functionality has been well tested as it will rely on that
    // as a basis for comparison.
    constexpr size_t NUM_SAMPLES = 1024;

    // Create some buffers for comparing test results to gold standard.
    std::unique_ptr< FlyingPhasorElementType[] > goldenElementBuf{new FlyingPhasorElementType[NUM_SAMPLES] };
    std::unique_ptr< FlyingPhasorElementType[] > testElementBuf{new FlyingPhasorElementType[NUM_SAMPLES] };
    std::unique_ptr< FlyingPhasorElementType[] > scratchElementBuf{new FlyingPhasorElementType[NUM_SAMPLES] };

    // Create some buffers for storing magnitude vectors
    std::unique_ptr< FlyingPhasorPrecisionType[] > toneOneMagBuf{new FlyingPhasorPrecisionType[NUM_SAMPLES] };
    std::unique_ptr< FlyingPhasorPrecisionType[] > toneTwoMagBuf{new FlyingPhasorPrecisionType[NUM_SAMPLES] };

    // Nothing fancy for envelopes we just want something non-constant and two distinct envelopes.
    for ( size_t i = 0; NUM_SAMPLES != i; ++i )
    {
        toneOneMagBuf[i] = 3.0 - double(i) / NUM_SAMPLES;
        toneTwoMagBuf[i] = 1.5 - double(i) / NUM_SAMPLES;
    }

    // Both the Gold Standard and the Test will utilize FlyingPhasor instances. We will just use them differently.
    FlyingPhasorToneGenerator goldenGen{ 1.0  };
    FlyingPhasorToneGenerator testGen{ 1.0 };

    // We are going to use toneOneMagBuf for the first samples we retrieve to test
    // the getSamplesScaled operation that takes a vector of scalars. We will build up
    // our golden buffer by fetching unscaled samples and manually scaling them.
    goldenGen.getSamples( goldenElementBuf.get(), NUM_SAMPLES );
    for ( size_t i = 0; NUM_SAMPLES != i; ++i )
        goldenElementBuf[i] *= toneOneMagBuf[i];

    // Invoke getSamplesScaled on our generator under test using the same scalar values as golden buffer.
    testGen.getSamplesScaled( testElementBuf.get(), NUM_SAMPLES, toneOneMagBuf.get() );

    // The Samples should compare 100%
    for ( size_t i = 0; NUM_SAMPLES != i; ++i )
    {
        if ( goldenElementBuf[i] != testElementBuf[i] )
        {
            std::cout << "Failed getSamplesScaled with vector of scalars test at index " << i << std::endl;
            return 11;
        }
    }

    // If we made it here, we want to test accumSamplesScaled with a vector of scalars.
    // We also want to use a different tone to accumulate and a different vector of scalars.
    // We will be making temporary use of our testElementBuffer to assist us in
    // building up our golden tone
    goldenGen.reset( 2.0 );
    goldenGen.getSamplesScaled( scratchElementBuf.get(), NUM_SAMPLES, toneTwoMagBuf.get() );
    for ( size_t i = 0; NUM_SAMPLES != i; ++i )
        goldenElementBuf[i] += scratchElementBuf[i];

    // Now test the accumSamplesScaled with the second scalar value.
    testGen.reset( 2.0 );
    testGen.accumSamplesScaled( testElementBuf.get(), NUM_SAMPLES, toneTwoMagBuf.get() );
    for ( size_t i = 0; NUM_SAMPLES != i; ++i )
    {
        if ( goldenElementBuf[i] != testElementBuf[i] )
        {
            std::cout << "Failed accumSamplesScaled with vector of scalars test at index " << i << std::endl;
            return 12;
        }
    }

    return 0;
}

int main()
{
    int retCode = 0;

    do
    {
        // Run Scaling with a constant test.
        // This tests both getting and accumulating while scaling with constant envelope.
        retCode = runScalingWithConstantTest();
        if ( 0 != retCode )
            break;

        // Run Scaling with a vector test.
        // This tests both getting and accumulating while scaling with variable envelope.
        retCode = runScalingWithVectorTest();
        if ( 0 != retCode )
            break;

    } while (false);

    exit( retCode );
    return retCode;
}
