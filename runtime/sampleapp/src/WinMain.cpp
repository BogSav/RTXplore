#include "App.hpp"
#include "engine/platform/Window.hpp"

void DisplayMessageBox(Window* window, const char* what, const char* type)
{
	HWND hWnd = window == nullptr ? nullptr : window->GetWindowHandler();
	OutputDebugStringA(what);
	MessageBoxA(hWnd, what, type, MB_OK | MB_ICONEXCLAMATION);
}

int main()
{
#ifdef _DEBUG
	// _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtMemState sOld;
	_CrtMemState sNew;
	_CrtMemState sDiff;
	_CrtMemCheckpoint(&sOld);
#endif

	std::unique_ptr<Window> window;
	try
	{
		window = std::make_unique<Window>(1200, 600, L"Happy Window");
		App app(*window);
		app.Run();
	}
	catch (const misc::CustomException& e)
	{
		OutputDebugStringA(e.what());
		DisplayMessageBox(window.get(), e.what(), e.GetType());
	}
	catch (const std::exception& e)
	{
		OutputDebugStringA(e.what());
		DisplayMessageBox(window.get(), e.what(), "Standard exceptiuon");
	}
	catch (...)
	{
		DisplayMessageBox(window.get(), "No idea what this is", "How???");
	}
	delete window.release();
	window = nullptr;

	GraphicsResources::DestroyInstance();

#ifdef _DEBUG
	DxgiInfoManager::DestroyInstance();

	_CrtMemCheckpoint(&sNew);
	if (_CrtMemDifference(&sDiff, &sOld, &sNew))
	{
		OutputDebugString(L"-----------_CrtMemDumpStatistics ---------");
		_CrtMemDumpStatistics(&sDiff);
		OutputDebugString(L"-----------_CrtMemDumpAllObjectsSince ---------");
		_CrtMemDumpAllObjectsSince(&sOld);
		OutputDebugString(L"-----------_CrtDumpMemoryLeaks ---------");
		_CrtDumpMemoryLeaks();
	}
#endif

	return -1;
}
