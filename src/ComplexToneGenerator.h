// Created on 20220105

#ifndef TSG_COMPLEXTONEGEN_COMPLEXTONEGENERATOR_H
#define TSG_COMPLEXTONEGEN_COMPLEXTONEGENERATOR_H

#include "TSG_ComplexToneGenExport.h"

#include <complex>

class TSG_ComplexToneGen_EXPORT ComplexToneGenerator
{
private:
    class Imple;

public:
    using PrecisionType = double;
    using ElementType = std::complex< PrecisionType >;
    using ElementBufferTypePtr = ElementType *;

    explicit ComplexToneGenerator( double radiansPerSample, double phi );
    ~ComplexToneGenerator();

    void getSamples( size_t numSamples, ElementBufferTypePtr pElementBufferType );
private:
    Imple * pImple;
};


#endif //TSG_COMPLEXTONEGEN_COMPLEXTONEGENERATOR_H
