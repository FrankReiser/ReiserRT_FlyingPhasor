#include "FlyingPhasorToneGenerator.h"

#include <iostream>
#include <memory>

int main()
{
//    double radiansPerSample = cmdLineParser.getRadsPerSample();
//    double phi = cmdLineParser.getPhase();
    double radiansPerSample = 1.0;
    double phi = 0.0;

    // Generate Samples
    std::unique_ptr< FlyingPhasorToneGenerator > pFlyingPhasorToneGen{ new FlyingPhasorToneGenerator{ radiansPerSample, phi } };
    constexpr size_t numSamples = 4096;
    std::unique_ptr< FlyingPhasorToneGenerator::ElementType[] > pToneSeries{new FlyingPhasorToneGenerator::ElementType [ numSamples] };
    constexpr FlyingPhasorToneGenerator::ElementType j{ 0.0, 1.0 };
    FlyingPhasorToneGenerator::ElementBufferTypePtr p = pToneSeries.get();
    pFlyingPhasorToneGen->getSamples( numSamples, p );

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
