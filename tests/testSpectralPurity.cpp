// Created on 20220111

#include "FlyingPhasorToneGenerator.h"
#include <memory>
#include <cmath>
#include <iostream>
#include <fftw3.h>

constexpr size_t numSamples = 4096;

struct PeakInfo
{
    size_t index = ~0;
    double value = 0.0;
};

std::pair< PeakInfo, PeakInfo > findPeaks( FlyingPhasorToneGenerator::ElementType pSpectralSeries[] )
{
    std::unique_ptr< FlyingPhasorToneGenerator::PrecisionType[] > pPowerSpectrum{new FlyingPhasorToneGenerator::PrecisionType [ numSamples ] };

    // Find the top maximum while also building a power spectrum.
    PeakInfo topPeak;
    for (size_t i=0; numSamples != i; ++i )
    {
        auto mag = std::abs( pSpectralSeries[i] );
        if ( topPeak.value < ( pPowerSpectrum[i] = mag * mag ) )
        {
            topPeak.value = pPowerSpectrum[i];
            topPeak.index = i;
        }
    }

    // Find the second maximum
    PeakInfo secondPeak;
    pPowerSpectrum[ topPeak.index ] = 0;    // So we don't find it again
    for (size_t i=0; numSamples != i; ++i )
    {
        if ( secondPeak.value < pPowerSpectrum[i] )
        {
            secondPeak.value = pPowerSpectrum[i];
            secondPeak.index = i;
        }
    }

    return { topPeak, secondPeak };
}

// I am only going to test this with exact basis functions because I want to use
// windowless, un-padded FFTs. This is the most straight forward way at proving
// the Flying Phase Generator Noise Floor is comparable to Legacy Methods.
int main( int argc, char * argv[] )
{
    int retCode = 0;


    std::cout << std::scientific;
    std::cout.precision(17);

    // Buffers big enough for tone generated and fft output
    std::unique_ptr< FlyingPhasorToneGenerator::ElementType[] > pToneSeries{new FlyingPhasorToneGenerator::ElementType [ numSamples ] };
    std::unique_ptr< FlyingPhasorToneGenerator::ElementType[] > pSpectralSeries{new FlyingPhasorToneGenerator::ElementType [ numSamples ] };

    // FFTW wants a plan to execute. It requires the number of samples and the source and
    // destination buffer addresses.
    fftw_plan fftwPlan;
    fftwPlan = fftw_plan_dft_1d( int(numSamples),
                                 (fftw_complex *)pToneSeries.get(),
                                 (fftw_complex *)pSpectralSeries.get(), FFTW_FORWARD, FFTW_ESTIMATE);

    // Test case #1
    int16_t basisFunctionUnderTest = 8; // Essentially a signed index.
    double radiansPerSample = (basisFunctionUnderTest * 2 * M_PI ) / numSamples;
    std::unique_ptr< FlyingPhasorToneGenerator > pFlyingPhasorToneGen{ new FlyingPhasorToneGenerator{ radiansPerSample, 0.0 } };
    pFlyingPhasorToneGen->getSamples( numSamples, pToneSeries.get() );
    fftw_execute( fftwPlan );
    auto peaks = findPeaks( pSpectralSeries.get() );
    std::cout << "Top Max of: " << peaks.first.value << " found at index: " << peaks.first.index << std::endl;
    std::cout << "Second Max of: " << peaks.second.value << " found at index: " << peaks.second.index << std::endl;
    std::cout << "SNR: " << 10 * std::log10( peaks.first.value / peaks.second.value ) << " dB" << std::endl;

    // Test case #2
    basisFunctionUnderTest = -8;
    radiansPerSample = ( basisFunctionUnderTest * 2 * M_PI ) / numSamples;
    pFlyingPhasorToneGen->reset( radiansPerSample, 0.0 );
    pFlyingPhasorToneGen->getSamples( numSamples, pToneSeries.get() );
    fftw_execute( fftwPlan );
    peaks = findPeaks( pSpectralSeries.get() );
    std::cout << "Top Max of: " << peaks.first.value << " found at index: " << peaks.first.index << std::endl;
    std::cout << "Second Max of: " << peaks.second.value << " found at index: " << peaks.second.index << std::endl;
    std::cout << "SNR: " << 10 * std::log10( peaks.first.value / peaks.second.value ) << " dB" << std::endl;

    // Test case #3
    basisFunctionUnderTest = 1000;
    radiansPerSample = ( basisFunctionUnderTest * 2 * M_PI ) / numSamples;
    pFlyingPhasorToneGen->reset( radiansPerSample, 0.0 );
    pFlyingPhasorToneGen->getSamples( numSamples, pToneSeries.get() );
    fftw_execute( fftwPlan );
    peaks = findPeaks( pSpectralSeries.get() );
    std::cout << "Top Max of: " << peaks.first.value << " found at index: " << peaks.first.index << std::endl;
    std::cout << "Second Max of: " << peaks.second.value << " found at index: " << peaks.second.index << std::endl;
    std::cout << "SNR: " << 10 * std::log10( peaks.first.value / peaks.second.value ) << " dB" << std::endl;

    // Done with the plan for now.
    fftw_destroy_plan( fftwPlan );

    exit( retCode );
    return retCode;
}
