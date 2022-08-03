//
// Created by Frank Reiser on 8/3/22.
//

#include "FlyingPhasorToneGenerator.h"

#include <memory>
#include <iostream>

int main()
{
    // An arbitrary epoch dwell in samples.
    constexpr size_t NUM_SAMPLES = 2048;

    // What sort of tones do we want to generate. We'll pick 4 and 5 thousand cycles per second.
    constexpr double freqToneA = 4000.0;
    constexpr double freqToneB = 5000.0;

    // The Sample Rate we will use here in samples per second. Roughly 65k
    constexpr double sampleRate = 65904.64;

    // Resulting radian per sample rates for toneA and toneB
    constexpr double radPerSecToneA = 2.0 * M_PI * freqToneA / sampleRate;
    constexpr double radPerSecToneB = 2.0 * M_PI * freqToneB / sampleRate;

    // Flying Phasor Tone Generators are cheap to use. They do zero dynamic allocation,
    // and contain only a minimal amount of state data. We are going to instantiate two directly on the stack.
    ReiserRT::Signal::FlyingPhasorToneGenerator toneGenA{ radPerSecToneA };
    ReiserRT::Signal::FlyingPhasorToneGenerator toneGenB{ radPerSecToneB };

    // Buffers for an epoch's worth of data for each tone.
    using SampleType = ReiserRT::Signal::FlyingPhasorToneGenerator::ElementType;
    std::unique_ptr< SampleType > pToneA{ new SampleType[ NUM_SAMPLES  ] };
    std::unique_ptr< SampleType > pToneB{ new SampleType[ NUM_SAMPLES  ] };

    // Get data from each of the tone generators.
    toneGenA.getSamples( NUM_SAMPLES, pToneA.get() );
    toneGenB.getSamples( NUM_SAMPLES, pToneB.get() );

    // Accumulate tone B into tone A.
    auto pSampleA = pToneA.get();
    auto pSampleB = pToneB.get();
    for ( size_t i = 0; NUM_SAMPLES != i; ++i )
        *pSampleA++ += *pSampleB++;

    // The result is two sinusoidal signals of amplitude 1, added together. Scaling and conversion to
    // 16bit signed integer to be done by consumer of output from the two tone result.
    // The result max deviation from zero is +/-2.0. To scale this to be withing +/-32767 integer values.
    // Suggesting half range of +/-16383 to be comfortably within acquired digital signal range, so we multiply by 8192.
    pSampleA = pToneA.get();
    for ( size_t i = 0; NUM_SAMPLES != i; ++i )
        *pSampleA++ *= 8192;

    // Print out a few samples, so we can evaluate what has transpired.
    pSampleA = pToneA.get();
    for ( size_t i = 0; 20 != i; ++i, ++pSampleA )
        std::cout << pSampleA->real() << ", " << pSampleA->imag() << std::endl;

    return 0;
}
