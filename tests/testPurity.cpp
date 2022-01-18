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
            case 0 : return { std::numeric_limits<long double>::quiet_NaN(), std::numeric_limits<long double>::quiet_NaN() };
            case 1 : return { mean, std::numeric_limits<long double>::quiet_NaN() };
            default : return { mean, M2 / (long double)(nSamples-1) };
        }
    }
    std::pair<double, double> getMinMaxDev() const
    {
        switch ( nSamples )
        {
            case 0 : return { std::numeric_limits<long double>::quiet_NaN(), std::numeric_limits<long double>::quiet_NaN() };
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
    std::pair<double, double> getMinMaxDev() const { return statsStateMachine.getMinMaxDev(); }

private:
    StatsStateMachine statsStateMachine{};
};

class MagPurityAnalyzer
{
public:
    void analyzeSinusoidMagnitudeStability( const FlyingPhasorToneGenerator::ElementBufferTypePtr & pBuf, size_t nSamples )
    {
        // Reset stats in case an instance is re-run.
        statsStateMachine.reset();

        for ( size_t n=0; nSamples != n; ++n )
        {
            // If the magnitude is far too far off of 1.0. This is an obvious failure.
            // We are trying to catch the oddball here as a few oddballs may not create much variance.
            // Variance we check later.
            auto mag = std::abs( pBuf[ n ] );
#if 0
            if ( !inTolerance( mag, 1.0, 1e-15 ) )
            {
                std::cout << "Magnitude error at sample: " << n << "! Expected: " << 1.0
                          << ", Detected: " << mag << std::endl;
                retCode = 1;
                break;
            }
#endif

            statsStateMachine.addSample( mag );
        }

#if 0
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
#endif
    }

    std::pair<double, double> getStats() const { return statsStateMachine.getStats(); }
    std::pair<double, double> getMinMaxDev() const { return statsStateMachine.getMinMaxDev(); }

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

    std::cout << "************ Legacy Performance Measurements ************" << std::endl;
    legacyPhasePurityAnalyzer.analyzePhaseStability( pLegacyToneSeries.get(), numSamples, radiansPerSample, phi );
    auto legacyPhaseStats = legacyPhasePurityAnalyzer.getStats();
    auto legacyPhaseMinMaxDev = legacyPhasePurityAnalyzer.getMinMaxDev();
    auto legacyPhasePeakAbsDev = std::max(-legacyPhaseMinMaxDev.first, legacyPhaseMinMaxDev.second );
    std::cout << "Mean Angular Rate: " << legacyPhaseStats.first << ", Variance: " << legacyPhaseStats.second << std::endl;
    std::cout << "Phase Noise: maxNegDev: " << legacyPhaseMinMaxDev.first << ", maxPosDev: "
              << legacyPhaseMinMaxDev.second << ", maxAbsDev: " << legacyPhasePeakAbsDev << std::endl;
    legacyMagPurityAnalyzer.analyzeSinusoidMagnitudeStability(pLegacyToneSeries.get(), numSamples );
    auto legacyMagStats = legacyMagPurityAnalyzer.getStats();
    auto legacySignalPower = legacyMagStats.first * legacyMagStats.first / 2;   // Should always be 0.5
    auto legacyMagMinMaxDev = legacyMagPurityAnalyzer.getMinMaxDev();
    auto legacyMagPeakAbsDev = std::max(-legacyMagMinMaxDev.first, legacyMagMinMaxDev.second );
    auto legacyMagDeltaT = t1 - t0;
    std::cout << "Mean Magnitude: " << legacyMagStats.first << ", Variance: " << legacyMagStats.second
              << ", SNR: " << 10.0 * std::log10( legacySignalPower / legacyMagStats.second ) << " dB" << std::endl;
    std::cout << "Magnitude Noise: maxNegDev: " << legacyMagMinMaxDev.first << ", maxPosDev: "
              << legacyMagMinMaxDev.second << ", maxAbsDev: " << legacyMagPeakAbsDev << std::endl;
    std::cout << "Performance for numSamples: " << numSamples
              << " is " << legacyMagDeltaT << " seconds." << std::endl;

    std::cout << std::endl;

    t0 = getClockMonotonic();
    p = pFlyingPhasorToneGenSeries.get();
    pFlyingPhasorToneGen->getSamples( numSamples, p );
    t1 = getClockMonotonic();

#if 0
    // What did we get
    for (size_t n = 0; numSamples != n; ++n )
    {
        FlyingPhasorToneGenerator::ElementType & s = p[n];
        std::cout << "n: " << n << ", x: " << real(s) << ", y: " << imag(s)
            << ", mag: " << abs(s) << ", phase: " << arg(s) << std::endl;
    }
#endif
    std::cout << "************ Flying Phasor Performance Measurements ************" << std::endl;
    flyingPhasorPhasePurityAnalyzer.analyzePhaseStability( pFlyingPhasorToneGenSeries.get(), numSamples,
        radiansPerSample, phi );
    auto flyingPhasorPhaseStats = flyingPhasorPhasePurityAnalyzer.getStats();
    auto flyingPhasorPhaseMinMaxDev = flyingPhasorPhasePurityAnalyzer.getMinMaxDev();
    auto flyingPhasorPhasePeakAbsDev = std::max(-flyingPhasorPhaseMinMaxDev.first, flyingPhasorPhaseMinMaxDev.second );
    std::cout << "Mean Angular Rate: " << flyingPhasorPhaseStats.first << ", Variance: " << flyingPhasorPhaseStats.second << std::endl;
    std::cout << "Phase Noise: maxNegDev: " << flyingPhasorPhaseMinMaxDev.first << ", maxPosDev: "
              << flyingPhasorPhaseMinMaxDev.second << ", maxAbsDev: " << flyingPhasorPhasePeakAbsDev << std::endl;
    flyingPhasorMagPurityAnalyzer.analyzeSinusoidMagnitudeStability(pFlyingPhasorToneGenSeries.get(), numSamples );
    auto flyingPhasorMagStats = flyingPhasorMagPurityAnalyzer.getStats();
    double signalPower = flyingPhasorMagStats.first * flyingPhasorMagStats.first / 2;   // Should always be 0.5
    auto flyingPhasorMagMinMaxDev = flyingPhasorMagPurityAnalyzer.getMinMaxDev();
    auto flyingPhasorMagPeakAbsDev = std::max(-flyingPhasorMagMinMaxDev.first, flyingPhasorMagMinMaxDev.second );
    auto flyingPhasorMagDeltaT = t1 - t0;
    std::cout << "Mean Magnitude: " << flyingPhasorMagStats.first << ", Variance: " << flyingPhasorMagStats.second
              << ", SNR: " << 10.0 * std::log10( signalPower / flyingPhasorMagStats.second ) << " dB" << std::endl;
    std::cout << "Magnitude Noise: maxNegDev: " << flyingPhasorMagMinMaxDev.first << ", maxPosDev: "
              << flyingPhasorMagMinMaxDev.second << ", maxAbsDev: " << flyingPhasorMagPeakAbsDev << std::endl;
    std::cout << "Performance for numSamples: " << numSamples
              << " is " << flyingPhasorMagDeltaT << " seconds." << std::endl;


    // Now for the Validating.
    // NOTE: We are not going to "Validate" performance of the Legacy Generator. It is very good, just slow.
    // Good up to some extremely high number of samples anyway.
    // We primarily run analysis on it to establish a baseline of performance metrics
    // to validate the Flying Phasor Generator against.
    int retCode = 0;
    do
    {
        // ***** Flying Phasor Phase Purity -  Mean, Variance and Peak Absolute Deviation *****
        // We are not comparing against the legacy here. Both are very good "mean" wise
        // and have extremely low variance.
        // We are simply going to verify that the difference is minuscule.
        if ( !inTolerance( flyingPhasorPhaseStats.first, radiansPerSample, 1e-15 ) )
        {
            std::cout << "Flying Phasor FAILS Mean Angular Rate Test! Expected: " << radiansPerSample
                      << ", Detected: " << flyingPhasorPhaseStats.first << std::endl;
            retCode = 1;
            break;
        }
        if ( flyingPhasorPhaseStats.second > 5e-32 )
        {
            std::cout << "Flying Phasor FAILS Angular Rate Variance Test! Expected: less than " << 5e-32
                      << ", Detected: " << flyingPhasorPhaseStats.second << std::endl;
            retCode = 2;
            break;
        }
        if ( flyingPhasorPhasePeakAbsDev > 7e-16 )
        {
            std::cout << "Flying Phasor FAILS Angular Rate Peak Absolute Deviation! Expected less than: " << 7e-16
                      << ", Detected: " << flyingPhasorPhasePeakAbsDev << std::endl;
            retCode = 3;
            break;
        }

        // ***** Flying Phasor Magnitude Purity -  Mean, Variance and Peak Absolute Deviation *****
        // We are not comparing against the legacy here. Both are very good "mean" wise
        // and have extremely low variance.
        // We are simply going to verify that the difference is minuscule.
        if ( !inTolerance( flyingPhasorMagStats.first, 1.0, 1e-15 ) )
        {
            std::cout << "Flying Phasor FAILS Mean Magnitude Test! Expected: " << 1.0
                      << ", Detected: " << flyingPhasorMagStats.first << std::endl;
            retCode = 4;
            break;
        }
        if ( flyingPhasorMagStats.second > 6.5e-33 )
        {
            std::cout << "Flying Phasor FAILS Magnitude Variance Test! Expected: less than " << 6.5e-33
                      << ", Detected: " << flyingPhasorMagStats.second << std::endl;
            retCode = 5;
            break;
        }
        if ( flyingPhasorMagPeakAbsDev > 3.5e-16 )
        {
            std::cout << "Flying Phasor FAILS Magnitude Peak Absolute Deviation! Expected less than: " << 3.5e-16
                      << ", Detected: " << flyingPhasorMagPeakAbsDev << std::endl;
            retCode = 6;
            break;
        }

    } while (false);


    exit( retCode );
    return retCode;
}
