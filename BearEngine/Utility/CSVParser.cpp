#include "CSVParser.h"

#include <cassert>
#include <filesystem>

namespace fs = std::filesystem;

void CSVParser::LoadCSV(std::vector< std::vector<std::string>>& data,const std::string& filePath)
{

	std::filesystem::path path = std::filesystem::path(filePath);
	path = path.generic_string();
	
	std::ifstream ifs(path);

	if(ifs)
	{
		std::string line;

		while (std::getline(ifs, line))
		{
			std::vector<std::string> str = Split(line, ',');
			data.push_back(str);
		}

	}
	else
	{
		assert(0);
	}
	

}

void CSVParser::SaveCSV(std::vector<std::vector<std::string>>& data, const std::string& filePath)
{
	std::filesystem::path path = std::filesystem::path(filePath);
	path = path.generic_string();

	std::ofstream ofs(path);

	if(ofs)
	{
		for (int y = 0; y < data.size(); ++y)
		{
			for (int x = 0; x < data[y].size(); ++x)
			{
				ofs << data[y][x];
				ofs << ',';
			}

			ofs << '\n';
		}

		ofs.close();
		
	}
	else
	{
		assert(0);
	}
	

}

// Œã‚ÅŽÀ‘•‚·‚é
void CSVParser::LoadCSV(std::vector<std::vector<std::string>>& data, const std::string& filePath,const std::string& fileName)
{
	//std::ifstream ifs;

	//std::filesystem::path path = std::filesystem::path(filePath);
	//std::filesystem::path filename = path.filename();

	//path = "Resources/";
	//path += filename;

	//ifs.open(path);

	//if (ifs.is_open())
	//{
	//	std::string line;

	//	while (std::getline(ifs, line))
	//	{
	//		std::vector<std::string> str = Split(line, ',');
	//		data.push_back(str);
	//	}

	//}
	//else
	//{
	//	assert(0);
	//}

}

std::vector<std::string> CSVParser::Split(const std::string& input, char delimiter)
{
	std::istringstream stream(input);
	std::string field;

	std::vector<std::string> result;

	while (std::getline(stream, field, delimiter))
	{
		result.push_back(field);
	}

	return result;
}
