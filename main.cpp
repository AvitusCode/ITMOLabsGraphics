// MySuper3DApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#include <WinUser.h>
#include <wrl.h>
#include <iostream>
#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

#include "game.hpp"
#include "logger.hpp"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	jd::logging::InternalLogger::InitLog(jd::logging::DefaultLogFile);
	LOG(INFO) << "ITMO graphics project VERSION = 0.01 has started";

	try
	{
		auto& game = jd::Game::getGame();
		if (!game.initialize(hInstance)) {
			return 0;
		}

		return game.run();
	}
	catch (const std::exception& ex)
	{
		LOG(JERROR) << ex.what() << std::endl;
		MessageBox(nullptr, L"Ops!", L"HR Failed", MB_OK);
		return 0;
	}
}
