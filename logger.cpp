#include "logger.hpp"

#include <iostream>
#include <fstream>

#include <windows.h>

#include "string_utils.hpp"

namespace
{
	class DefaultFileLogger final
	{
	public:
		DefaultFileLogger();
		void toFile(std::string_view str);
		~DefaultFileLogger();

	private:
		std::ofstream file;
	};

	DefaultFileLogger::DefaultFileLogger()
	{
		if (DWORD fileAttrib = GetFileAttributesA("log"); fileAttrib == INVALID_FILE_ATTRIBUTES)
		{
			if (!CreateDirectoryA("log", nullptr))
			{
				std::abort();
			}
		}

		file.open("log/render_project_log.txt", std::ios::out | std::ios::trunc);
	}

	void DefaultFileLogger::toFile(std::string_view str) {
		file << str;
	}

	DefaultFileLogger::~DefaultFileLogger() {
		file << std::endl;
	}
}

namespace jd::logging
{
	const char* LoggerError::name() const noexcept {
		return "Logger errors";
	}

	std::string LoggerError::message(int err) const {
		switch (err)
		{
		case 0: return "[INFO]: ";
		case 1: return "[WARNING]: ";
		case 2: return "[ERROR]: ";
		case 3: return "[FATAL]: ";
		}

		__assume(0);
	}

	std::error_code make_error_code(LogErrors err) noexcept {
		return std::error_code(static_cast<int>(err), LoggerError::instance());
	}

	void DefaultLogStd(std::string_view str) {
		std::cerr << str;
	}

	void DefaultLogFile(std::string_view str) {
		static DefaultFileLogger fileLogger{};
		fileLogger.toFile(str);
	}

	void DefaultPrefixFunc(std::ostream& os, std::string_view info, int num)
	{
		SYSTEMTIME lt;
		GetLocalTime(&lt);
		os << jd::strings::print("LOCAL TIME %02d/%02d/%02d %02d:%02d:%02d ", lt.wDay, lt.wMonth, lt.wYear, lt.wHour, lt.wMinute, lt.wSecond);
		os << jd::strings::getFileName(info) << ":" << num << " ";
	}
}