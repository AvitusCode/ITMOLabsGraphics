#include "string_utils.hpp"

#include <string_view>
#include <sstream>
#include <cstdarg>

#include <Windows.h>

namespace jd::strings
{
	std::wstring stringToWstring(const std::string& str)
	{
		std::wstring wstr{};
		if (str.empty()) {
			return wstr;
		}

		int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);
		if (size_needed <= 0) {
			return wstr;
		}

		wstr.resize(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), &wstr[0], size_needed);
		return wstr;
	}

	std::vector<std::string> split(std::string_view sv, char sep)
	{
		std::vector<std::string> result;

		while (true)
		{
			size_t pos = sv.find(sep);

			if (pos == sv.npos)
			{
				if (sv.size() > 0) {
					result.push_back(std::string(sv));
				}

				break;
			}

			std::string_view temp = sv.substr(0, pos);
			if (temp.size() != 0) {
				result.push_back(std::string(temp));
			}

			sv.remove_prefix(pos + 1);
		}

		return result;
	}


	std::string upDirectory(std::string_view sv, char slash)
	{
		bool isTrailingSlash = sv.back() == slash;
		std::vector<std::string> subPath = split(sv, slash);
		std::string result{};

		for (size_t i = 0; i < subPath.size() - 1; i++)
		{
			if (i > 0) {
				result += slash;
			}
			result += subPath[i];
		}

		if (isTrailingSlash && !result.empty()) {
			result += slash;
		}

		return result;
	}

	std::string makeCorrectSlashes(std::string_view path, char slash)
	{
		std::string result = std::string(path);

		for (char& c : result) {
			if (c == '\\' || c == '/') {
				c = slash;
			}
		}

		return result;
	}

	std::string getFileName(std::string_view path)
	{
		size_t indx = path.find_last_of('\\');
		if (indx == path.npos) {
			indx = path.find_last_of('/');
		}

		return indx == path.npos ? "" : std::string(path.substr(indx + 1));
	}

	std::string getDirPath(std::string_view path)
	{
		size_t indx = path.find_last_of('\\');
		if (indx == path.npos) {
			indx = path.find_last_of('/');
		}

		if (indx == path.npos) {
			return "";
		}

		return std::string(path.substr(0, indx + 1));
	}

	std::string print(const char* fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		int needed = vsnprintf(nullptr, 0, fmt, ap);
		va_end(ap);
		std::string result(needed + 1, '\0');
		va_start(ap, fmt);
		vsnprintf(result.data(), result.size(), fmt, ap);
		va_end(ap);
		result.pop_back(); // remove last '\0'

		return result;
	}
}