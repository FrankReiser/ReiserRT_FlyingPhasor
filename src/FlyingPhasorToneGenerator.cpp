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
    , phasor{ std::polar( 1.0, phi ) }
    , sampleCounter{}
{
}

void FlyingPhasorToneGenerator::getSamples( FlyingPhasorElementBufferTypePtr pElementBuffer, size_t numSamples )
{
    for ( size_t i = 0; numSamples != i; ++i )
    {
        // We always start with the current phasor to nail the very first sample (s0)
        // and advance (rotate) afterward.
        *pElementBuffer++ = phasor;

        // Now advance (rotate) the phasor by our rate (complex multiply)
        phasor *= rate;

        // Perform normalization
        normalize();
    }
}

void FlyingPhasorToneGenerator::getSamplesScaled( FlyingPhasorElementBufferTypePtr pElementBuffer, size_t numSamples,
        double scalar )
{
    for ( size_t i = 0; numSamples != i; ++i )
    {
        // We always start with the current phasor to nail the very first sample (s0)
        // and advance (rotate) afterward.
        *pElementBuffer++ = phasor * scalar;

        // Now advance (rotate) the phasor by our rate (complex multiply)
        phasor *= rate;

        // Perform normalization
        normalize();
    }
}

void FlyingPhasorToneGenerator::getSamplesScaled( FlyingPhasorElementBufferTypePtr pElementBuffer, size_t numSamples,
        const double * pScalars )
{
    for ( size_t i = 0; numSamples != i; ++i )
    {
        // We always start with the current phasor to nail the very first sample (s0)
        // and advance (rotate) afterward.
        *pElementBuffer++ = phasor * *pScalars++;

        // Now advance (rotate) the phasor by our rate (complex multiply)
        phasor *= rate;

        // Perform normalization
        normalize();
    }
}

void FlyingPhasorToneGenerator::accumSamples( FlyingPhasorElementBufferTypePtr pElementBuffer, size_t numSamples )
{
    for ( size_t i = 0; numSamples != i; ++i )
    {
        // We always start with the current phasor to nail the very first sample (s0)
        // and advance (rotate) afterward.
        *pElementBuffer++ += phasor;

        // Now advance (rotate) the phasor by our rate (complex multiply)
        phasor *= rate;

        // Perform normalization
        normalize();
    }
}

void FlyingPhasorToneGenerator::accumSamplesScaled( FlyingPhasorElementBufferTypePtr pElementBuffer, size_t numSamples,
                   double scalar )
{
    for ( size_t i = 0; numSamples != i; ++i )
    {
        // We always start with the current phasor to nail the very first sample (s0)
        // and advance (rotate) afterward.
        *pElementBuffer++ += phasor * scalar;

        // Now advance (rotate) the phasor by our rate (complex multiply)
        phasor *= rate;

        // Perform normalization
        normalize();
    }
}

void FlyingPhasorToneGenerator::accumSamplesScaled( FlyingPhasorElementBufferTypePtr pElementBuffer, size_t numSamples,
                   const double * pScalars )
{
    for ( size_t i = 0; numSamples != i; ++i )
    {
        // We always start with the current phasor to nail the very first sample (s0)
        // and advance (rotate) afterward.
        *pElementBuffer++ += phasor * *pScalars++;

        // Now advance (rotate) the phasor by our rate (complex multiply)
        phasor *= rate;

        // Perform normalization
        normalize();
    }
}

void FlyingPhasorToneGenerator::reset( double radiansPerSample, double phi )
{
    rate = std::polar( 1.0, radiansPerSample );
    phasor = std::polar( 1.0, phi );
    sampleCounter = 0;
}

FlyingPhasorElementType FlyingPhasorToneGenerator::getSample()
{
    // We always start with the current phasor to nail the very first sample (s0)
    // and advance (rotate) afterward.
    auto retValue = phasor;

    // Now advance (rotate) the phasor by our rate (complex multiply)
    phasor *= rate;

    // Perform normalization
    normalize();

    return retValue;
}
