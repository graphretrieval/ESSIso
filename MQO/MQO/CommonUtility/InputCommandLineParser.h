#ifndef INPUT_COMMAND_PARSER
#define INPUT_COMMAND_PARSER
#include <algorithm>
#include<string>
#include <vector>

class InputCommandLineParser{

public:

	/* parse the input command line */
	// static char* getCmdOption(int argc, char * argv[], const std::string & option);
	static std::string getCmdOption(const std::vector<std::string> & args, const std::string & option);

	// static bool cmdOptionExists(int argc, char * argv[], const std::string & option);
	static bool cmdOptionExists(const std::vector<std::string> & args, const std::string& option);


};
#endif