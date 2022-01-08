// Created on 20220105

#include "ComplexToneGenerator.h"

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
// Implements the Welford's "Online" in a state machine.
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
    }

    // Currently, returns mean and variance
    std::pair<double,double> getStats() const
    {
        constexpr long double myNAN = std::numeric_limits<long double>::quiet_NaN();
        switch ( nSamples )
        {
            case 0 : return { myNAN, myNAN };
            case 1 : return { mean, myNAN };
            default : return { mean, M2 / (long double)(nSamples-1) };
        }
    }

private:
    long double mean{0.0};
    long double M2{0.0};
    size_t nSamples{0};
};

double deltaAngle( double angleA, double angleB )
{
    auto delta = angleB - angleA;
    if ( delta > M_PI ) delta -= 2*M_PI;
    else if ( delta < -M_PI ) delta += 2*M_PI;
//    delta += ( delta > M_PI ) ? -2*M_PI : ( delta < -M_PI ) ? 2*M_PI : 0;
    return delta;
}

bool inTolerance( double value, double desiredValue, double toleranceRatio )
{
    auto minValue = desiredValue * ( 1 - toleranceRatio );
    auto maxValue = desiredValue * ( 1 + toleranceRatio );

    return ( minValue <= value && value <= maxValue );
}

///@todo Not using all the arguments.
int analyzeSinusoidPhaseStability(const ComplexToneGenerator::ElementBufferTypePtr & pBuf, size_t nSamples,
                                  double radiansPerSample, double phi )
{
    StatsStateMachine rateStatsMachine{};
    int retCode = 0;

    for ( size_t n=0; nSamples != n; ++n )
    {
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
        rateStatsMachine.addSample( phaseDelta );
    }

    // Now test the statistics look good.
    auto rateStats = rateStatsMachine.getStats();
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

int analyzeSinusoidMagnitudeStability(const ComplexToneGenerator::ElementBufferTypePtr & pBuf, size_t nSamples )
{
    StatsStateMachine magStatsMachine{};
    int retCode = 0;

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

        magStatsMachine.addSample( mag );
    }

    // Now test the statistics look good.
    auto magStats = magStatsMachine.getStats();
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

int main( int argc, char * argv[])
{
    std::cout << "Hello TSG Complex Tone Generator" << std::endl;

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

    // Instantiate the ComplexToneGen
//    constexpr double radiansPerSample = M_PI_2;
    double radiansPerSample = cmdLineParser.getRadsPerSample();
    double phi = cmdLineParser.getPhase();
    std::unique_ptr< ComplexToneGenerator > pComplexToneGen{ new ComplexToneGenerator{ radiansPerSample, phi } };

    // Create buffers for a larger than needed number of samples
    const size_t maxSamples = 8192;
    std::unique_ptr< ComplexToneGenerator::ElementType[] > pToneGenSeries{new ComplexToneGenerator::ElementType [ maxSamples] };
    std::unique_ptr< ComplexToneGenerator::ElementType[] > pCmplxExpSeries{new ComplexToneGenerator::ElementType [ maxSamples] };

    std::cout << std::scientific;
    std::cout.precision(17);

    // Ask for some samples from the complex tone generator
    size_t numSamples = 4096;
    double t0, t1;
    t0 = getClockMonotonic();
    ComplexToneGenerator::ElementBufferTypePtr p = pToneGenSeries.get();
    pComplexToneGen->getSamples( numSamples, p );
    t1 = getClockMonotonic();
    std::cout << "New Generator Performance for numSamples: " << numSamples
        << " is " << t1-t0 << " seconds." << std::endl;

    analyzeSinusoidPhaseStability(pToneGenSeries.get(), numSamples, radiansPerSample, phi);
    analyzeSinusoidMagnitudeStability(pToneGenSeries.get(), numSamples);
#if 0
    // What did we get
    for (size_t n = 0; numSamples != n; ++n )
    {
        ComplexToneGenerator::ElementType & s = p[n];
        std::cout << "n: " << n << ", x: " << real(s) << ", y: " << imag(s)
            << ", mag: " << abs(s) << ", phase: " << arg(s) << std::endl;
    }
#endif

    // New, Old-Fashioned Way.
    // Loop on complex exponential
    constexpr ComplexToneGenerator::ElementType j{ 0.0, 1.0 };
    p = pCmplxExpSeries.get();
    t0 = getClockMonotonic();
    for (size_t n = 0; numSamples != n; ++n )
    {
        p[n] = exp( j * ( double( n ) * radiansPerSample + phi ) );
    }
    t1 = getClockMonotonic();
    std::cout << "Old Generator Performance for numSamples: " << numSamples
              << " is " << t1-t0 << " seconds." << std::endl;
#if 0
    // What did we get
    for (size_t n = 0; numSamples != n; ++n )
    {
        ComplexToneGenerator::ElementType & s = p[n];
        std::cout << "n: " << n << ", x: " << real(s) << ", y: " << imag(s)
                  << ", mag: " << abs(s) << ", phase: " << arg(s) << std::endl;
    }
#endif

    analyzeSinusoidPhaseStability(pCmplxExpSeries.get(), numSamples, radiansPerSample, phi);
    analyzeSinusoidMagnitudeStability(pCmplxExpSeries.get(), numSamples);

// New Generator Re-normalizing every 4th sample
//n: 4095, x: -6.59759965582308316e-02, y: -9.97821210376963585e-01, mag: 1.00000000000000000e+00, phase: -1.63682028109054922e+00

    int retCode = 0;

#if 0
    // ***** Phase Noise Work *****
    std::unique_ptr< ComplexToneGenerator::PrecisionType[] > pDeltaPhaseSeries{ new ComplexToneGenerator::PrecisionType [ maxSamples ] };
    for ( size_t n = 0; numSamples != n; ++n )
    {
        double phaseA = std::arg( pCmplxExpSeries[n] );
        double phaseB = std::arg( pToneGenSeries[n] );
        double deltaPhase = phaseA - phaseB;
        if ( deltaPhase > M_PI ) deltaPhase -= 2 * M_PI;
        if ( deltaPhase <=  -M_PI ) deltaPhase += 2 * M_PI;
        pDeltaPhaseSeries[n] = deltaPhase;
        std::cout << "n: " << n << ", deltaPhase: " << pDeltaPhaseSeries[n] << std::endl;
    }
    // It does not seem practical to run statistical analysis on this as the mean is low and the variance is low.
    // That leads to instability using numerical methods.
#endif

#if 0
    // Get samples a bunch more times to find out if this thing ever breaks
    for ( size_t epoch = 0; 3000 != epoch; ++epoch )
    {
        pComplexToneGen->getSamples( numSamples, p );
    }
    for (size_t n = 0; numSamples != n; ++n )
    {
        ComplexToneGenerator::ElementType & s = p[n];
        std::cout << "n: " << n << ", x: " << real(s) << ", y: " << imag(s)
                  << ", mag: " << abs(s) << ", phase: " << arg(s) << std::endl;
    }
#endif

#if 0
    StatsStateMachine blah{};
    blah.addSample( 3 );
    blah.addSample( 4 );
    blah.addSample( 3 );
    blah.addSample( 5 );
    blah.addSample( 3 );
    blah.addSample( 4 );
    blah.addSample( 5 );
    auto res = blah.getStats();
    std::cout << "Blah: mean=" << res.first << ", variance=" << res.second << std::endl;
#endif

    exit( retCode );
    return retCode;
}
