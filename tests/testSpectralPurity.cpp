// Created on 20220111

#include "FlyingPhasorToneGenerator.h"
#include <memory>
#include <cmath>
#include <iostream>
#include <fftw3.h>

int main( int argc, char * argv[] )
{
    int retCode = 0;


    std::cout << std::scientific;
    std::cout.precision(17);

    // I am only going to test this with exact basis functions because I want to use
    // windowless, un-padded FFTs here. It's the most straight forward way at proving
    // the Flying Phase Generator Noise Floor is comparable to Legacy Methods.
    constexpr size_t numSamples = 4096;
    constexpr double radiansPerFilter = (2 * M_PI / numSamples);
    double radiansPerSample = 8 * radiansPerFilter;  // Eighth basis function excluding DC.
    double phi = 0.0;

    // Epoch size

    // Buffers big enough for tone generated, fft output and a power spectrum.
    std::unique_ptr< FlyingPhasorToneGenerator::ElementType[] > pToneSeries{new FlyingPhasorToneGenerator::ElementType [ numSamples ] };
    std::unique_ptr< FlyingPhasorToneGenerator::ElementType[] > pSpectralSeries{new FlyingPhasorToneGenerator::ElementType [ numSamples ] };
    std::unique_ptr< FlyingPhasorToneGenerator::PrecisionType[] > pPowerSpectrum{new FlyingPhasorToneGenerator::PrecisionType [ numSamples ] };
    // Create a tone.

    // Instantiate the FlyingPhasorToneGenerator. This is what we are testing the purity of as compared to legacy methods.
    std::unique_ptr< FlyingPhasorToneGenerator > pFlyingPhasorToneGen{ new FlyingPhasorToneGenerator{radiansPerSample, phi } };

    // Generate the tone in the padded buffer.
    pFlyingPhasorToneGen->getSamples( numSamples, pToneSeries.get() );

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

    // Build the power spectrum and while where at it record the Top Maximum
    double topMaxVal = 0.0;
    size_t topMaxIndex = -1;
    for ( size_t i=0; numSamples != i; ++i )
    {
        auto mag = std::abs( pSpectralSeries[i] );
        if ( topMaxVal < ( pPowerSpectrum[i] = mag * mag ) )
        {
            topMaxVal = pPowerSpectrum[i];
            topMaxIndex = i;
        }
#if 0
        std::cout << pPowerSpectrum[i] << std::endl;
#endif
    }
    std::cout << "Top Max of: " << topMaxVal << " found at index: " << topMaxIndex << std::endl;
    std::cout << "\tPhase: " << std::arg( pSpectralSeries[ topMaxIndex ] ) << std::endl;

    // Find the Second Maximum
    pPowerSpectrum[topMaxIndex] = 0;    // So we don't find it again
#if 0
    pPowerSpectrum[1024] = 1;    // Test Signal Poke
#endif
    double secondMaxVal = 0.0;
    size_t secondMaxIndex = -1;
    for ( size_t i=0; numSamples != i; ++i )
    {
        if ( secondMaxVal < pPowerSpectrum[i] )
        {
            secondMaxVal = pPowerSpectrum[i];
            secondMaxIndex = i;
        }
#if 0
        std::cout << pPowerSpectrum[i] << std::endl;
#endif
    }
    std::cout << "Second Max of: " << secondMaxVal << " found at index: " << secondMaxIndex << std::endl;
    std::cout << "SNR: " << 10 * std::log10( topMaxVal / secondMaxVal ) << " dB" << std::endl;

#if 0
    // Now I just want to look about the Nyquist point. Any artifacts of our re-normalization
    // process (every other sample), should show up here.
    constexpr size_t searchWidth = 128;
    constexpr size_t startPoint = ( numSamples >> 1 ) - ( searchWidth >> 1 );
    double nthVal = 0.0;
    size_t nthIndex = -1;
    for ( size_t i=startPoint, j=0; searchWidth != j; ++i, ++j )
    {
        if ( nthVal < pPowerSpectrum[i] )
        {
            nthVal = pPowerSpectrum[i];
            nthIndex = i;
        }
    }
    std::cout << "nth Max of: " << nthVal << " found at index: " << nthIndex << std::endl;
#endif

    exit( retCode );
    return retCode;
}
