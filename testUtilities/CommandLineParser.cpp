
#include "CommandLineParser.h"

#include <iostream>

#include <getopt.h>

int CommandLineParser::parseCommandLine( int argc, char * argv[] )
{
    int c;
//    int digitOptIndex = 0;
    int retCode = 0;

    enum eOptions { RadsPerSample=1, Phase, ChunkSize, NumChunks, SkipChunks, StreamFormat, Help, IncludeX };

    // While options still left to parse
    while (true) {
//        int thisOptionOptIndex = optind ? optind : 1;
        int optionIndex = 0;
        static struct option longOptions[] = {
                {"radsPerSample", required_argument, nullptr, RadsPerSample },
                {"phase", required_argument, nullptr, Phase },
                {"chunkSize", required_argument, nullptr, ChunkSize },
                {"numChunks", required_argument, nullptr, NumChunks },
                {"skipChunks", required_argument, nullptr, SkipChunks },
                {"streamFormatIn", required_argument, nullptr, StreamFormat },
                {"help", no_argument, nullptr, Help },
                {"includeX", no_argument, nullptr, IncludeX },
                {nullptr, 0, nullptr, 0 }
        };

        c = getopt_long(argc, argv, "",
                        longOptions, &optionIndex);

        // When getopt_long has completed the parsing of the command line, it returns -1.
        if (c == -1) {
            break;
        }

        switch (c) {
            case RadsPerSample:
                radsPerSampleIn = std::stod( optarg );
                break;

            case Phase:
                phaseIn = std::stod( optarg );
                break;

            case ChunkSize:
                chunkSizeIn = std::stoul( optarg );
                break;

            case NumChunks:
                numChunksIn = std::stoul( optarg );
                break;

            case SkipChunks:
                skipChunksIn = std::stoul( optarg );
                break;

            case StreamFormat:
            {
                // This one is more complicated. We either detect a valid string here, or we don't.
                const std::string streamFormatStr{ optarg };
                if ( streamFormatStr == "t32" )
                    streamFormatIn = StreamFormat::Text32;
                else if ( streamFormatStr == "t64" )
                    streamFormatIn = StreamFormat::Text64;
                else if ( streamFormatStr == "b32" )
                    streamFormatIn = StreamFormat::Bin32;
                else if ( streamFormatStr == "b64" )
                    streamFormatIn = StreamFormat::Bin64;
                else
                    streamFormatIn = StreamFormat::Invalid;
                break;
            }

            case Help:
                helpFlagIn = true;
                break;

            case IncludeX:
                includeX_In = true;
                break;

            case '?':
//                std::cout << "The getopt_long call returned '?'" << std::endl;
                retCode = 1;
                break;
            default:
//                std::cout << "The getopt_long call returned character code" << c << std::endl;
                retCode = 2;
                break;
        }
    }

    return retCode;
}
