// Created on 20220108

#include "MiscTestUtilities.h"
#include <cmath>

bool inTolerance( double value, double desiredValue, double toleranceRatio )
{
    auto limitA = desiredValue * ( 1 + toleranceRatio );
    auto limitB = desiredValue * ( 1 - toleranceRatio );
    auto minValue = std::signbit( desiredValue ) ? limitA : limitB;
    auto maxValue = std::signbit( desiredValue ) ? limitB : limitA;

    return ( minValue <= value && value <= maxValue );
}

double deltaAngle( double angleA, double angleB )
{
    auto delta = angleB - angleA;
    if ( delta > M_PI ) delta -= 2*M_PI;
    else if ( delta < -M_PI ) delta += 2*M_PI;
    return delta;
}
