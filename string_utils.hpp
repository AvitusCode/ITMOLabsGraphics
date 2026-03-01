#pragma once
#include <vector>
#include <string>

namespace jd::strings
{
	std::wstring stringToWstring(const std::string& str);

	inline std::vector<std::string> split(std::string_view sv, char sep);

	std::string upDirectory(std::string_view sv, char slash);

	std::string makeCorrectSlashes(std::string_view path, char slash);

	std::string getFileName(std::string_view path);

	std::string getDirPath(std::string_view path);

	std::string print(const char* fmt, ...);
}