#pragma once

#include <exception>
#include <string>

#include <Windows.h>

namespace jd::exception
{
    class DxException final : public std::exception
    {
    public:
        DxException() = default;
        DxException(HRESULT hr, std::string functionName, std::string filename, int lineNumber);

        const char* what() const noexcept override;
    private:
        std::string errorMsg_;
    };
}

#define ThrowIfFailed(x)                                              \
{                                                                     \
    if(HRESULT hr__ = (x); FAILED(hr__)) { throw jd::exception::DxException(hr__, #x, __FILE__, __LINE__); } \
}