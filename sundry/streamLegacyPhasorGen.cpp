// Created on 20220109

#include "FlyingPhasorToneGenerator.h"

#include "CommandLineParser.h"

#include <iostream>
#include <memory>

using namespace ReiserRT::Signal;

void printHelpScreen()
{
    std::cout << "Usage:" << std::endl;
    std::cout << "    streamLegacyPhasorGen [options]" << std::endl;
    std::cout << "Available Options:" << std::endl;
    std::cout << "    --help" << std::endl;
    std::cout << "        Displays this help screen and exits." << std::endl;
    std::cout << "    --radsPerSample=<double>" << std::endl;
    std::cout << "        The number of radians per sample to be generated." << std::endl;
    std::cout << "        Defaults to pi/256 radians per sample if unspecified." << std::endl;
    std::cout << "    --phase=<double>" << std::endl;
    std::cout << "        The initial phase of the starting sample in radians." << std::endl;
    std::cout << "        Defaults to 0.0 radians if unspecified." << std::endl;
    std::cout << "    --chunkSize=<uint>" << std::endl;
    std::cout << "        The number of samples to produce per chunk. If zero, no samples are produced." << std::endl;
    std::cout << "        Defaults to 4096 radians if unspecified." << std::endl;
    std::cout << "    --numChunks=<uint>" << std::endl;
    std::cout << "        The number of chunks to generate. If zero, runs continually." << std::endl;
    std::cout << "        Defaults to 1 radians if unspecified." << std::endl;
    std::cout << "    --streamFormat=<string>" << std::endl;
    std::cout << "        t32 - Outputs samples in text format with floating point precision of (9 decimal places)." << std::endl;
    std::cout << "        t64 - Outputs samples in text format with floating point precision (17 decimal places)." << std::endl;
    std::cout << "        b32 - Outputs data in raw binary with 32bit precision (uint32 and float), native endian-ness." << std::endl;
    std::cout << "        b64 - Outputs data in raw binary 64bit precision (uint64 and double), native endian-ness." << std::endl;
    std::cout << "        Defaults to t64 if unspecified." << std::endl;
    std::cout << "    --includeX" << std::endl;
    std::cout << "        Include sample count in the output stream. This is useful for gnuplot using any format" << std::endl;
    std::cout << "        Defaults to no inclusion if unspecified." << std::endl;
    std::cout << std::endl;
    std::cout << "Error Returns:" << std::endl;
    std::cout << "    1 - Command Line Parsing Error - Unrecognized Long Option." << std::endl;
    std::cout << "    2 - Command Line Parsing Error - Unrecognized Short Option (none supported)." << std::endl;
    std::cout << "    3 - Invalid Stream Format Specified" << std::endl;
}

int main( int argc, char * argv[] )
{
    // Parse potential command line. Defaults provided otherwise.
    CommandLineParser cmdLineParser{};

    auto parseRes = cmdLineParser.parseCommandLine(argc, argv);
    if ( 0 != parseRes )
    {
        std::cout << "streamFlyingPhasorGen Parse Error: Use command line argument --help for instructions" << std::endl;
        exit(parseRes);
    }

    if ( cmdLineParser.getHelpFlag() )
    {
        printHelpScreen();
        exit( 0 );
    }

    // If one of the text formats, set output precision appropriately
    auto streamFormat = cmdLineParser.getStreamFormat();
    if ( CommandLineParser::StreamFormat::Invalid == streamFormat )
    {
        std::cout << "streamFlyingPhasorGen Error: Invalid Stream Format Specified. Use --help for instructions" << std::endl;
        exit( 3 );
    }

    // Get Frequency and Starting Phase
    auto radiansPerSample = cmdLineParser.getRadsPerSample();
    auto phi = cmdLineParser.getPhase();

    // Get Chunk Size and Number of Chunks.
    const auto chunkSize = cmdLineParser.getChunkSize();
    const auto numChunks = cmdLineParser.getNumChunks();

    // Allocate Memory for Chunk Size
    std::unique_ptr< FlyingPhasorElementType[] > pToneSeries{ new FlyingPhasorElementType [ chunkSize ] };

    // If we are using a text stream format, set the output precision
    if ( CommandLineParser::StreamFormat::Text32 == streamFormat)
    {
        std::cout << std::scientific;
        std::cout.precision(9);
    }
    else if ( CommandLineParser::StreamFormat::Text64 == streamFormat)
    {
        std::cout << std::scientific;
        std::cout.precision(17);
    }

    // Are we including Sample count in the output?
    auto includeX = cmdLineParser.getIncludeX();

    constexpr FlyingPhasorElementType j{ 0.0, 1.0 };
    FlyingPhasorElementBufferTypePtr p = pToneSeries.get();
    size_t sampleCount = 0;
    for ( size_t chunk = 0; numChunks != chunk; ++chunk )
    {
        // Get Samples using complex exponential function
        for ( size_t n = 0; chunkSize != n; ++n )
        {
            // Legacy Complex Exponential Equivalent
            p[n] = exp( j * ( double( chunkSize * chunk + n ) * radiansPerSample + phi ) );
        }

        if ( CommandLineParser::StreamFormat::Text32 == streamFormat ||
             CommandLineParser::StreamFormat::Text64 == streamFormat )
        {
            for ( size_t n = 0; chunkSize != n; ++n )
            {
                if ( includeX ) std::cout << sampleCount++ << " ";
                std::cout << p[n].real() << " " << p[n].imag() << std::endl;
            }
        }
        else if ( CommandLineParser::StreamFormat::Bin32 == streamFormat )
        {
            for ( size_t n = 0; chunkSize != n; ++n )
            {
                if ( includeX )
                {
                    auto sVal = uint32_t( sampleCount++);
                    std::cout.write( reinterpret_cast< const char * >(&sVal), sizeof( sVal ) );
                }
                auto fVal = float( p[n].real() );
                std::cout.write( reinterpret_cast< const char * >(&fVal), sizeof( fVal ) );
                fVal = float( p[n].imag() );
                std::cout.write( reinterpret_cast< const char * >(&fVal), sizeof( fVal ) );
            }
        }
        else if ( CommandLineParser::StreamFormat::Bin64 == streamFormat )
        {
            for ( size_t n = 0; chunkSize != n; ++n )
            {
                if ( includeX )
                {
                    auto sVal = sampleCount++;
                    std::cout.write( reinterpret_cast< const char * >(&sVal), sizeof( sVal ) );
                }
                auto fVal = p[n].real();
                std::cout.write( reinterpret_cast< const char * >(&fVal), sizeof( fVal ) );
                fVal = p[n].imag();
                std::cout.write( reinterpret_cast< const char * >(&fVal), sizeof( fVal ) );
            }
        }
        std::cout.flush();
    }

#if 0
    // Generate Samples
    FlyingPhasorElementBufferTypePtr p = pToneSeries.get();
    for ( size_t n = 0; numSamples != n; ++n )
    {
        // Legacy Complex Exponential Equivalent
        p[n] = exp( j * ( double( n ) * radiansPerSample + phi ) );
    }

    // Write to standard out. It can be redirected.
    std::cout << std::scientific;
    std::cout.precision(17);
    for ( size_t n = 0; numSamples != n; ++n )
    {
        std::cout << p[n].real() << " " << p[n].imag() << std::endl;
    }
#endif

    exit( 0 );
    return 0;
}
