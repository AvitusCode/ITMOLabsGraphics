#include "exception.hpp"


namespace jd::exception
{
	DxException::DxException(HRESULT hr, std::string functionName, std::string filename, int lineNumber)
		: errorMsg_{ functionName + " failed in " + filename + "; line " + std::to_string(lineNumber) + "; error: " + std::to_string(hr) }
	{
	}

	const char* DxException::what() const noexcept
	{
		return errorMsg_.c_str();
	}
}