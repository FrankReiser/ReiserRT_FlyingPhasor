# ReiserRT_FlyingPhasor

Frank Reiser's C++11 implementation of a fast and accurate, sin/cos waveform pair (I/Q) generator.

## Overview
This tone generator evolved out of a desire to generate complex exponential waveforms (sinusoids) fast and accurate.
The traditional way of doing this, involved repeated calls to sin and cos functions with an advancing,
radian input argument. This produces accurate results, at least over a limited domain interval.
However, it is anything but fast.

If a continual sequence of complex values are what is required for an application.
The task of generating this sequence can be accomplished by simply rotating a phasor around the unit circle.
The only trigonometric functions required, are invoked during initialization.
After initialization, we require no more than a handful of arithmetic operations (maybe a couple handfuls).
This feat is accomplished with a sprinkle of "state data" and a little "loving care".
It is immune from domain range limits since it does not track, or care about, "cycles".
Note that this is not necessarily true for implementations of std::sin and std::cos as large radian
input values may result in instability. 

## Details
A little more needs to be said regarding the "loving care" mentioned above.
This tone generator is taking advantage of Euler's mathematics of the unit circle.
When you multiply two phasors, you multiply the magnitudes and add the angles.
When applied to unit vectors (magnitudes of one), the resultant magnitude stays one,
you simply add the angles. Our state data unit vectors are in rectangular form,
we simply preform a complex multiply in rectangular form, the resultant magnitude
of which may drift away from one. Because of this, a re-normalization cycle must be
preformed on some periodic basis and this adds to the cost. This tone generator performs
this re-normalization every other sample. This was chosen for two reasons.
One, it puts any resultant spur at the Nyquist (edge of bandwidth).
Two, because at every other sample, the re-normalization required is minimal,
keeping its noise contribution minimum. Additionally, with such small errors,
a simple and inexpensive linear approximation is all that is required to maintain stability.
Benchmarking indicates that this tone generator is approximately a factor of 5 times faster
than the traditional method. Accuracy is such that it is "almost" immeasurably worse.
You be the judge. 

Regarding the "state data", this tone generator was designed to generate a single tone per instance.
An instance is constructed with an initial frequency and phase.
When an initial number of samples are requested from an instance, they are delivered
from the starting phase angle at a fixed radians per sample, rate. Subsequent sample requests,
are delivered in phase (continuous) with the previous samples delivered. However, an instance
may be "reset", to produce a different phased tone. Doing so, re-initializes all "state data"
as if the object were just constructed. The amount of state data maintained is fairly small.
If numerous tones are simultaneously required, instantiate multiple tone generators.

# Thread Safety
This tone generator is NOT "thread safe". There are no concurrent access mechanisms
in place and there is no good reason for addressing this. To the contrary,
state left by one thread would make no sense to another, never mind the concurrency issues.
Have threads use their own unique instances.

# Acknowledgements
I cannot take credit for this algorithm. It is really just high school math.
This implementation was derived from something I saw on Stack Exchange.
What I have done is to actually utilize std::complex instead of a discrete reimplementation
of complex math, fine honed the re-normalization, and turned it into a reusable object that
meets my needs. Also, I provide some test harnesses that prove its worthiness. 

## Building and Installation
Roughly as follows:
1) Obtain a copy of the project
2) Create a build folder within the project root folder.
3) Switch directory to the build folder and run the following
   to configure and build the project for your platform:
   ```
   cmake ..
   cmake --build .
   ```
4) Test the library
   ```
   ctest
   ```
5) Install the library as follows (You'll most likely
   need root permissions to do this):
   ```
   sudo cmake --install .
   ```
