// Created on 20220105

#include "FlyingPhasorToneGenerator.h"

#include "CommandLineParser.h"

#include <iostream>
#include <memory>
#include <cmath>
#include <limits>

#include <ctime>

double getClockMonotonic()
{
    timespec tNow = { 0, 0 };
    clock_gettime( CLOCK_MONOTONIC, &tNow );

    return double( tNow.tv_sec ) + double( tNow.tv_nsec ) / 1e9;
}

// Performs "Running/Online" statistics accumulation.
// Implements the Welford's "Online" in a state machine plus additional statistics.
// This algorithm is much less prone to loss of precision due to catastrophic cancellation.
// Additionally, it uses "long double" format for mathematics and state to better compute
// variance from small deviations in the input train.
class StatsStateMachine
{
public:
    void addSample( double value )
    {
        if ( value < min ) min = value;
        if ( value > max ) max = value;
        long double delta = value - mean;
        ++nSamples;
        mean += delta / (long double)( nSamples );
        M2 += delta * ( value - mean );
    }

    // Currently, returns mean and variance
    std::pair<double, double> getStats() const
    {
        constexpr long double myNAN = std::numeric_limits<long double>::quiet_NaN();
        switch ( nSamples )
        {
            case 0 : return { myNAN, myNAN };
            case 1 : return { mean, myNAN };
            default : return { mean, M2 / (long double)(nSamples-1) };
        }
    }
    std::pair<double, double> getPopcorn() const { return {min,max}; }

    void reset() { mean=0.0; M2=0.0; min=0.0, max=0.0; nSamples=0; }


private:
    long double mean{0.0};
    long double M2{0.0};
    double max{std::numeric_limits< double >::max()};
    double min{-max};
    size_t nSamples{0};
};

bool inTolerance( double value, double desiredValue, double toleranceRatio )
{
    auto minValue = desiredValue * ( 1 - toleranceRatio );
    auto maxValue = desiredValue * ( 1 + toleranceRatio );

    return ( minValue <= value && value <= maxValue );
}

class PhasePurityAnalyzer
{
public:

    static double deltaAngle( double angleA, double angleB )
    {
        auto delta = angleB - angleA;
        if ( delta > M_PI ) delta -= 2*M_PI;
        else if ( delta < -M_PI ) delta += 2*M_PI;
        return delta;
    }

    int analyzeSinusoidPhaseStability( const FlyingPhasorToneGenerator::ElementBufferTypePtr & pBuf, size_t nSamples,
                                      double radiansPerSample, double phi )
    {
        int retCode = 0;

        // Reset stats in case an instance is re-run.
        statsStateMachine.reset();

        for ( size_t n=0; nSamples != n; ++n )
        {
            ///@todo Make initialization a separate test that we run so we can remove it from here.
            // If n is zero, then we merely want to ensure that our first phase is approximately equivalent
            // to argument "phi". If not, this is an obvious failure.
            if ( 0 == n )
            {
                // This 1e-9 tolerance check adequate for initial phi setting.
                if ( !inTolerance( std::arg( pBuf[ n ] ), phi, 1e-9 ) )
                {
                    std::cout << "Starting Phase: " <<  std::arg( pBuf[ n ] ) << " Out of Tolerance! Should be: "
                              << phi << std::endl;

                    retCode = 1;
                    break;
                }
            }

            // We cheat the first sample because there is no previous one in order to compute
            // a delta. Being that we are expecting a periodic complex waveform, this would be
            // the radiansPerSample given as an argument.
            auto phaseDelta = 0 == n ? radiansPerSample : deltaAngle(std::arg(pBuf[n - 1 ] ), std::arg(pBuf[ n ] ) );

            // If the phaseDelta is far too far off of radiansPerSample. This is an obvious failure.
            // We are trying to catch the oddball here as a few oddballs may not create much variance.
            // Variance we check later.
            if ( !inTolerance( phaseDelta, radiansPerSample, 1e-12 ) )
            {
                std::cout << "Phase error at sample: " << n << "! Expected: " << radiansPerSample
                          << ", Detected: " << phaseDelta << std::endl;
                retCode = 2;
                break;
            }

            // Run stats state machine for overall mean and variance check.
            statsStateMachine.addSample( phaseDelta );
        }

        // Now test the statistics look good.
        auto rateStats = statsStateMachine.getStats();
        if ( !inTolerance( rateStats.first, radiansPerSample, 1e-15 ) )
        {
            std::cout << "Mean Angular Rate error! Expected: " << radiansPerSample
                      << ", Detected: " << rateStats.first << std::endl;
            retCode = 3;
        }
        else if ( rateStats.second > 1.5e-32 )
        {
            std::cout << "Phase Noise error! Expected less than: " << 1.5e-32
                      << ", Detected: " << rateStats.second << std::endl;
            retCode = 4;
        }

        std::cout << "Mean Angular Rate: " << rateStats.first << ", Variance: " << rateStats.second << std::endl;

        return retCode;
    }

private:
    StatsStateMachine statsStateMachine{};
};

