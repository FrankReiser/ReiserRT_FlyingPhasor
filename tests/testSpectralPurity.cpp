// Created on 20220111

#include "FlyingPhasorToneGenerator.h"
#include "CommandLineParser.h"
#include <memory>
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
#include <fftw3.h>

#define CONSOLIDATE_ADJACENT_LMX_ENTRIES 1
#define SORT_LMX_ENTRIES 1
#define GENERATE_CFAR_TEST_TONE 0

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
                leadingNoisePower += powerSpectrum[ algIndex ];
                ++algIndex &= mask;
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
                trailingNoisePower += powerSpectrum[ algIndex ];
                ++algIndex &= mask;
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
#if defined( CONSOLIDATE_ADJACENT_LMX_ENTRIES ) && ( CONSOLIDATE_ADJACENT_LMX_ENTRIES != 0 )
                // Do we have an adjacent previously recorded LMX entry?
                if ( !localMaxBuffer.empty() && ( cut -1 == localMaxBuffer.back().atIndex ||
                    cut -2 == localMaxBuffer.back().atIndex ) )
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

int main( int argc, char * argv[] )
{
    int retCode = 0;


    std::cout << std::scientific;
    std::cout.precision(17);

    // Parse potential command line. Defaults provided otherwise.
    CommandLineParser cmdLineParser{};
    if ( 0 != cmdLineParser.parseCommandLine(argc, argv) )
    {
        std::cout << "Failed parsing command line" << std::endl;
        std::cout << "Optional Arguments are:" << std::endl;
        std::cout << "\t--radsPerSample=<double>: The radians per sample to used." << std::endl;
        std::cout << "\t--phase=<double>: The initial phase in radians." << std::endl;

        exit( -1 );
    }
#if 1
    else
    {
        std::cout << "Parsed: --radiansPerSample=" << cmdLineParser.getRadsPerSample()
                  << " --phase=" << cmdLineParser.getPhase() << std::endl << std::endl;
    }
#endif
    double radiansPerSample = cmdLineParser.getRadsPerSample();
    double phi = cmdLineParser.getPhase();

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

    // Instantiate Tone Generator
    std::unique_ptr< FlyingPhasorToneGenerator > pFlyingPhasorToneGen{ new FlyingPhasorToneGenerator{ radiansPerSample, phi } };
    pFlyingPhasorToneGen->getSamples( numSamples, pToneSeries.get() );

#if defined( GENERATE_CFAR_TEST_TONE ) && ( GENERATE_CFAR_TEST_TONE != 0 )
    ///@todo Ideally, this would utilize a legacy tone generator. But we're only taking a fraction from it.
//    pFlyingPhasorToneGen->reset( -radiansPerSample, -phi );
    pFlyingPhasorToneGen->reset( radiansPerSample + M_PI / 2.0, 0.0 );
    pFlyingPhasorToneGen->getSamples( numSamples, pToneSeries2.get() );
    for ( size_t i=0; i != numSamples; ++i )
    {
//        pToneSeries[i] += pToneSeries2[i] / 1e3;    // 20log(1e-3) = -60dB
//        pToneSeries[i] += pToneSeries2[i] / 1e4;    // 20log(1e-4) = -80dB
//        pToneSeries[i] += pToneSeries2[i] / 1e5;    // 20log(1e-5) = -100dB
//        pToneSeries[i] += pToneSeries2[i] / 1e6;    // 20log(1e-6) = -120dB
//        pToneSeries[i] += pToneSeries2[i] / 1e7;    // 20log(1e-6) = -140dB
//        pToneSeries[i] += pToneSeries2[i] / 1e8;    // 20log(1e-6) = -160dB
        pToneSeries[i] += pToneSeries2[i] / 1e9;    // 20log(1e-6) = -180dB
    }
#endif

    // Multiply Epoch portion of pToneSeries by Blackman window.
    for ( size_t i=0; numSamples != i; ++i )
    {
        pToneSeries[i] *= bWnd[i];
    }

    // Perform FFT
    fftw_execute( fftwPlan );

    // Create a Power Spectrum from the Complex Magnitude. Note we're x2 expanded here due to zero padding.
    // While we are at it, build a noise floor from the average per filter.
    double noiseFloor{};
    size_t noiseElements{};
    for ( size_t i=0; numSamples * 2 != i; ++i )
    {
        auto mag = std::abs( pSpectralSeries[i] );
#if 0
        // NOTE: This is somewhat arbitrary. We know we have high quality signals. We want to exclude
        // windowing tail regions around actual signal spectra.
        if ( 1e-3 > ( pPowerSpectrum[i] = mag * mag ) )
        {
            noiseFloor += pPowerSpectrum[i];
            ++noiseElements;
        }
#else
        noiseFloor += pPowerSpectrum[i] = mag * mag;
        ++noiseElements;
#endif
    }
//    if ( !noiseElements ) ++noiseElements;
//    noiseFloor /= double( noiseElements );
//    std::cout << "Noise Floor Estimate: " << noiseFloor << ", Noise Floor Elements: " << noiseElements << std::endl;
#if 0
    for ( size_t i=8; 24 != i; ++i )
        std::cout << "Power Spec[" << i << "] = " << pPowerSpectrum[i] << std::endl;
#endif

    // nT:5 nG:2 Thresh:5.0 works damned well.
//    CFAR_Algorithm cfarAlgorithm{ epochSizePowerTwo+1, 5, 2, 5.2 };
//    CFAR_Algorithm cfarAlgorithm{ epochSizePowerTwo+1, 5, 2, 5.0 };
//    CFAR_Algorithm cfarAlgorithm{ epochSizePowerTwo+1, 5, 2, 5.0 };
//    CFAR_Algorithm cfarAlgorithm{ epochSizePowerTwo+1, 5, 2, 4.5 };
    CFAR_Algorithm cfarAlgorithm{ epochSizePowerTwo+1, 6, 2, 7.5 };
    // We are willing to get some false alarms out of this algorithm. We are almost counting on it. Er, NO, do NOT
//    CFAR_Algorithm cfarAlgorithm{ epochSizePowerTwo+1, 5, 2, 2.75 };
//    CFAR_Algorithm cfarAlgorithm{ epochSizePowerTwo+1, 5, 2, 2.30 };
//    CFAR_Algorithm cfarAlgorithm{ epochSizePowerTwo+1, 5, 2, 1.800 };
    auto lmxTable = std::move( cfarAlgorithm.run( pPowerSpectrum ) );
    // Window out LMXs from the noise floor estimation.
    for ( auto & lmx : lmxTable )
    {
        auto algIndex = uint32_t( lmx.atIndex - ( 30 ) ) & (numSamples * 2 - 1);
        for ( uint32_t i=0; 30*2+1 != i; ++i )
        {
            auto & p = pPowerSpectrum[ algIndex ];
            if ( p != 0.0 )
            {
                noiseFloor -= p;
                p = 0.0;           // We do not want to possibly get it again on another LMX.
                --noiseElements;
            }
            ++algIndex &= (numSamples * 2 - 1);
        }
    }
    if ( !noiseElements ) ++noiseElements;
    noiseFloor /= double( noiseElements );
    std::cout << "New Noise Floor Estimate: " << noiseFloor << ", Noise Floor Elements: " << noiseElements << std::endl;
    for ( auto & lmx : lmxTable )
    {
        std::cout << "LMX @" << lmx.atIndex
            << ", powerLvl: " << lmx.pwrLevel
            << ", centroid: " << lmx.centroid
            << ", radiansPerSample: " << 2 * M_PI * lmx.centroid / 2 / numSamples
                << std::endl;
    }



    // Done with the plan for now.
    fftw_destroy_plan( fftwPlan );

    exit( retCode );
    return retCode;
}
