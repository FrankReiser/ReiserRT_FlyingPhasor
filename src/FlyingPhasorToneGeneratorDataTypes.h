/**
 * @file FlyingPhasorToneGeneratorDataTypes.h
 * @brief The Specification file for the Flying Phasor Tone Generator Data Types
 *
 * @authors Frank Reiser
 * @date Initiated on Aug 05, 2022
 */

#ifndef REISERRT_FLYINGPHASOR_FLYINGPHASORTONEGENERATORDATATYPES_H
#define REISERRT_FLYINGPHASOR_FLYINGPHASORTONEGENERATORDATATYPES_H

#include <complex>

namespace ReiserRT
{
    namespace Signal
    {

        /**
        * @brief Alias for Precision Type
        *
        * This is simply an alias for a double (a 64bit IEEE 754 floating point value)
        */
        using FlyingPhasorPrecisionType = double;

        /**
        * @brief Alias for Complex Number Type
        *
        * This is simply an alias for a std::complex< FlyingPhasorPrecisionType > or std::complex< double >.
        */
        using FlyingPhasorElementType = std::complex< FlyingPhasorPrecisionType >;

        /**
        * @brief Alias for Buffer Type Pointer
        *
        * This is simply an alias for a pointer type to our FlyingPhasorElementType.
        */
        using FlyingPhasorElementBufferTypePtr = FlyingPhasorElementType *;
    }
}

#endif //REISERRT_FLYINGPHASOR_FLYINGPHASORTONEGENERATORDATATYPES_H
