/**
 * @file testInitialization.cpp
 * @brief Initializes Tone Generator and Performs Basic Tests
 *
 * @authors Frank Reiser
 * @date Initiated on Jan 08, 2022
 */

#include "FlyingPhasorToneGenerator.h"

#include "MiscTestUtilities.h"

#include <iostream>
#include <memory>

using namespace ReiserRT::Signal;

int main()
{
    int retCode = 0;
    std::cout << "Initialization Testing of TSG Flying Phasor Tone Generator" << std::endl;

    // For maximum view of significant digits for diagnostic purposes.
    std::cout << std::scientific;
    std::cout.precision(17);

    do {
        // A small buffer for storing two elements
        std::unique_ptr< FlyingPhasorElementType[] > pElementBuf{new FlyingPhasorElementType [2] };

        // Instantiate Default Complex Tone Generator (Defaults => 0.0 radsPerSample, 0.0 phi = pure DC)
        {
            std::unique_ptr< FlyingPhasorToneGenerator > pFlyingPhasorToneGen{ new FlyingPhasorToneGenerator{ } };

            // Fetch 2 samples.
            pFlyingPhasorToneGen->getSamples( pElementBuf.get(), 2 );

            // The first sample should have a mag of one and a phase of zero.
            {
                auto phase = std::arg( pElementBuf[0] );
                if ( !inTolerance( phase, 0.0, 1e-12 ) )
                {
                    std::cout << "First Sample Phase: " <<  phase << " out of Tolerance! Should be: "
                              << 0.0 << std::endl;
                    retCode = 1;
                    break;
                }
                auto mag = std::abs( pElementBuf[0] );
                if ( !inTolerance( mag, 1.0, 1e-12 ) )
                {
                    std::cout << "First Sample Magnitude: " <<  mag << " out of Tolerance! Should be: "
                              << 1.0 << std::endl;
                    retCode = 2;
                    break;
                }
            }

            // The second sample should also have a mag of one and a phase of zero (pure DC).
            {
                auto phase = std::arg( pElementBuf[1] );
                if ( !inTolerance( phase, 0.0, 1e-12 ) )
                {
                    std::cout << "Second Sample Phase: " <<  phase << " out of Tolerance! Should be: "
                              << 0.0 << std::endl;
                    retCode = 3;
                    break;
                }
                auto mag = std::abs( pElementBuf[1] );
                if ( !inTolerance( mag, 1.0, 1e-12 ) )
                {
                    std::cout << "Second Sample Magnitude: " <<  mag << " out of Tolerance! Should be: "
                              << 1.0 << std::endl;
                    retCode = 4;
                    break;
                }
            }

            // Attempt a reset to something else and test again.
            pFlyingPhasorToneGen->reset( 1.0, 1.0 );
            pFlyingPhasorToneGen->getSamples( pElementBuf.get(), 2 );

            // The first sample should have a mag of one and a phase of 1.0.
            {
                auto phase = std::arg( pElementBuf[0] );
                if ( !inTolerance( phase, 1.0, 1e-12 ) )
                {
                    std::cout << "First Sample Phase: " <<  phase << " out of Tolerance! Should be: "
                              << 1.0 << std::endl;
                    retCode = 5;
                    break;
                }
                auto mag = std::abs( pElementBuf[0] );
                if ( !inTolerance( mag, 1.0, 1e-12 ) )
                {
                    std::cout << "First Sample Magnitude: " <<  mag << " out of Tolerance! Should be: "
                              << 1.0 << std::endl;
                    retCode = 6;
                    break;
                }
            }

            // The second sample should have a mag of one and a phase of 2.0.
            {
                auto phase = std::arg( pElementBuf[1] );
                if ( !inTolerance( phase, 2.0, 1e-12 ) )
                {
                    std::cout << "Second Sample Phase: " <<  phase << " out of Tolerance! Should be: "
                              << 2.0 << std::endl;
                    retCode = 7;
                    break;
                }
                auto mag = std::abs( pElementBuf[1] );
                if ( !inTolerance( mag, 1.0, 1e-12 ) )
                {
                    std::cout << "Second Sample Magnitude: " <<  mag << " out of Tolerance! Should be: "
                              << 1.0 << std::endl;
                    retCode = 8;
                    break;
                }
            }
        }

        // Instantiate Specific Complex Tone Generator (-1.5 radsPerSample, 1.0 phi)
        {
            std::unique_ptr<FlyingPhasorToneGenerator> pFlyingPhasorToneGen{new FlyingPhasorToneGenerator{ -1.5, 1.0 }};

            // Fetch 2 samples.
            pFlyingPhasorToneGen->getSamples( pElementBuf.get(), 2 );

            // The first sample should have a mag of one and a phase of negative one.
            {
                auto phase = std::arg( pElementBuf[0] );
                if ( !inTolerance( phase, 1.0, 1e-12 ) )
                {
                    std::cout << "First Sample Phase: " <<  phase << " out of Tolerance! Should be: "
                              << 1.0 << std::endl;
                    retCode = 11;
                    break;
                }
                auto mag = std::abs( pElementBuf[0] );
                if ( !inTolerance( mag, 1.0, 1e-12 ) )
                {
                    std::cout << "First Sample Magnitude: " <<  mag << " out of Tolerance! Should be: "
                              << 1.0 << std::endl;
                    retCode = 12;
                    break;
                }
            }

            // The second sample should have a mag of one and a phase of -0.5
            {
                auto phase = std::arg( pElementBuf[1] );
                if ( !inTolerance( phase, -0.5, 1e-12 ) )
                {
                    std::cout << "Second Sample Phase: " <<  phase << " out of Tolerance! Should be: "
                              << -0.5 << std::endl;
                    retCode = 13;
                    break;
                }
                auto mag = std::abs( pElementBuf[1] );
                if ( !inTolerance( mag, 1.0, 1e-12 ) )
                {
                    std::cout << "Second Sample Magnitude: " <<  mag << " out of Tolerance! Should be: "
                              << 1.0 << std::endl;
                    retCode = 14;
                    break;
                }
            }

            // Attempt a reset to something else and test again.
            pFlyingPhasorToneGen->reset();
            pFlyingPhasorToneGen->getSamples( pElementBuf.get(), 2 );

            // The first sample should have a mag of 1.0 and a phase of 0.0.
            {
                auto phase = std::arg( pElementBuf[0] );
                if ( !inTolerance( phase, 0.0, 1e-12 ) )
                {
                    std::cout << "First Sample Phase: " <<  phase << " out of Tolerance! Should be: "
                              << 0.0 << std::endl;
                    retCode = 15;
                    break;
                }
                auto mag = std::abs( pElementBuf[0] );
                if ( !inTolerance( mag, 1.0, 1e-12 ) )
                {
                    std::cout << "First Sample Magnitude: " <<  mag << " out of Tolerance! Should be: "
                              << 1.0 << std::endl;
                    retCode = 16;
                    break;
                }
            }

            // The second sample should also have a mag of one and a phase of zero (pure DC).
            {
                auto phase = std::arg( pElementBuf[1] );
                if ( !inTolerance( phase, 0.0, 1e-12 ) )
                {
                    std::cout << "Second Sample Phase: " <<  phase << " out of Tolerance! Should be: "
                              << 0.0 << std::endl;
                    retCode = 17;
                    break;
                }
                auto mag = std::abs( pElementBuf[1] );
                if ( !inTolerance( mag, 1.0, 1e-12 ) )
                {
                    std::cout << "Second Sample Magnitude: " <<  mag << " out of Tolerance! Should be: "
                              << 1.0 << std::endl;
                    retCode = 18;
                    break;
                }
            }
        }

        // A quick test of the accumSamples function. It is almost exactly the same as getSamples.
        // So, if we accumulate samples into a pre-zeroed buffer, it should result in the same outcome
        // as getSamples would. This test repeats the start of our last test, using accumSamples instead.
        // Instantiate Specific Complex Tone Generator (-1.5 radsPerSample, 1.0 phi)
        {
            std::unique_ptr<FlyingPhasorToneGenerator> pFlyingPhasorToneGen{new FlyingPhasorToneGenerator{ -1.5, 1.0 }};

            // Zero out the first two elements our buffer.
            pElementBuf[0] = pElementBuf[1] = 0.0;

            // Accumulate 2 samples.
            pFlyingPhasorToneGen->accumSamples( pElementBuf.get(), 2 );

            // The first sample should have a mag of one and a phase of negative one.
            {
                auto phase = std::arg( pElementBuf[0] );
                if ( !inTolerance( phase, 1.0, 1e-12 ) )
                {
                    std::cout << "First Sample Phase: " <<  phase << " out of Tolerance! Should be: "
                              << 1.0 << std::endl;
                    retCode = 20;
                    break;
                }
                auto mag = std::abs( pElementBuf[0] );
                if ( !inTolerance( mag, 1.0, 1e-12 ) )
                {
                    std::cout << "First Sample Magnitude: " <<  mag << " out of Tolerance! Should be: "
                              << 1.0 << std::endl;
                    retCode = 21;
                    break;
                }
            }

            // The second sample should have a mag of one and a phase of -0.5
            {
                auto phase = std::arg( pElementBuf[1] );
                if ( !inTolerance( phase, -0.5, 1e-12 ) )
                {
                    std::cout << "Second Sample Phase: " <<  phase << " out of Tolerance! Should be: "
                              << -0.5 << std::endl;
                    retCode = 22;
                    break;
                }
                auto mag = std::abs( pElementBuf[1] );
                if ( !inTolerance( mag, 1.0, 1e-12 ) )
                {
                    std::cout << "Second Sample Magnitude: " <<  mag << " out of Tolerance! Should be: "
                              << 1.0 << std::endl;
                    retCode = 23;
                    break;
                }
            }
        }

        // A quick test of the getSample function. This function returns just a single sample.
        // We will test it against the getSamples function. Results should be exactly the same.
        {
            std::unique_ptr<FlyingPhasorToneGenerator> pFlyingPhasorToneGen{new FlyingPhasorToneGenerator{ M_PI / 256.0 } };

            // Fetch 2 samples.
            pFlyingPhasorToneGen->getSamples( pElementBuf.get(), 2 );

            // Now reset it and get two samples, one at a time.
            pFlyingPhasorToneGen->reset( M_PI / 256.0 );
            for ( size_t i = 0; 2 != i; ++i )
            {
                const auto sample = pFlyingPhasorToneGen->getSample();
                if ( sample != pElementBuf[i] )
                {
                    std::cout << "Get Single Sample failed at index " << i << ". Expected " << pElementBuf[i]
                        << ", obtained " << sample << std::endl;
                    retCode = 24;
                    break;
                }
            }
            if ( retCode ) break;
        }

    } while (false);

    exit( retCode );
    return retCode;
}
