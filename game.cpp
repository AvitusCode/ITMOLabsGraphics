#include "game.hpp"

#include <windowsx.h>
#include <DirectXMath.h>

#include "config_parser.hpp"
#include "display_builder.hpp"
#include "pong_component.hpp"
#include "string_utils.hpp"
#include "exception.hpp"

#include "logger.hpp"

using Microsoft::WRL::ComPtr;

namespace jd
{
	bool Game::initialize(HINSTANCE hInstance)
	{
		if (not initMainWindow(hInstance)) {
			return false;
		}

		inputDevice_ = std::make_unique<InputDevice>(*this);

		if (not initDirect3D())
			return false;

		if (not initComponents()) {
			return false;
		}

		return true;
	}

	int Game::run()
	{
		MSG msg = { 0 };
		timer_.reset();
		static constexpr int targetFPS = 60; // TODO: make generic

		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				timer_.tick();

				if (!is_paused_) [[likely]]
				{
					calculateFrameStats();
					onUpdate(timer_.deltaTime());
					Draw();

					timer_.limitFPS(targetFPS);
				}
				else
				{
					WaitMessage();
				}
			}
		}

		return static_cast<int>(msg.wParam);
	}

	void Game::calculateFrameStats()
	{
		static int frameCnt{};
		static double timeElapsed{};

		frameCnt++;

		if ((timer_.totalTime() - timeElapsed) >= 1.0)
		{
			const float fps = static_cast<float>(frameCnt);
			const float mspf = 1000.0f / fps;

			std::wstring fpsStr  = std::to_wstring(fps);
			std::wstring mspfStr = std::to_wstring(mspf);

			std::wstring windowText = appName_ +
				L"    fps: " + fpsStr +
				L"   mspf: " + mspfStr;

			SetWindowText(display_.hwnd, windowText.c_str());

			frameCnt = 0;
			timeElapsed++;
		}
	}

	bool Game::initMainWindow(HINSTANCE hInstance)
	{
		struct ConfigData {
			uint32_t width{ 800u };
			uint32_t height{ 800u };
			std::string appName{ "MyApp" };
			bool with_cursore{ true };
		};

		ConfigData configData;

		HANDLE configOnLoad = CreateFile(
			L"resources/config.txt",
			GENERIC_READ,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (configOnLoad != INVALID_HANDLE_VALUE)
		{
			LARGE_INTEGER size;
			GetFileSizeEx(configOnLoad, &size);
			auto bufferData = std::make_unique<char[]>(size.QuadPart);

			DWORD bytesIterated;
			if (ReadFile(configOnLoad, bufferData.get(), static_cast<DWORD>(size.QuadPart), &bytesIterated, NULL))
			{
				auto parseCmd = utils::ConfigParser<ConfigData>::make_unique({
				    {"WINDOW_WIDTH ", &ConfigData::width},
				    {"WINDOW_HEIGHT ", &ConfigData::height},
				    {"APP_NAME ", &ConfigData::appName},
				    {"CURSOR_MODE ", &ConfigData::with_cursore}
				});

				configData = parseCmd->parse(bufferData.get());
			}

			CloseHandle(configOnLoad);
		}
		else {
			LOG(WARNING) << "Failed to open config file";
		}

		appName_ = strings::stringToWstring(configData.appName);

		auto [config, is_valid] = DisplayBuilder{}
			.withHeight(configData.height)
			.withWidth(configData.width)
			.withAppName(appName_)
			.withHINSTANCE(hInstance)
			.withCursore(configData.with_cursore)
			.build();

		if (is_valid) {
			LOG(INFO) << "Window has created with height=" << configData.height << ", width=" << configData.width;
		}
		else {
			LOG(JERROR) << "Window creation problems!";
		}

		display_ = config;

		return is_valid;
	}

	bool Game::initDirect3D()
	{
		D3D_FEATURE_LEVEL featureLevel[] = { D3D_FEATURE_LEVEL_11_1 };

		DXGI_SWAP_CHAIN_DESC swapDesc = {};
		swapDesc.BufferCount = 2;
		swapDesc.BufferDesc.Width = display_.clientWidth;
		swapDesc.BufferDesc.Height = display_.clientHeight;
		swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapDesc.OutputWindow = display_.hwnd;
		swapDesc.Windowed = true;
		swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		swapDesc.SampleDesc.Count = 1;
		swapDesc.SampleDesc.Quality = 0;

		ThrowIfFailed(D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			D3D11_CREATE_DEVICE_DEBUG,
			featureLevel,
			1,
			D3D11_SDK_VERSION,
			&swapDesc,
			&swapChain_,
			&device_,
			nullptr,
			&context_));

		LOG(INFO) << "Game RTV with swap buffers has created";
		is_rtv_init_ = true; // TODO: maybe state machine in the future with error handling?

		onResize();

		return true;
	}

	bool Game::initComponents()
	{
		components_.push_back(std::make_shared<PongGameComponent>());

		for (auto& component : components_) {
			component->onInit();
		}

		return true;
	}

	LRESULT Game::msgHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_CREATE:
		{
			// TODO:
			break;
		}

		case WM_INPUT:
		{
			UINT dwSize{};
			GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
			auto lpb = std::make_unique<BYTE[]>(dwSize);
			if (lpb == nullptr) {
				break;
			}

			if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, lpb.get(), &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) [[unlikely]] {
				LOG(WARNING) << "GetRawInputData does not return correct size!";
			}

			RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(lpb.get());

			if (raw->header.dwType == RIM_TYPEKEYBOARD)
			{
				inputDevice_->onKeyDown({
					raw->data.keyboard.MakeCode,
					raw->data.keyboard.Flags,
					raw->data.keyboard.VKey,
					raw->data.keyboard.Message
					});
			}
			else if (raw->header.dwType == RIM_TYPEMOUSE)
			{
				inputDevice_->onMouseMove({
					raw->data.mouse.usFlags,
					raw->data.mouse.usButtonFlags,
					static_cast<int>(raw->data.mouse.ulExtraInformation),
					static_cast<int>(raw->data.mouse.ulRawButtons),
					static_cast<short>(raw->data.mouse.usButtonData),
					raw->data.mouse.lLastX,
					raw->data.mouse.lLastY
					});
			}
		}
		    break;
		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE) {
				PostMessage(hwnd, WM_DESTROY, 0, 0);
			}
			break;

		case WM_SIZE:
			display_.clientWidth  = LOWORD(lParam);
			display_.clientHeight = HIWORD(lParam);
			display_.aspect = static_cast<float>(display_.clientWidth) / static_cast<float>(display_.clientHeight);
			onResize();
			break;

		case WM_MENUCHAR:
			// Don't beep when we alt-enter.
			return MAKELRESULT(0, MNC_CLOSE);

			// Catch this message so to prevent the window from becoming too small.
		case WM_GETMINMAXINFO:
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
			break;

		case WM_DESTROY:
		{
			for (auto& component : components_) {
				component->onDestroy();
			}

			LOG(INFO) << "Successfully end programm!";

			PostQuitMessage(0);
			ExitProcess(0);
			break;
		}
		}

		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	void Game::onResize()
	{
		if (not is_rtv_init_) {
			return;
		}

		rtv_.Reset();

		ThrowIfFailed(swapChain_->ResizeBuffers(
			2,
			display_.clientWidth,
			display_.clientHeight,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			0));

		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
		ThrowIfFailed(swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer));
		ThrowIfFailed(device_->CreateRenderTargetView(backBuffer.Get(), nullptr, rtv_.GetAddressOf()));

		D3D11_VIEWPORT viewport = {};
		viewport.Width = static_cast<float>(display_.clientWidth);
		viewport.Height = static_cast<float>(display_.clientHeight);
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1.0f;

		context_->RSSetViewports(1, &viewport);
		context_->OMSetRenderTargets(0, nullptr, nullptr);
	}

	void Game::onUpdate(double deltaTime)
	{
		for (auto& component : components_) {
			component->onUpdate(deltaTime);
		}
	}

	void Game::Draw()
	{
		float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };

		context_->OMSetRenderTargets(1, rtv_.GetAddressOf(), nullptr);
		context_->ClearRenderTargetView(rtv_.Get(), color);

		for (auto& component : components_) {
			component->Draw();
		}

		context_->OMSetRenderTargets(0, nullptr, nullptr);

		swapChain_->Present(1, /*DXGI_PRESENT_DO_NOT_WAIT*/ 0);
	}
}