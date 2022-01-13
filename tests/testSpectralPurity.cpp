// Created on 20220111

#include "FlyingPhasorToneGenerator.h"
#include <memory>
#include <cmath>
#include <iostream>
#include <fftw3.h>

using WindowPtrType = std::unique_ptr< FlyingPhasorToneGenerator::PrecisionType[] >;

#if 0
WindowPtrType blackman( size_t nSamples )
{
    WindowPtrType w{new FlyingPhasorToneGenerator::PrecisionType[ nSamples ] };

    for (size_t i = 0; i != nSamples ; ++i )
    {
#if 0
        // 'Symetric'
        w[i] = 0.42 -
            0.5 * std::cos( 2.0 * M_PI * double( i ) / double( nSamples - 1) ) +
            0.08 * std::cos(4.0 * M_PI * double( i ) / double( nSamples - 1 ) );
#else
        // 'Periodic'
        w[i] = 0.42 -
               0.5 * std::cos( 2.0 * M_PI * double( i ) / double( nSamples ) ) +
               0.08 * std::cos(4.0 * M_PI * double( i ) / double( nSamples ) );
#endif
    }

    return std::move( w );
}
#endif

int main( int argc, char * argv[] )
{
    int retCode = 0;


//    std::cout << "Hello Spectral Purity Test" << std::endl;

    std::cout << std::scientific;
    std::cout.precision(17);

#if 0
    double radiansPerSample = cmdLineParser.getRadsPerSample();
    double phi = cmdLineParser.getPhase();
#else
//    double radiansPerSample = 1.0;
    double radiansPerSample = 3 * (2*M_PI / 4096);  // Third basis function excluding DC.
    double phi = 0.0;
#endif

    // Epoch size
    constexpr size_t numSamples = 4096;

#if 0
    // Blackman Window of Epoch size
    auto blackmanW=blackman( numSamples );

    // For maximum view of significant digits for diagnostic purposes.
    for ( size_t i = 0; i != numSamples; ++i )
        std::cout << blackmanW[ i ] << std::endl;
#endif

    // Buffers big enough for tone generated and fft output
    std::unique_ptr< FlyingPhasorToneGenerator::ElementType[] > pToneSeries{new FlyingPhasorToneGenerator::ElementType [ numSamples ]  };
    std::unique_ptr< FlyingPhasorToneGenerator::ElementType[] > pSpectralSeries{new FlyingPhasorToneGenerator::ElementType [ numSamples ]  };
    std::unique_ptr< FlyingPhasorToneGenerator::PrecisionType [] > pPowerSpectrum{new FlyingPhasorToneGenerator::PrecisionType [ numSamples ]  };
    // Create a tone.

    // Instantiate the FlyingPhasorToneGenerator. This is what we are testing the purity of as compared to legacy methods.
    std::unique_ptr< FlyingPhasorToneGenerator > pFlyingPhasorToneGen{ new FlyingPhasorToneGenerator{ radiansPerSample, phi } };

    // Generate the tone in the padded buffer.
    pFlyingPhasorToneGen->getSamples( numSamples, pToneSeries.get() );

#if 0 // No Windowing
    // Multiply numSamples portion of pToneSeries by Blackman window.
    for ( size_t i=0; numSamples != i; ++i )
    {
        pToneSeries[i] *= blackmanW[i];
    }
#endif

#if 0
    for ( size_t i=0; numSamples*16 != i; ++i)
        std::cout << pToneSeries[i].real() << " " << pToneSeries[i].imag() << std::endl;
#endif

    fftw_plan fftwPlan;
    fftwPlan = fftw_plan_dft_1d( int(numSamples),
            (fftw_complex *)pToneSeries.get(),
            (fftw_complex *)pSpectralSeries.get(), FFTW_FORWARD, FFTW_ESTIMATE);

    fftw_execute( fftwPlan );
    // Done with the plan for now.
    fftw_destroy_plan( fftwPlan );

#if 0
    for ( size_t i=0; numSamples*16 != i; ++i)
        std::cout << pSpectralSeries[i].real() << " " << pSpectralSeries[i].imag() << std::endl;
#endif

    // Build the power spectrum
    for ( size_t i=0; numSamples != i; ++i )
    {
        auto mag = std::abs( pSpectralSeries[i] );
        pPowerSpectrum[i] = mag * mag;
#if 1
        std::cout << pPowerSpectrum[i] << std::endl;
#endif
    }


    exit( retCode );
    return retCode;
}
