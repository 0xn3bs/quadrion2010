
#ifndef INC_SPLIT_H
#define INC_SPLIT_H



#pragma warning(disable:4786) 

#include <vector>
#include <string>

// This is pretty much a replica of the VB (and other languages) function Split()
std::vector <std::string> Split( const std::string& _separator, std::string _string );


std::vector <std::string> Split( const std::string& _separator, std::string _string, int _limit );

// 3rd Parameter returns the lengh
std::vector <std::string> SplitC( const std::string& _separator, std::string _string, int &length );


#endif