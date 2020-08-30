/**
* This file wraps the utility functions for string operations
*/

#ifndef STRING_UTILITY
#define STRING_UTILITY
#include<string>
#include<vector>
class String_Utility{
public:
	/**
	* extract the integer numbers from a string
	*/
	static void readIntegersFromString(std::string, std::vector<int>&);
	static void readFloatsFromString(std::string, std::vector<float>&);
};



#endif