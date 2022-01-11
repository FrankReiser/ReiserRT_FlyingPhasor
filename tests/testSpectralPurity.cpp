// Created on 20220111

#include "FlyingPhasorToneGenerator.h"
#include <memory>
#include <cmath>
#include <iostream>
#include <fftw3.h>

using WindowPtrType = std::unique_ptr< FlyingPhasorToneGenerator::PrecisionType[] >;

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
    double radiansPerSample = 1.0;
    double phi = 0.0;
#endif

    // Epoch size
    size_t numSamples = 4096;

    // Blackman Window of Epoch size
    auto blackmanW=blackman( numSamples );

#if 0
    // For maximum view of significant digits for diagnostic purposes.
    for ( size_t i = 0; i != numSamples; ++i )
        std::cout << blackmanW[ i ] << std::endl;
#endif

    // Buffers big enough for tone generated and fft output, padded x16
    std::unique_ptr< FlyingPhasorToneGenerator::ElementType[] > pToneSeries{new FlyingPhasorToneGenerator::ElementType [ numSamples * 16 ]  };
    std::unique_ptr< FlyingPhasorToneGenerator::ElementType[] > pSpectralSeries{new FlyingPhasorToneGenerator::ElementType [ numSamples * 16 ]  };
    std::unique_ptr< FlyingPhasorToneGenerator::PrecisionType [] > pPowerSpectrum{new FlyingPhasorToneGenerator::PrecisionType [ numSamples * 16 ]  };
    // Create a tone.

    // Instantiate the FlyingPhasorToneGenerator. This is what we are testing the purity of as compared to legacy methods.
    std::unique_ptr< FlyingPhasorToneGenerator > pFlyingPhasorToneGen{ new FlyingPhasorToneGenerator{ radiansPerSample, phi } };

    // Generate the tone in the padded buffer.
    pFlyingPhasorToneGen->getSamples( numSamples, pToneSeries.get() );

    // Multiply numSamples portion of pToneSeries by Blackman window.
    for ( size_t i=0; numSamples != i; ++i )
    {
        pToneSeries[i] *= blackmanW[i];
    }

#if 0
    for ( size_t i=0; numSamples*16 != i; ++i)
        std::cout << pToneSeries[i].real() << " " << pToneSeries[i].imag() << std::endl;
#endif

    fftw_plan fftwPlan;
    fftwPlan = fftw_plan_dft_1d(int(numSamples*16),
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
    for ( size_t i=0; numSamples*16 != i; ++i )
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
