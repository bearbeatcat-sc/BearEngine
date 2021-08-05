#ifndef _LOG_SYSTEM_H_
#define _LOG_SYSTEM_H_

#include <string>
#include <vector>

class LogSystem
{
public:
	LogSystem();
	~LogSystem();
	static void Init();
	static void AddLog(const std::string& text);
	static void DrawLog();
	
private:
	static std::vector<std::string> m_Logs;
};

#endif