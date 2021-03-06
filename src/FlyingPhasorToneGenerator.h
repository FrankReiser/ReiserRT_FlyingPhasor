/**
 * @file FlyingPhasorToneGenerator.h
 * @brief The Specification file for the Flying Phasor Tone Generator
 *
 * @authors Frank Reiser
 * @date Initiated on Jan 05, 2022
 */

#ifndef REISER_RT_FLYING_PHASOR_H
#define REISER_RT_FLYING_PHASOR_H

#include "ReiserRT_FlyingPhasorExport.h"

#include <complex>

namespace ReiserRT
{
    namespace Signal
    {

        /**
         * Class FlyingPhasorToneGenerator
         *
         * This was developed to replace multiple invocations to cos( theta ) + j*sin( theta ),
         * or alternatively exp( j*theta ), with an updated value for theta each sample of every
         * complex sinusoid needed (many). All we really require is that the phase advance (or retard)
         * by some radian per sample quantity. We can advance phase by complex multiplication
         * of two unit vectors. The only trig functions necessary are one each of cos and sin
         * to initialize the complex "rate" member variable and another pair to initialize
         * the complex "phasor" from phi. These occur during construction or reset and never again.
         *
         * FlyingPhasorToneGenerator maintains state. You can ask for a quantity of samples
         * and come back later and ask for more and the waveform will pick up right where
         * where it left off. This allows for signal segments to be built up maintaining phase
         * coherency for a given instance of this class.
         *
         * Being that you construct with a radian rate per sample and an phase angle (phi).
         * You would want to construct separate instances of FlyingPhasorToneGenerator for each
         * tone/phase you require. State data is minimal so this should not be a problem
         * (i.o.w., cheap).
         *
         * NOTE: Testing indicates that this is >10x faster than traditional means
         * for generating complex sinusoidal waveforms under "release" builds.
         * The implementation file contains greater details on the mathematics
         * that make this possible.
         *
         * @todo Finish Documenting This Class
         */
        class ReiserRT_FlyingPhasor_EXPORT FlyingPhasorToneGenerator
        {
        public:
            using PrecisionType = double;
            using ElementType = std::complex< PrecisionType >;
            using ElementBufferTypePtr = ElementType *;

            explicit FlyingPhasorToneGenerator( double radiansPerSample=0.0, double phi=0.0 );
            ~FlyingPhasorToneGenerator() = default;

            void getSamples( size_t numSamples, ElementBufferTypePtr pElementBufferType );
            void reset( double radiansPerSample=0.0, double phi=0.0 );

        private:
            ElementType rate;
            ElementType phasor;
            size_t sampleCounter;
        };

    }
}


#endif //REISER_RT_FLYING_PHASOR_H
