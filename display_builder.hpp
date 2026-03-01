#pragma once
#include "jd_types.hpp"

#include <string>
#include <utility>

#include <Windows.h>

#include "display_win32.hpp"

namespace jd
{
	class DisplayBuilder final
	{
	public:
		[[nodiscard]] std::pair<DisplayWin32, bool> build();

		DisplayBuilder& withHeight(uint32_t height) noexcept {
			clientHeight_ = height;
			return *this;
		}

		DisplayBuilder& withWidth(uint32_t width) noexcept {
			clientWidth_ = width;
			return *this;
		}

		DisplayBuilder& withAppName(std::wstring_view appName) noexcept {
			appName_ = appName;
			return *this;
		}

		DisplayBuilder& withHINSTANCE(HINSTANCE hInstance) noexcept {
			hInstance_ = hInstance;
			return *this;
		}

		DisplayBuilder& withCursore(bool state) noexcept {
			is_cursore_ = state;
			return *this;
		}

	private:
		uint32_t clientHeight_;
		uint32_t clientWidth_;
		std::wstring_view appName_;
		HINSTANCE hInstance_;
		bool is_cursore_{false};
	};
}