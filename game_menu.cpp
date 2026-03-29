#include "game.hpp"

#include "camera_event.hpp"
#include "logger.hpp"

namespace jd
{
	void Game::createContextMenu()
	{
		contextMenu_.reset(CreatePopupMenu());
		selectSubMenu_ = CreatePopupMenu();

		for (size_t i = 0; i < objects_; ++i)
		{
			std::wstring name = L"Object " + std::to_wstring(i);
			AppendMenu(selectSubMenu_, MF_STRING, MENU_SELECT_OBJECT_BASE + i, name.c_str());
		}

		AppendMenu(contextMenu_.get(), MF_POPUP, (UINT_PTR)selectSubMenu_, L"Select Object");
		AppendMenu(contextMenu_.get(), MF_SEPARATOR, 0, nullptr);
		AppendMenu(contextMenu_.get(), MF_STRING, MENU_ACTION_CHANGE_CAM, L"Change Camera");
		AppendMenu(contextMenu_.get(), MF_STRING, MENU_ACTION_SPEED_UP, L"Speed Up");
		AppendMenu(contextMenu_.get(), MF_STRING, MENU_ACTION_SLOW_DOWN, L"Slow Down");
	}

	void Game::applyWMCommand(HWND hwnd, WPARAM wParam, LPARAM lParam)
	{
		UINT id = LOWORD(wParam);
		static UINT selectedObject_ = 0u;
		static UINT lastSelectedObject_ = ~0u;
		static auto camType = event::CameraType::FPS;
		static StaticInfo info{};

		if (id >= MENU_SELECT_OBJECT_BASE && id < MENU_SELECT_OBJECT_BASE + objects_)
		{
			selectedObject_ = id - MENU_SELECT_OBJECT_BASE;

			LOG(INFO) << "Selected object: " << selectedObject_;

			auto ev = event::SimplePickEvent{
					.color = DirectX::SimpleMath::Vector4(1.0f, 0.0f, 0.0f, 1.0f),
					.indx = static_cast<uint32_t>(selectedObject_ % objectsPInstance_)
			};

			if (selectedObject_ >= objectsPInstance_) {
				info = pickSpheresDelegate_.Execute(ev);
			}
			else {
				info = pickCubesDelegate_.Execute(ev);
			}

			if (lastSelectedObject_ != ~0u) {
				auto ev = event::SimplePickEvent{
					.color = DirectX::SimpleMath::Vector4(1.0f, 1.0f, 1.0f, 1.0f),
					.indx = static_cast<uint32_t>(lastSelectedObject_ % objectsPInstance_)
				};

				if (lastSelectedObject_ >= objectsPInstance_) {
					pickSpheresDelegate_.Execute(ev);
				}
				else {
					pickCubesDelegate_.Execute(ev);
				}
			}

			lastSelectedObject_ = selectedObject_;

			CheckMenuRadioItem(
				selectSubMenu_,
				MENU_SELECT_OBJECT_BASE,
				MENU_SELECT_OBJECT_BASE + objects_ - 1,
				MENU_SELECT_OBJECT_BASE + selectedObject_,
				MF_BYCOMMAND
			);
		}
		else if (id == MENU_ACTION_SPEED_UP)
		{
			LOG(INFO) << "Speed up object: " << selectedObject_;
		}
		else if (id == MENU_ACTION_SLOW_DOWN)
		{
			LOG(INFO) << "Slow down object: " << selectedObject_;
		}
		else if (id == MENU_ACTION_CHANGE_CAM)
		{
			auto ev = event::CameraEvent{};
			ev.fov = 60.0f;
			if (camType == event::CameraType::FPS) {
				ev.type = event::CameraType::ORBIT;
				ev.info = info;
			}
			else {
				ev.type = event::CameraType::FPS;
			}
			camType = ev.type;
			cameraDelegate_.Execute(ev);
			LOG(INFO) << "Change camera";
		}
	}
}