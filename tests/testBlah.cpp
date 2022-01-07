// Created on 20220105

#include "ComplexToneGenerator.h"

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

///@todo Not using all the arguments.
int analyzeSeries( const ComplexToneGenerator::ElementBufferTypePtr & pBuf, size_t nSamples,
                   double radiansPerSecond, double phi )
{
    StatsStateMachine rateStatsMachine{};
    StatsStateMachine magStatsMachine{};
    int retCode = 0;

    for ( size_t n=1; nSamples != n; ++n )
    {
        auto phaseA = std::arg( pBuf[ n-1 ] );
        auto phaseB = std::arg( pBuf[ n ] );
        double phaseDelta = deltaAngle( phaseA, phaseB );

//        std::cout << "phaseA = " << phaseA << ", phaseB = " << phaseB
//            << ", phaseDelta = " << phaseDelta << std::endl;
        rateStatsMachine.addSample(phaseDelta );

//        if ( 20 == n ) break;
    }
    for ( size_t n=0; nSamples != n; ++n )
    {
        magStatsMachine.addSample( std::abs( pBuf[ n ] ) );
    }

    auto rateStats = rateStatsMachine.getStats();
    std::cout << "Mean Angular Rate: " << rateStats.first << ", Variance: " << rateStats.second << std::endl;
    auto magStats = magStatsMachine.getStats();
    std::cout << "Mean Magnitude: " << magStats.first << ", Variance: " << magStats.second << std::endl;

    return retCode;
}


int main()
{
    std::cout << "Hello TSG Complex Tone Generator" << std::endl;

    // Instantiate the ComplexToneGen
//    constexpr double radiansPerSample = M_PI_2;
    double radiansPerSample = 1.0;
    double phi = 0.0;
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

    analyzeSeries( pToneGenSeries.get(), numSamples, radiansPerSample, phi );
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
        p[n] = exp( j * ( double( n ) * radiansPerSample ) );
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

    analyzeSeries( pCmplxExpSeries.get(), numSamples, radiansPerSample, phi );

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
