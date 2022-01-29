// Created on 20220111

#include "FlyingPhasorToneGenerator.h"
#include <memory>
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
#include <fftw3.h>

#define CONDENSE_ADJACENT_LMX_ENTRIES 1
#define SORT_LMX_ENTRIES 1
#define GENERATE_CFAR_TEST_TONE 1

constexpr size_t epochSizePowerTwo = 12;
constexpr size_t numSamples = 1 << epochSizePowerTwo;

using ComplexBufferType = std::unique_ptr< FlyingPhasorToneGenerator::ElementType[] >;
using ScalarBufferType = std::unique_ptr< FlyingPhasorToneGenerator::PrecisionType[] >;

struct LocalMax
{
    LocalMax( double thePwrLevel, double theCentroid, size_t theIndex )
            : pwrLevel{thePwrLevel}, atIndex{theIndex}, centroid{theCentroid}
    {
    }

    double pwrLevel{};
    double centroid{};
    size_t atIndex{};
};

// Purposefully mis-ordered so that the biggest if first, not last.
bool operator< ( const LocalMax & a, const LocalMax & b )
{
    return a.pwrLevel > b.pwrLevel;
}

using LocalMaxBuffer = std::vector< LocalMax >;

// Constant False Alarm Rate Algorithm
// Assumes algorithm will be run on power spectrum of a power of two length ordered as normal
// fft output would be (positive through negative frequencies with the cusp at the center).
// It needs to know the power of two because it wraps at DC with twos complement math.
class CFAR_Algorithm
{
public:
    CFAR_Algorithm( uint32_t powerOfTwo, uint32_t numTrainingCells, uint32_t numGuardCells, double threshold )
      : numElements( 1 << powerOfTwo )
      , mask( numElements - 1 )
      , nTrainingCells( numTrainingCells )
      , nGuardCells( numGuardCells )
      , shift( sizeof( mask ) * 8 - powerOfTwo )
      , alpha( threshold )
    {
    }

    LocalMaxBuffer run( ScalarBufferType & powerSpectrum ) const
    {
        LocalMaxBuffer localMaxBuffer{};

        for ( uint32_t cut=0; numElements != cut; ++cut )
        {
            double leadingNoisePower{};
            double trailingNoisePower{};

            auto algIndex = uint32_t( cut - ( nTrainingCells + nGuardCells ) ) & mask;

            for ( uint32_t i = 0; nTrainingCells != i; ++i )
            {
#if 0
                if ( cut == 0 ) std::cout << "\tCUT: " << cut << " Bottom Half Filter algIndex: " << algIndex << std::endl;
                if ( cut == ( mask >> 1 ) ) std::cout << "\tCUT: " << cut << " Bottom Half Filter algIndex: " << algIndex << std::endl;
                if ( cut == mask ) std::cout << "\tCUT: " << cut << " Bottom Half Filter algIndex: " << algIndex << std::endl;
#endif
                ++algIndex &= mask;
                leadingNoisePower += powerSpectrum[ algIndex++ ];
            }

            // Skip over Upper and Lower Guard Cells and the CUT index itself. Well get
            // to extracting its power when we are ready for it.
            algIndex += 2 * nGuardCells + 1;
            algIndex &= mask;

            // Average top half of Training Cells
            for ( uint32_t i = 0; nTrainingCells != i; ++i )
            {
#if 0
                if ( cut == 0 ) std::cout << "\tCUT: " << cut << " Upper Half Filter algIndex: " << algIndex << std::endl;
                if ( cut == ( mask >> 1 ) ) std::cout << "\tCUT: " << cut << " Upper Half Filter algIndex: " << algIndex << std::endl;
                if ( cut == mask ) std::cout << "\tCUT: " << cut << " Upper Half Filter algIndex: " << algIndex << std::endl;
#endif
                ++algIndex &= mask;
                trailingNoisePower += powerSpectrum[ algIndex++ ];
            }

            ///@note This breaks down at Nyquist.
            auto centroid = [&]()
            {
                double spanPower = 0;
                double productAccum = 0;
                ///@todo This is NOT handling Signage Correctly. Think I am going too have to resurrect the shift logic.
                auto signedIndex = int32_t((cut << shift) - (nGuardCells << shift)) >> shift; // We want signed here.
                for ( uint32_t j=0; 2 * nGuardCells + 1 != j; ++j, ++signedIndex )
                {
                    auto jPow = powerSpectrum[ uint32_t( signedIndex & mask ) ];
                    spanPower += jPow;
                    productAccum += jPow * signedIndex;
                }
                return productAccum / spanPower;
            };

            // Do we have a Local Maximum (LMX)
            auto cutPower = powerSpectrum[ cut ];
            leadingNoisePower /= nTrainingCells;
            trailingNoisePower /= nTrainingCells;
            if ( cutPower > alpha * ( leadingNoisePower + trailingNoisePower ) / 2 )
            {
#if defined( CONDENSE_ADJACENT_LMX_ENTRIES ) && ( CONDENSE_ADJACENT_LMX_ENTRIES != 0 )
                // Do we have an adjacent previously recorded LMX entry?
                if ( !localMaxBuffer.empty() && cut -1 == localMaxBuffer.back().atIndex )
                {
                    auto & prevAdj = localMaxBuffer.back();

                    // We're either going to throw the adjacent away, or replace the previous entry
                    // with it. In order to replace. We need to have a hotter signal strength to replace.
                    if ( prevAdj.pwrLevel < cutPower )
                    {
                        prevAdj = LocalMax{ cutPower, centroid(), cut };
                    }
                }
                // Else, no adjacent, previously recorded LMX entry. This is a standalone LMX
                // until determined otherwise. Append to the LMX buffer.
                else
#endif
                {
                    localMaxBuffer.emplace_back( LocalMax{ cutPower, centroid(), cut } );
                }
            }
        }

#if defined( SORT_LMX_ENTRIES ) && ( SORT_LMX_ENTRIES != 0 )
        // Sort by Max Power Descending.
        std::sort( localMaxBuffer.begin(), localMaxBuffer.end() );
#endif

        return std::move( localMaxBuffer );
    }

private:
    const uint32_t numElements;
    const uint32_t mask;
    const uint32_t nTrainingCells;
    const uint32_t nGuardCells;
    const size_t shift;
    const double alpha;

};

