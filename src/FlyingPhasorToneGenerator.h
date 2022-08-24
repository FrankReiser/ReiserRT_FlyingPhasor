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
#include "FlyingPhasorToneGeneratorDataTypes.h"

namespace ReiserRT
{
    namespace Signal
    {
        /**
         * Class FlyingPhasorToneGenerator
         *
         * This was developed to replace multiple invocations of cos( theta ) + j*sin( theta ),
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
         * NOTE: Testing indicates that this is 5 to 10 times faster than traditional means
         * for generating complex sinusoidal waveforms under "release" builds.
         * The implementation file contains greater details on the mathematics
         * that make this possible.
         */
        class ReiserRT_FlyingPhasor_EXPORT FlyingPhasorToneGenerator
        {
        public:
            /**
             * @brief Construct a Flying Phasor Instance
             *
             * This operation constructs a FlyingPhasorToneGenerator instance.
             *
             * @param radiansPerSample The number of radians to advance each sample (synonymous with frequency).
             * @param phi The initial phase of the state phasor.
             */
            explicit FlyingPhasorToneGenerator( double radiansPerSample=0.0, double phi=0.0 );

            /**
             * @brief Destruct a Flying Phasor Instance
             *
             * This operation is defaulted. There is nothing to do or clean up.
             */
            ~FlyingPhasorToneGenerator() = default;

            /**
             * @brief Get Samples Operation
             *
             * This operation delivers 'N' number samples from the tone generator into the user provided buffer.
             * The samples are unscaled (i.e., a magnitude of one).
             *
             * @param pElementBuffer User provided buffer large enough to hold the requested number of samples.
             * @param numSamples The number of samples to be delivered.
             */
            void getSamples( FlyingPhasorElementBufferTypePtr pElementBuffer, size_t numSamples );

            /**
             * @brief Get Samples Scaled Operation
             *
             * This operation delivers 'N' number samples from the tone generator into the user provided buffer.
             * The samples are scaled by user provided scalar.
             *
             * @param pElementBuffer User provided buffer large enough to hold the requested number of samples.
             * @param numSamples The number of samples to be delivered.
             * @param scalar The scalar to be multiplied against each sample.
             */
            void getSamplesScaled( FlyingPhasorElementBufferTypePtr pElementBuffer, size_t numSamples,
                                   double scalar );

            /**
             * @brief Get Samples Scaled Operation
             *
             * This operation delivers 'N' number samples from the tone generator into the user provided buffer.
             * The samples are scaled by user provided scalar vector.
             *
             * @param pElementBuffer User provided buffer large enough to hold the requested number of samples.
             * @param numSamples The number of samples to be delivered.
             * @param pScalars A vector of scalars at least as long as the number of samples requested.
             */
            void getSamplesScaled( FlyingPhasorElementBufferTypePtr pElementBuffer, size_t numSamples,
                                   const double * pScalars );

            /**
             * @brief Accumulate Samples Operation
             *
             * This operation accumulates 'N' number of samples from the tone generator into the user provided buffer.
             * The samples accumulated are unscaled (i.e., a magnitude of one).
             *
             * @param pElementBuffer  User provided buffer large enough to hold the requested number of samples.
             * @param numSamples The number of samples to be accumulated.
             */
            void accumSamples( FlyingPhasorElementBufferTypePtr pElementBuffer, size_t numSamples );

            /**
             * @brief Accumulate Samples Operation
             *
             * This operation accumulates 'N' number of samples from the tone generator into the user provided buffer.
             * The samples accumulated are unscaled (i.e., a magnitude of one).
             *
             * @param pElementBuffer
             * @param numSamples The number of samples to be accumulated.
             * @param scalar The scalar to be multiplied against each sample before accumulating.
             */
            void accumSamplesScaled( FlyingPhasorElementBufferTypePtr pElementBuffer, size_t numSamples,
                               double scalar );

            /**
             * @brief Accumulate Samples Operation
             *
             * This operation accumulates 'N' number of samples from the tone generator into the user provided buffer.
             * The samples accumulated are scaled by user provided scalar vector.
             *
             * @param pElementBuffer
             * @param numSamples The number of samples to be accumulated.
             * @param pScalars A vector of scalars at least as long as the number of samples requested.
             */
            void accumSamplesScaled( FlyingPhasorElementBufferTypePtr pElementBuffer, size_t numSamples,
                               const double * pScalars );

            /**
             * @brief Reset Operation
             *
             * This operation resets an instance to a known state. This is simply
             * an instantaneous phase and a fixed frequency in radians per sample.
             *
             * @param radiansPerSample The number of radians to advance each sample (synonymous with frequency).
             * @param phi The initial phase of the state phasor.
             */
            void reset( double radiansPerSample=0.0, double phi=0.0 );

        private:
            /**
             * @brief The Normalize Operation.
             *
             * Normalize every N iterations to ensure we maintain a unit vector
             * as rounding errors accumulate. Doing this too often reduces computational performance
             * and not doing it often enough increases noise (phase and amplitude).
             * We are being pretty aggressive as it is at every 2 iterations.
             * By normalizing every two iterations, we push any slight adjustments to the nyquist rate.
             * This means that any spectral spurs created are at the nyquist and hopefully of less consequence.
             * Declared inline here for efficient reuse within the implementation.
             */
            inline void normalize( )
            {
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

        private:
            FlyingPhasorElementType rate;
            FlyingPhasorElementType phasor;
            size_t sampleCounter;
        };

    }
}


#endif //REISER_RT_FLYING_PHASOR_H
