#include "display_builder.hpp"
#include "game.hpp"

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before mhMainWnd is valid.
	return jd::Game::getGame().msgHandler(hwnd, msg, wParam, lParam);
}

namespace jd
{
	[[nodiscard]] std::pair<DisplayWin32, bool> DisplayBuilder::build()
	{
		WNDCLASSEX wc;
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc = MainWndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance_;
		wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
		wc.hIconSm = wc.hIcon;
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = appName_.data();
		wc.cbSize = sizeof(WNDCLASSEX);

		if (!RegisterClassEx(&wc))
		{
			MessageBox(0, L"RegisterClass Failed.", 0, 0);
			return { DisplayWin32{}, false };
		}

		// Compute window rectangle dimensions based on requested client area dimensions.
		RECT R = { 0, 0, (LONG)clientWidth_, (LONG)clientHeight_ };
		AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);

		auto dwStyle = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_THICKFRAME;

		auto posX = (GetSystemMetrics(SM_CXSCREEN) - clientWidth_) / 2;
		auto posY = (GetSystemMetrics(SM_CYSCREEN) - clientHeight_) / 2;

		HWND hWnd = CreateWindowEx(WS_EX_APPWINDOW, appName_.data(), appName_.data(),
			dwStyle,
			posX, posY,
			R.right - R.left,
			R.bottom - R.top,
			nullptr, nullptr, hInstance_, nullptr);

		if (!hWnd)
		{
			MessageBox(0, L"CreateWindow Failed.", 0, 0);
			return { DisplayWin32{}, false };
		}

		ShowWindow(hWnd, SW_SHOW);
		SetForegroundWindow(hWnd);
		SetFocus(hWnd);

		ShowCursor(is_cursore_);

		auto display = DisplayWin32 {
			.clientHeight = clientHeight_,
			.clientWidth = clientWidth_,
			.hInstance = hInstance_,
			.hwnd = hWnd
		};

		return { display, true };
	}

}