ScalarBufferType blackman( size_t nSamples )
{
    ScalarBufferType w{new FlyingPhasorToneGenerator::PrecisionType[ nSamples ] };

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

    // Buffers big enough for tone generated and fft output, we are going use x2 zero padding
    ComplexBufferType pToneSeries{new FlyingPhasorToneGenerator::ElementType [ numSamples * 2 ] };
    ComplexBufferType pSpectralSeries{new FlyingPhasorToneGenerator::ElementType [ numSamples * 2 ] };
    ScalarBufferType pPowerSpectrum{new FlyingPhasorToneGenerator::PrecisionType [ numSamples * 2 ] };
#if defined( GENERATE_CFAR_TEST_TONE ) && ( GENERATE_CFAR_TEST_TONE != 0 )
    ComplexBufferType pToneSeries2{new FlyingPhasorToneGenerator::ElementType [ numSamples * 2 ] };
#endif

    // Blackman Window of Epoch size
    auto bWnd = blackman( numSamples );

    // FFTW wants a plan to execute. It requires the number of samples and the source and
    // destination buffer addresses.
    fftw_plan fftwPlan;
    fftwPlan = fftw_plan_dft_1d( int(numSamples*2),
                                 (fftw_complex *)pToneSeries.get(),
                                 (fftw_complex *)pSpectralSeries.get(), FFTW_FORWARD, FFTW_ESTIMATE);

    // Test case #1
    int16_t basisFunctionUnderTest = 8; // Essentially a signed index.
    double radiansPerSample = (basisFunctionUnderTest * 2 * M_PI ) / numSamples;
    std::unique_ptr< FlyingPhasorToneGenerator > pFlyingPhasorToneGen{ new FlyingPhasorToneGenerator{ radiansPerSample, 0.0 } };
    pFlyingPhasorToneGen->getSamples( numSamples, pToneSeries.get() );

#if defined( GENERATE_CFAR_TEST_TONE ) && ( GENERATE_CFAR_TEST_TONE != 0 )
    ///@todo Ideally, this would utilize a legacy tone generator. But we're only taking 1e-6 from it.
    double testToneRadiansPerSample = ( -0.45 * 2 * M_PI );   // FOR NOW.
    pFlyingPhasorToneGen->reset( testToneRadiansPerSample, 0.0 );
    pFlyingPhasorToneGen->getSamples( numSamples, pToneSeries2.get() );
    for ( size_t i=0; i != numSamples; ++i )
    {
        pToneSeries[i] += pToneSeries2[i] / 1e6;
    }
#endif

    // Multiply Epoch portion of pToneSeries by Blackman window.
    for ( size_t i=0; numSamples != i; ++i )
    {
        pToneSeries[i] *= bWnd[i];
    }

    // Perform FFT
    fftw_execute( fftwPlan );
#if 1
    // Create a Power Spectrum from the Complex Magnitude. Note we're x2 expanded here due to zero padding.
    for ( size_t i=0; numSamples * 2 != i; ++i )
    {
        auto mag = std::abs( pSpectralSeries[i] );
        pPowerSpectrum[i] = mag * mag;
    }
#if 0
    for ( size_t i=8; 24 != i; ++i )
        std::cout << "Power Spec[" << i << "] = " << pPowerSpectrum[i] << std::endl;
#endif

#else
    auto peaks = findPeaks( pSpectralSeries.get() );
    std::cout << "Top Max of: " << peaks.first.value << " found at index: " << peaks.first.index << std::endl;
    std::cout << "Second Max of: " << peaks.second.value << " found at index: " << peaks.second.index << std::endl;
    std::cout << "SNR: " << 10 * std::log10( peaks.first.value / peaks.second.value ) << " dB" << std::endl;
#endif

    // nT:5 nG:2 Thresh:5.0 works damned well.
//    CFAR_Algorithm cfarAlgorithm{ epochSizePowerTwo+1, 5, 2, 5.2 };
//    CFAR_Algorithm cfarAlgorithm{ epochSizePowerTwo+1, 5, 2, 5.0 };
    CFAR_Algorithm cfarAlgorithm{ epochSizePowerTwo+1, 5, 2, 4.8 };
//    CFAR_Algorithm cfarAlgorithm{ epochSizePowerTwo+1, 5, 2, 4.0 };
    auto lmxTable = std::move( cfarAlgorithm.run( pPowerSpectrum ) );
    for ( auto & lmx : lmxTable )
    {
        std::cout << "LMX @" << lmx.atIndex << ", powerLvl: " << lmx.pwrLevel << ", centroid: " << lmx.centroid << std::endl;
    }

#if 0

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
#endif

    // Done with the plan for now.
    fftw_destroy_plan( fftwPlan );

    exit( retCode );
    return retCode;
}
