
#include "CommandLineParser.h"

#include <iostream>
#include <cstdlib>

#include <getopt.h>

#if 0
///@note Unfortunately, we are sort of stuck in the C98 world here and strtol does not offer a lot of ways
/// to validate. It returns zero if no conversion is able to occur but this is a valid number in a lot of cases.
int getIntArg( const char * pIntArgStr )
{
    return strtol( pIntArgStr, 0, 10 );
}
#endif

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
                {"radsPerSample", optional_argument, nullptr, RadsPerSample },
                {"phase", optional_argument, nullptr, Phase },
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
                std::cout << "The getopt_long call detected the --radsPerSample=" << optarg
                          << ". Value extracted = " << radsPerSampleIn << "." << std::endl;

                ///@todo Validate limit on nyquist pi(rads)/sample? Maybe not.

                break;
            case Phase:
                phaseIn = std::stod( optarg );
                std::cout << "The getopt_long call detected the --phase=" << optarg
                          << ". Value extracted = " << phaseIn << "." << std::endl;

                ///@todo Validate that Rads Per Sample must be non-zero to specify phase? Maybe not.

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
