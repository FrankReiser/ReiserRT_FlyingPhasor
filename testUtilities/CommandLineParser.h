// Created on 20220108

#ifndef TSG_COMPLEXTONEGEN_COMMANDLINEPARSER_H
#define TSG_COMPLEXTONEGEN_COMMANDLINEPARSER_H

class CommandLineParser
{
public:
    CommandLineParser() = default;
    ~CommandLineParser() = default;

    int parseCommandLine( int argc, char * argv[] );

    inline double getRadsPerSample() const { return radsPerSampleIn; }
    inline double getPhase() const { return phaseIn; }

private:
    double radsPerSampleIn{ 1.0 };
    double phaseIn{ 0.0 };
};


#endif //TSG_COMPLEXTONEGEN_COMMANDLINEPARSER_H
