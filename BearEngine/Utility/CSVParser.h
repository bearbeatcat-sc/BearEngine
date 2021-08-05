#ifndef _CSV_PARSER_H_
#define _CSV_PARSER_H_

#include <string>
#include <fstream>
#include <sstream>
#include <vector>

class CSVParser
{
public:
	static void LoadCSV(std::vector<std::vector<std::string>>& data, const std::string& filePath);
	static void SaveCSV(std::vector<std::vector<std::string>>& data, const std::string& filePath);
	static void LoadCSV(std::vector<std::vector<std::string>>& data, const std::string& filePath, const std::string& fileName);
	static std::vector<std::string> Split(const std::string& input, char delimiter);
};

#endif