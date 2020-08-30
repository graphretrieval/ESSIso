#include"StringUtility.h"
#include <iostream>
#include <locale>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <cstring>

using namespace std;


std::string extract_ints(std::ctype_base::mask category, std::string str, std::ctype<char> const& facet)
{
	using std::strlen;

	char const *begin = &str.front(),
		*end   = &str.back();

	auto res = facet.scan_is(category, begin, end);

	begin = &res[0];
	end   = &res[strlen(res)];

	return std::string(begin, end);
}

std::string extract_ints(std::string str)
{
	return extract_ints(std::ctype_base::digit, str,
		std::use_facet<std::ctype<char>>(std::locale("")));
}


void String_Utility::readIntegersFromString(string stringContents, std::vector<int>& intNumbers){
	int integerNumber;
	intNumbers.clear();
	std::stringstream ss(extract_ints(stringContents));
	while(ss>>integerNumber){
		intNumbers.push_back(integerNumber);
	}
}

void String_Utility::readFloatsFromString(string stringContents, std::vector<float>& floatNumbers){
	float floatNumber;
	floatNumbers.clear();
	std::stringstream ss(stringContents);
	while(ss>>floatNumber){
		// cout << floatNumber << " ";
		floatNumbers.push_back(floatNumber);
	}
	// cout << endl;
}
