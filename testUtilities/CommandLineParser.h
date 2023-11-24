// Created on 20220108

#ifndef TSG_COMPLEXTONEGEN_COMMANDLINEPARSER_H
#define TSG_COMPLEXTONEGEN_COMMANDLINEPARSER_H

#include <cmath>

class CommandLineParser
{
public:
    CommandLineParser() = default;
    ~CommandLineParser() = default;

    int parseCommandLine( int argc, char * argv[] );

    inline double getRadsPerSample() const { return radsPerSampleIn; }
    inline double getPhase() const { return phaseIn; }

    inline unsigned long getChunkSize() const { return chunkSizeIn; }
    inline unsigned long getNumChunks() const { return numChunksIn; }
    inline unsigned long getSkipChunks() const { return skipChunksIn; }

    enum class StreamFormat : short { Invalid=0, Text32, Text64, Bin32, Bin64 };
    StreamFormat getStreamFormat() const { return streamFormatIn; }

    inline bool getHelpFlag() const { return helpFlagIn; }
    inline bool getIncludeX() const { return includeX_In; }

private:
    double radsPerSampleIn{ M_PI / 256 };
    double phaseIn{ 0.0 };
    unsigned long chunkSizeIn{ 4096 };
    unsigned long numChunksIn{ 1 };
    unsigned long skipChunksIn{ 0 };
    bool helpFlagIn{ false };
    bool includeX_In{ false };

    StreamFormat streamFormatIn{ StreamFormat::Text64 };
};


#endif //TSG_COMPLEXTONEGEN_COMMANDLINEPARSER_H
