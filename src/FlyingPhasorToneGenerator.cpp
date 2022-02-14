/**
 * @file FlyingPhasorToneGenerator.cpp
 * @brief The Implementation file for the Flying Phasor Tone Generator
 *
 * @authors Frank Reiser
 * @date Initiated on Jan 05, 2022
 */


#include "FlyingPhasorToneGenerator.h"

using namespace ReiserRT::Signal;

FlyingPhasorToneGenerator::FlyingPhasorToneGenerator( double radiansPerSample, double phi )
    : rate{ std::polar( 1.0, radiansPerSample ) }
    , phasor{ ElementType{ 1.0, 0.0 } * std::polar( 1.0, phi ) }
    , sampleCounter{}
{
}

void FlyingPhasorToneGenerator::getSamples( size_t numSamples, ElementBufferTypePtr pElementBufferType )
{
    for ( size_t i = 0; numSamples != i; ++i )
    {
        // We always start with the current phasor to nail the very first sample (s0)
        // and advance (rotate) afterwards.
        *pElementBufferType++ = phasor;

        // Now advance (rotate) the phasor by our rate (complex multiply)
        phasor *= rate;

        // Re-normalize every N iterations to ensure we maintain a unit vector
        // as rounding errors accumulate. Doing this too often reduces computational performance
        // and not doing it often enough increases noise (phase and amplitude).
        // We are being pretty aggressive as it is at every 2 iterations.
        // Although, every two iterations pushes any slight modulation to the nyquist limit.
        // This means that any spectral spurs created are at the nyquist and of
        // thereby of less consequence.
        // Super-fast modulo 2 (for 4, 8, 16..., use 0x3, 0x7, 0xF...)
        if ( ( sampleCounter++ & 0x1 ) == 0x1 )
        {
            // Normally, this would require a sqrt invocation. However, when the sum of squares
            // is near a value of 1, the square root would also be near 1.
            // This is a first order Taylor Series approximation around 1 for the sqrt function.
            // The re-normalization adjustment is a scalar multiply (not complex multiply).
            const double d = 1.0 - ( phasor.real()*phasor.real() + phasor.imag()*phasor.imag() - 1.0 ) / 2.0;
            phasor *= d;
        }
    }
}

void FlyingPhasorToneGenerator::reset( double radiansPerSample, double phi )
{
    rate = std::polar( 1.0, radiansPerSample );
    phasor = ElementType{ 1.0, 0.0 } * std::polar( 1.0, phi );
    sampleCounter = 0;
}
