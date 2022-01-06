// Created on 20220105

#include "ComplexToneGenerator.h"

class ComplexToneGenerator::Imple
{
private:
    friend class ComplexToneGenerator;

    Imple( double theRadiansPerSample, double phi )
#if 0
        : dx{ std::cos( theRadiansPerSample ) }
        , dy{ std::sin( theRadiansPerSample ) }
        , sampleCounter{}
#else
        : rate{ std::cos( theRadiansPerSample ), std::sin( theRadiansPerSample ) }
        , phasor{ ElementType{ 1.0, 0.0 } * std::polar( 1.0, phi ) }
        , sampleCounter{}
#endif
    {
    }

    ~Imple() {}

    void getSamples( size_t numSamples, ElementBufferTypePtr pElementBufferType )
    {
        for ( size_t i = 0; numSamples != i; ++i )
        {
            // We always start with the current phasor to nail the very first sample (s0).
            *pElementBufferType++ = phasor;

            // Now advance the phasor by our rate.
            phasor *= rate;

#if 0
            // Re-normalize each iteration. It may be unnecessary but, it is cheap enough.
            const double d = 1.0 - ( phasor.real()*phasor.real() + phasor.imag()*phasor.imag() - 1.0 ) / 2.0;
            phasor *= d;
#else
            // Re-normalize every N iterations.
            if ( ( ++sampleCounter % 4 ) == 0 )
            {
                const double d = 1.0 - ( phasor.real()*phasor.real() + phasor.imag()*phasor.imag() - 1.0 ) / 2.0;
                phasor *= d;

            }
#endif
        }
    }

#if 0
    double dx;
    double dy;
#else
    const ElementType rate;
    ElementType phasor;
    size_t sampleCounter;
#endif
};

ComplexToneGenerator::ComplexToneGenerator( double radiansPerSample, double phi )
  : pImple{ new Imple{ radiansPerSample, phi } }
{
}

ComplexToneGenerator::~ComplexToneGenerator()
{
    delete pImple;
}

void ComplexToneGenerator::getSamples( size_t numSamples, ElementBufferTypePtr pElementBufferType )
{
    pImple->getSamples( numSamples, pElementBufferType );
}
