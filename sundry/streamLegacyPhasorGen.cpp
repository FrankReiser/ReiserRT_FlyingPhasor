// Created on 20220109

#include "FlyingPhasorToneGenerator.h"

#include "CommandLineParser.h"

#include <iostream>
#include <memory>

using namespace ReiserRT::Signal;

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
#if 0
    else
    {
        std::cout << "Parsed: --radiansPerSample=" << cmdLineParser.getRadsPerSample()
                  << " --phase=" << cmdLineParser.getPhase() << std::endl << std::endl;
    }
#endif
    double radiansPerSample = cmdLineParser.getRadsPerSample();
    double phi = cmdLineParser.getPhase();

    // Generate Samples
    constexpr size_t numSamples = 4096;
    std::unique_ptr< FlyingPhasorElementType[] > pToneSeries{new FlyingPhasorElementType [ numSamples] };
    constexpr FlyingPhasorElementType j{0.0, 1.0 };
    FlyingPhasorElementBufferTypePtr p = pToneSeries.get();
    for ( size_t n = 0; numSamples != n; ++n )
    {
        // Legacy Complex Exponential Equivalent
        p[n] = exp( j * ( double( n ) * radiansPerSample + phi ) );
    }

    // Write to standard out. It can be redirected.
    std::cout << std::scientific;
    std::cout.precision(17);
    for ( size_t n = 0; numSamples != n; ++n )
    {
        std::cout << p[n].real() << " " << p[n].imag() << std::endl;
    }

    exit( 0 );
    return 0;
}
