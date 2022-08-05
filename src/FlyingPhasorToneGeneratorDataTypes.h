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
        using FlyingPhasorPrecisionType = double;
        using FlyingPhasorElementType = std::complex< FlyingPhasorPrecisionType >;
        using FlyingPhasorElementBufferTypePtr = FlyingPhasorElementType *;
    }
}

#endif //REISERRT_FLYINGPHASOR_FLYINGPHASORTONEGENERATORDATATYPES_H
