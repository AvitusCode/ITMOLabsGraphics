#include "input_device.hpp"

#include <iostream>

#include "game.hpp"
#include "logger.hpp"
#include "string_utils.hpp"

namespace jd
{
	InputDevice::InputDevice(Game& inGame) 
		: game_(inGame)
	{
		RAWINPUTDEVICE Rid[2];

		Rid[0].usUsagePage = 0x01;
		Rid[0].usUsage = 0x02; // mouse identificator
		Rid[0].dwFlags = 0;   // adds HID mouse and also ignores legacy mouse messages
		Rid[0].hwndTarget = game_.getDisplay().hwnd;

		Rid[1].usUsagePage = 0x01;
		Rid[1].usUsage = 0x06; // standart keyboard identificator
		Rid[1].dwFlags = 0;   // adds HID keyboard and also ignores legacy keyboard messages
		Rid[1].hwndTarget = game_.getDisplay().hwnd;

		if (RegisterRawInputDevices(Rid, 2, sizeof(Rid[0])) == FALSE)
		{
			auto errorCode = GetLastError();
			LOG(JERROR) << "Problems with the devices registration, error=" << errorCode;
		}
	}

	void InputDevice::onKeyDown(KeyboardInputEventArgs args)
	{
		bool Break = args.Flags & 0x01;

		auto key = static_cast<Keys>(args.VKey);

		if (args.MakeCode == 42) {
			key = Keys::LeftShift;
		}
		else if (args.MakeCode == 54) {
			key = Keys::RightShift;
		}

		if (Break) {
			if (keys_.test(to_underlying(key))) {
				removePressedKey(key);
			}
		}
		else {
			if (!keys_.test(to_underlying(key))) {
				addPressedKey(key);
			}
		}
	}

	void InputDevice::onMouseMove(RawMouseEventArgs args)
	{
		if (args.ButtonFlags & static_cast<int32_t>(MouseButtonFlags::LeftButtonDown))
			addPressedKey(Keys::LeftButton);
		if (args.ButtonFlags & static_cast<int32_t>(MouseButtonFlags::LeftButtonUp))
			removePressedKey(Keys::LeftButton);
		if (args.ButtonFlags & static_cast<int32_t>(MouseButtonFlags::RightButtonDown))
			addPressedKey(Keys::RightButton);
		if (args.ButtonFlags & static_cast<int32_t>(MouseButtonFlags::RightButtonUp))
			removePressedKey(Keys::RightButton);
		if (args.ButtonFlags & static_cast<int32_t>(MouseButtonFlags::MiddleButtonDown))
			addPressedKey(Keys::MiddleButton);
		if (args.ButtonFlags & static_cast<int32_t>(MouseButtonFlags::MiddleButtonUp))
			removePressedKey(Keys::MiddleButton);

		POINT p;
		GetCursorPos(&p);
		ScreenToClient(game_.getDisplay().hwnd, &p);

		mousePosition_ = p;
		mouseOffset_ = POINT{ .x = args.X, .y = args.Y };
		mouseWheelDelta_ = args.WheelDelta;

		const MouseMoveEventArgs moveArgs = { 
			.position = mousePosition_,
			.offset = mouseOffset_,
			.wheelDelta = mouseWheelDelta_
		};

		/*DLOG(INFO) << strings::print("Mouse: posX=%04.4f posY:%04.4f offsetX:%04.4f offsetY:%04.4f, wheelDelta=%04d",
			mousePosition.x,
			mousePosition.y,
			mouseOffset.x,
			mouseOffset.y,
			mouseWheelDelta);*/

		mouseMove_.Broadcast(moveArgs);
	}

	void InputDevice::addPressedKey(Keys key)
	{
		keys_.set(to_underlying(key));
	}

	void InputDevice::removePressedKey(Keys key)
	{
		keys_.reset(to_underlying(key));
	}

	bool InputDevice::isKeyDown(Keys key) const
	{
		return keys_.test(to_underlying(key));
	}
}
