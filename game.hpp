#pragma once
#include "jd_types.hpp"

#include <windows.h>
#include <WinUser.h>
#include <wrl.h>
#include <d3d11.h>

#include <vector>
#include <array>

#include "display_win32.hpp"
#include "game_timer.hpp"
#include "input_device.hpp"
#include "menu_ptr.hpp"
#include "camera_event.hpp"
#include "static_component.hpp"

#include "pick_event.hpp"

namespace jd
{
	class GameComponent;

	class Game final
	{
	public:
		enum MenuCommands : UINT
		{
			MENU_SELECT_OBJECT_BASE = 1000,
			MENU_ACTION_SPEED_UP = 2000,
			MENU_ACTION_SLOW_DOWN = 2001,
			MENU_ACTION_CHANGE_CAM = 2002
		};

		static Game& getGame() {
			static Game game;
			return game;
		}

		Game(const Game&) = delete;
		Game(Game&&) = delete;
		Game& operator=(const Game&) = delete;
		Game& operator=(Game&&) = delete;

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
		menu_ptr contextMenu_;
		HMENU selectSubMenu_;

		DelegateHandle mouseDelegateHandle_;
		Delegate<void, const event::CameraEvent&> cameraDelegate_;
		Delegate<StaticInfo, const event::SimplePickEvent&> pickCubesDelegate_;
		Delegate<StaticInfo, const event::SimplePickEvent&> pickSpheresDelegate_;

		Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_;
		Microsoft::WRL::ComPtr<ID3D11Device> device_;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv_;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView_;

		size_t objects_{};
		size_t objectsPInstance_{};

		std::vector<sptr<GameComponent>> components_; // TODO: intrusive ptr

		bool is_paused_{ false };
		bool is_rtv_init_{ false };

		Game() = default;

		bool initMainWindow(HINSTANCE hInstance);
		bool initDirect3D();
		bool initComponents();
		bool createDepthStencilView();
		void createContextMenu();
		void printInfo();

		void applyWMCommand(HWND hwnd, WPARAM wParam, LPARAM lParam);

		void onResize();
		void onUpdate(double deltaTime);
		void Draw();
	};
}