#pragma once
#include "jd_types.hpp"

#include <windows.h>
#include <WinUser.h>
#include <wrl.h>
#include <d3d11.h>

#include <vector>

#include "display_win32.hpp"
#include "game_timer.hpp"
#include "input_device.hpp"

namespace jd
{
	class GameComponent;

	class Game final
	{
	public:

		static Game& getGame() {
			static Game game;
			return game;
		}

		bool initialize(HINSTANCE hInstance);
		LRESULT msgHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		int run();

		auto getContext() const {
			return context_;
		}

		auto getDevice() const {
			return device_;
		}

		const InputDevice& getInputDevice() const noexcept
		{
			return *inputDevice_;
		}

		const DisplayWin32& getDisplay() const noexcept {
			return display_;
		}

	private:
		DisplayWin32 display_;
		GameTimer timer_;
		std::wstring appName_{ L"MyApp" };

		uptr<InputDevice> inputDevice_;

		Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_;
		Microsoft::WRL::ComPtr<ID3D11Device> device_;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv_;

		std::vector<sptr<GameComponent>> components_; // TODO: intrusive ptr

		bool is_paused_{ false };
		bool is_rtv_init_{ false };

		bool initMainWindow(HINSTANCE hInstance);
		bool initDirect3D();
		bool initComponents();
		void calculateFrameStats();

		void onResize();
		void onUpdate(double deltaTime);
		void Draw();
	};
}