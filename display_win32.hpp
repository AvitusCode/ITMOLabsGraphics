#pragma once
#include "display_interface.hpp"
#include "jd_types.hpp"

#include <string>

#include <Windows.h>

namespace jd
{
	struct DisplayWin32
	{
		uint32_t clientHeight;
		uint32_t clientWidth;
		float aspect;

		HINSTANCE hInstance;
		HWND hwnd;
	};
}