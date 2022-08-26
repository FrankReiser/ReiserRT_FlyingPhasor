
#include "CommandLineParser.h"

#include <iostream>

#include <getopt.h>

int CommandLineParser::parseCommandLine( int argc, char * argv[] )
{
    int c;
//    int digitOptIndex = 0;
    int retCode = 0;

    enum eOptions { RadsPerSample=1, Phase=2 };

    while (true) {
//        int thisOptionOptIndex = optind ? optind : 1;
        int optionIndex = 0;
        static struct option longOptions[] = {
                {"radsPerSample", required_argument, nullptr, RadsPerSample },
                {"phase", required_argument, nullptr, Phase },
                {nullptr, 0, nullptr, 0 }
        };

        c = getopt_long(argc, argv, "",
                        longOptions, &optionIndex);
        if (c == -1) {
            break;
        }

        switch (c) {
            case RadsPerSample:
                radsPerSampleIn = std::stod( optarg );
#if 0
                std::cout << "The getopt_long call detected the --radsPerSample=" << optarg
                          << ". Value extracted = " << radsPerSampleIn << "." << std::endl;
#endif
                break;
            case Phase:
                phaseIn = std::stod( optarg );
#if 0
                std::cout << "The getopt_long call detected the --phase=" << optarg
                          << ". Value extracted = " << phaseIn << "." << std::endl;
#endif
                break;
            case '?':
                std::cout << "The getopt_long call returned '?'" << std::endl;
                retCode = -1;
                break;
            default:
                std::cout << "The getopt_long call returned character code" << c << std::endl;
                retCode = -1;
                break;
        }
    }

    return retCode;
}

#if 0
int main( int argc, char * argv[] ) {

    std::cout << "Hello, World!" << " The value of argc is " << argc << std::endl;

    if (1 < argc)
    {
        if ( 0 != parseCommandLine(argc, argv) )
        {
            std::cout << "Failed parsing command line" << std::endl;
            return -1;
        }
    }
    else
    {
        std::cout << "No program arguments, entering interactive mode" << std::endl;
    }

    return 0;
}
#endif
