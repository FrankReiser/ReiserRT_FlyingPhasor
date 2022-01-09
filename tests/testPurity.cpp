// Created on 20220105

#include "FlyingPhasorToneGenerator.h"

#include "CommandLineParser.h"
#include "MiscTestUtilities.h"

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
    static constexpr long double myNAN = std::numeric_limits<long double>::quiet_NaN();

public:
    void addSample( double value )
    {
        long double delta = value - mean;
        ++nSamples;
        mean += delta / (long double)( nSamples );
        M2 += delta * ( value - mean );

        delta = value - mean;
        if (delta < maxNegDev ) maxNegDev = delta;
        if (delta > maxPosDev ) maxPosDev = delta;
    }

    // Currently, returns mean and variance
    std::pair<double, double> getStats() const
    {
        switch ( nSamples )
        {
            case 0 : return { myNAN, myNAN };
            case 1 : return { mean, myNAN };
            default : return { mean, M2 / (long double)(nSamples-1) };
        }
    }
    std::pair<double, double> getPopcorn() const
    {
        switch ( nSamples )
        {
            case 0 : return { myNAN, myNAN };
            default : return {maxNegDev, maxPosDev };
        }
    }

    void reset()
    {
        mean = 0.0;
        M2 = 0.0;
        maxNegDev = std::numeric_limits< double >::max();
        maxPosDev = std::numeric_limits< double >::lowest();
        nSamples = 0;
    }

private:
    long double mean{ 0.0 };
    long double M2{ 0.0 };
    long double maxNegDev{std::numeric_limits< long double >::max() };
    long double maxPosDev{std::numeric_limits< long double >::lowest() };
    size_t nSamples{};
};

class PhasePurityAnalyzer
{
public:

    void analyzePhaseStability( const FlyingPhasorToneGenerator::ElementBufferTypePtr & pBuf, size_t nSamples,
                              double radiansPerSample, double phi )
    {
        // Reset stats in case an instance is re-run.
        statsStateMachine.reset();

        for ( size_t n=0; nSamples != n; ++n )
        {
            // We cheat the first sample because there is no previous one in order to compute
            // a delta. Being that we are expecting a periodic complex waveform, this would be
            // the radiansPerSample given as an argument.
            auto phaseDelta = 0 == n ? radiansPerSample : deltaAngle(std::arg(pBuf[n - 1 ] ), std::arg(pBuf[ n ] ) );

#if 0
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
#endif
            // Run stats state machine for overall mean and variance check.
            statsStateMachine.addSample( phaseDelta );
        }

#if 0
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
#endif
    }

