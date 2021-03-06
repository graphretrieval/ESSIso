
#include"InputCommandLineParser.h"

// char* InputCommandLineParser::getCmdOption(int argc, char * argv[], const std::string & option)
// {
//     char ** itr = std::find(argv, argv + argc, option);
//     if (itr != argv + argc && ++itr != argv + argc){
//         return *itr;
//     }
//     return 0;
// }

std::string InputCommandLineParser::getCmdOption(const std::vector<std::string> & args, const std::string & option)
{
    auto itr = std::find(args.begin(), args.end(), option);
    if (itr != args.end() && ++itr != args.end())
    {
        return *itr;
    }
    return 0;
}

// bool InputCommandLineParser::cmdOptionExists(int argc, char * argv[], const std::string & option){
//     return std::find(argv, argv + argc, option) != argv + argc;
// }

bool InputCommandLineParser::cmdOptionExists(const std::vector<std::string> & args, const std::string& option)
{
    return std::find(args.begin(), args.end(), option) != args.end();
}