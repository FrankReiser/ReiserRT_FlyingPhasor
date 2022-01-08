// Created on 20220105

#include "ComplexToneGenerator.h"

class ComplexToneGenerator::Imple
{
private:
    friend class ComplexToneGenerator;

    /**
     * Constructor for ComplexToneGenerator::Imple
     *
     * We store the radians per sample argument in rectangular form as a unit vector.
     * This stores how much the tracked "phasor" is rotated each sample.
     * We also initialize our phasor as a unit vector rotated from 0 by
     * phi radians. This phasor gets updated each sample delivered with
     * the getSamples operation.
     *
     * @param theRadiansPerSample How many radians to rotate per sample
     * @param phi The initial phase angle offset.
     */
    Imple( double theRadiansPerSample, double phi )
        : rate{ std::polar( 1.0, theRadiansPerSample ) }
        , phasor{ ElementType{ 1.0, 0.0 } * std::polar( 1.0, phi ) }
        , sampleCounter{}
    {
    }

    ~Imple() {}

    void getSamples( size_t numSamples, ElementBufferTypePtr pElementBufferType )
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
