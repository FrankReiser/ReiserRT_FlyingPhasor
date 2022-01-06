// Created on 20220105

#include "ComplexToneGenerator.h"

#include <iostream>
#include <memory>
#include <cmath>

#include <ctime>
double getClockMonotonic()
{
    timespec tNow = { 0, 0 };
    clock_gettime( CLOCK_MONOTONIC, &tNow );

    return double( tNow.tv_sec ) + double( tNow.tv_nsec ) / 1e9;
}



int main()
{
    std::cout << "Hello TSG Complex Tone Generator" << std::endl;

    // Instantiate the ComplexToneGen
//    std::unique_ptr< ComplexToneGenerator > pComplexToneGen{ new ComplexToneGenerator{ M_PI_4, 0.0 } };
    constexpr double radiansPerSample = 0.7;
    std::unique_ptr< ComplexToneGenerator > pComplexToneGen{ new ComplexToneGenerator{ radiansPerSample, 0.0 } };

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

#if 0
    // What did we get
    for (size_t n = 0; numSamples != n; ++n )
    {
        ComplexToneGenerator::ElementType & s = p[n];
        std::cout << "n: " << n << ", x: " << real(s) << ", y: " << imag(s)
            << ", mag: " << abs(s) << ", phase: " << arg(s) << std::endl;
    }
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

//n: 4095, x: 2.01898937688913371e-01, y: 9.79406360485824190e-01, mag: 9.99999999999999889e-01, phase: 1.36749992610856652e+00
    int retCode = 0;

    // ***** Phase Noise Work *****
    std::unique_ptr< ComplexToneGenerator::PrecisionType[] > pDeltaPhaseSeries{ new ComplexToneGenerator::PrecisionType [ maxSamples ] };
    for ( size_t n = 0; numSamples != n; ++n )
    {
        double phaseA = std::arg( pCmplxExpSeries[n] );
        double phaseB = std::arg( pToneGenSeries[n] );
        pDeltaPhaseSeries[n] = ( phaseA + M_PI ) - (phaseB + M_PI );
        std::cout << "n: " << n << ", deltaPhase: " << pDeltaPhaseSeries[n] << std::endl;
    }
    // It does not seem practical to run statistical analysis on this as the mean is low and the variance is low.
    // That leads to instability using numerical methods.


    exit( retCode );
    return retCode;
}