class MagPurityAnalyzer
{
public:
    int analyzeSinusoidMagnitudeStability( const FlyingPhasorToneGenerator::ElementBufferTypePtr & pBuf, size_t nSamples )
    {
        int retCode = 0;

        // Reset stats in case an instance is re-run.
        statsStateMachine.reset();

        for ( size_t n=0; nSamples != n; ++n )
        {
            // If the magnitude is far too far off of 1.0. This is an obvious failure.
            // We are trying to catch the oddball here as a few oddballs may not create much variance.
            // Variance we check later.
            auto mag = std::abs( pBuf[ n ] );
            if ( !inTolerance( mag, 1.0, 1e-15 ) )
            {
                std::cout << "Magnitude error at sample: " << n << "! Expected: " << 1.0
                          << ", Detected: " << mag << std::endl;
                retCode = 1;
                break;
            }

            statsStateMachine.addSample( mag );
        }

        // Now test the statistics look good.
        auto magStats = statsStateMachine.getStats();
        if ( !inTolerance( magStats.first, 1.0, 1e-12 ) )
        {
            std::cout << "Mean Magnitude error! Expected: " << 1.0
                      << ", Detected: " << magStats.first << std::endl;
            retCode = 2;
        }
        else if ( magStats.second > 5e-33 )
        {
            std::cout << "Magnitude Noise error! Expected less than: " << 5e-33
                      << ", Detected: " << magStats.second << std::endl;
            retCode = 3;
        }

        double signalPower = magStats.first * magStats.first / 2;   // Should always be 0.5
        std::cout << "Mean Magnitude: " << magStats.first << ", Variance: " << magStats.second
                  << ", SNR: " << 10.0 * std::log10( signalPower / magStats.second ) << " dB" << std::endl;

        return retCode;
    }

private:
    StatsStateMachine statsStateMachine{};
};

int main( int argc, char * argv[])
{
    std::cout << "Hello TSG Complex Tone Generator" << std::endl;

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

    // For maximum view of significant digits for diagnostic purposes.
    std::cout << std::scientific;
    std::cout.precision(17);

    // Create buffers for a number of samples for both the legacy and "Flying Phasor" generators.
    const size_t maxSamples = 8192;
    std::unique_ptr< FlyingPhasorToneGenerator::ElementType[] > pLegacyToneSeries{new FlyingPhasorToneGenerator::ElementType [ maxSamples] };
    std::unique_ptr< FlyingPhasorToneGenerator::ElementType[] > pFlyingPhasorToneGenSeries{new FlyingPhasorToneGenerator::ElementType [ maxSamples] };

    // Instantiate the FlyingPhasorToneGenerator. This is what we are testing the purity of as compared to legacy methods.
    std::unique_ptr< FlyingPhasorToneGenerator > pFlyingPhasorToneGen{ new FlyingPhasorToneGenerator{ radiansPerSample, phi } };

    // Phase and Magnitude Purity Analyzers for each "FlyingPhasor" and "Legacy" tone generators.
    PhasePurityAnalyzer legacyPhasePurityAnalyzer{};
    MagPurityAnalyzer legacyMagPurityAnalyzer{};
    PhasePurityAnalyzer flyingPhasorPhasePurityAnalyzer{};
    MagPurityAnalyzer flyingPhasorMagPurityAnalyzer{};

    // Ask for some samples from the complex tone generator
    size_t numSamples = 4096;
    double t0, t1;

    // A New, Old-Fashioned Way. Loop on complex exponential. It implements the same cos(x) + j * sin(x).
    constexpr FlyingPhasorToneGenerator::ElementType j{ 0.0, 1.0 };
    FlyingPhasorToneGenerator::ElementBufferTypePtr p = pLegacyToneSeries.get();
    t0 = getClockMonotonic();
    for (size_t n = 0; numSamples != n; ++n )
    {
        p[n] = exp( j * ( double( n ) * radiansPerSample + phi ) );
    }
    t1 = getClockMonotonic();
    std::cout << "Legacy Generator Performance for numSamples: " << numSamples
              << " is " << t1-t0 << " seconds." << std::endl;
#if 0
    // What did we get
    for (size_t n = 0; numSamples != n; ++n )
    {
        FlyingPhasorToneGenerator::ElementType & s = p[n];
        std::cout << "n: " << n << ", x: " << real(s) << ", y: " << imag(s)
                  << ", mag: " << abs(s) << ", phase: " << arg(s) << std::endl;
    }
#endif

    int retCode = 0;

    legacyPhasePurityAnalyzer.analyzeSinusoidPhaseStability(pLegacyToneSeries.get(), numSamples, radiansPerSample, phi );
    legacyMagPurityAnalyzer.analyzeSinusoidMagnitudeStability(pLegacyToneSeries.get(), numSamples );
    t0 = getClockMonotonic();
    p = pFlyingPhasorToneGenSeries.get();
    pFlyingPhasorToneGen->getSamples(numSamples, p );
    t1 = getClockMonotonic();
    std::cout << "Flying Phasor Generator Performance for numSamples: " << numSamples
        << " is " << t1-t0 << " seconds." << std::endl;

#if 0
    // What did we get
    for (size_t n = 0; numSamples != n; ++n )
    {
        FlyingPhasorToneGenerator::ElementType & s = p[n];
        std::cout << "n: " << n << ", x: " << real(s) << ", y: " << imag(s)
            << ", mag: " << abs(s) << ", phase: " << arg(s) << std::endl;
    }
#endif

    flyingPhasorPhasePurityAnalyzer.analyzeSinusoidPhaseStability(pFlyingPhasorToneGenSeries.get(), numSamples, radiansPerSample, phi );
    flyingPhasorMagPurityAnalyzer.analyzeSinusoidMagnitudeStability(pFlyingPhasorToneGenSeries.get(), numSamples );


    exit( retCode );
    return retCode;
}