    std::pair<double, double> getStats() const { return statsStateMachine.getStats(); }
    std::pair<double, double> getPopcorn() const { return statsStateMachine.getPopcorn(); }

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

int main( int argc, char * argv[] )
{
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

    // New Old-Fashioned Way. Loop on complex exponential. It implements the same cos(x) + j * sin(x)
    // as the Legacy generator. Just in a little less source code written by me.
    constexpr FlyingPhasorToneGenerator::ElementType j{ 0.0, 1.0 };
    FlyingPhasorToneGenerator::ElementBufferTypePtr p = pLegacyToneSeries.get();
    t0 = getClockMonotonic();
    for ( size_t n = 0; numSamples != n; ++n )
    {
        // Complex Exponential
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

    // NOTE: We are not going to "Validate" performance of the Legacy Generator. It's good.
    // At least possibly up to some extremely high number of samples. The Flying Phasor Generator
    // is immune to number of samples up to infinity.
    // We primarily run analysis on it to establish a baseline of performance metrics
    // to validate the Flying Phasor Generator against.
    legacyPhasePurityAnalyzer.analyzePhaseStability( pLegacyToneSeries.get(), numSamples, radiansPerSample, phi );
    {
        auto stats = legacyPhasePurityAnalyzer.getStats();
        auto popCorn = legacyPhasePurityAnalyzer.getPopcorn();
        std::cout << "Mean Angular Rate: " << stats.first << ", Variance: " << stats.second << std::endl;
        std::cout << "Phase PopCorn Noise: maxNegDev: " << popCorn.first << ", maxPosDev: " << popCorn.second << std::endl;
    }
    legacyMagPurityAnalyzer.analyzeSinusoidMagnitudeStability(pLegacyToneSeries.get(), numSamples );


    t0 = getClockMonotonic();
    p = pFlyingPhasorToneGenSeries.get();
    pFlyingPhasorToneGen->getSamples( numSamples, p );
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

    flyingPhasorPhasePurityAnalyzer.analyzePhaseStability( pFlyingPhasorToneGenSeries.get(), numSamples,
        radiansPerSample, phi );
    {
        auto stats = flyingPhasorPhasePurityAnalyzer.getStats();
        auto popCorn = flyingPhasorPhasePurityAnalyzer.getPopcorn();
        std::cout << "Mean Angular Rate: " << stats.first << ", Variance: " << stats.second << std::endl;
        std::cout << "Phase PopCorn Noise: maxNegDev: " << popCorn.first << ", maxPosDev: " << popCorn.second << std::endl;
    }
    flyingPhasorMagPurityAnalyzer.analyzeSinusoidMagnitudeStability(pFlyingPhasorToneGenSeries.get(), numSamples );


    // Now for the Validating.
    int retCode = 0;

    do
    {
        // Flying Phasor Tone Generator Phase Purity Validation
        {
            auto legacyPopcorn = legacyPhasePurityAnalyzer.getPopcorn();
            auto flyingPhasorPopcorn = flyingPhasorPhasePurityAnalyzer.getPopcorn();
            auto legacyPeakAbsDev = std::max( -legacyPopcorn.first, legacyPopcorn.second );
            auto flyingPhasorPeakAbsDev = std::max( -flyingPhasorPopcorn.first, flyingPhasorPopcorn.second );

            std::cout << std::endl << std::endl
                << "Old Peak Phase Noise: " << legacyPeakAbsDev
                << ", New Peak Phase Noise: " << flyingPhasorPeakAbsDev << std::endl
                ;

            if ( flyingPhasorPeakAbsDev < legacyPeakAbsDev )
            {
                std::cout << "Flying Phasor BEATS out Legacy in Peak Abs Phase Deviation Test!" << std::endl;
            }
            else if (flyingPhasorPeakAbsDev < legacyPeakAbsDev * 1.01 )
            {
                std::cout << "Flying Phasor WITHIN Peak Abs Phase Deviation Test Tolerance." << std::endl;
            }
            else
            {
                std::cout << "Flying Phasor FAILS Peak Abs Phase Deviation Test - Out of Tolerance!" << std::endl;
                retCode = 1;
                break;
            }

            auto flyingPhasorStats = flyingPhasorPhasePurityAnalyzer.getStats();
            auto flyingPhasorMean = flyingPhasorStats.first;
            if ( !inTolerance( flyingPhasorMean, radiansPerSample, 1e-15 ) )
            {
                std::cout << "Flying Phasor FAILS Mean Angular Rate Test! Expected: " << radiansPerSample
                          << ", Detected: " << flyingPhasorMean << std::endl;
                retCode = 2;
                break;
            }

            auto legacyStats = legacyPhasePurityAnalyzer.getStats();
            auto legacyVariance = legacyStats.second;
            auto flyingPhasorVariance = flyingPhasorStats.second;
            if ( flyingPhasorVariance < legacyVariance )
            {
                std::cout << "Flying Phasor BEATS out Legacy in Phase Variance Test!" << std::endl;
            }
            else if (flyingPhasorVariance < legacyVariance * 1.075 )
            {
                std::cout << "Flying Phasor WITHIN Phase Variance Test Tolerance." << std::endl;
            }
            else
            {
                std::cout << "Flying Phasor FAILS Phase Variance Test - Out of Tolerance!" << std::endl;
                retCode = 3;
                break;
            }
        }

    } while (false);


    exit( retCode );
    return retCode;
}
