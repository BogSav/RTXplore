#include "App.hpp"

#include "engine/core/Utilities.hpp"
#include "engine/core/Settings.hpp"
#include "engine/platform/Window.hpp"

#include <sstream>

App::App(Window& wnd) : window(wnd)
{
	if (engine::core::Settings::UseRayTracing())
	{
		pGraphics = std::make_unique<RayTracingGraphics>(GraphicsResources::GetInstance());
	}
	else
	{
		pGraphics = std::make_unique<RasterizationGraphics>(GraphicsResources::GetInstance());
	}
}

int App::Run()
{
	m_timer.StartClock();

	while (true)
	{
		const auto msg = Window::ProcessMessage();
		if (msg.has_value())
		{
			return *msg;
		}

		m_timer.UpdateDeltaTime();
		CalculateFrameStats();
		Update();
	}
}

void App::Update()
{
	if (GraphicsResources::GetInstance().GetSizeChnaged())
	{
		pGraphics->GetCameraController().RefreshCamera();
		GraphicsResources::GetInstance().SetSizeChnaged(false);
	}

	if (window.GetMouse().LeftIsPressed())
	{
		const auto ok = window.GetMouse().Read();
		if (ok.has_value() && ok->GetType() == Mouse::Event::Type::Move)
		{
			pGraphics->GetCameraController().RotateFirstPerson_OY(
				ok->GetDeltaX() * engine::core::Settings::GetGameSettings().GetMouseSensitivityOX());
			pGraphics->GetCameraController().RotateFirstPerson_OX(
				ok->GetDeltaY() * -engine::core::Settings::GetGameSettings().GetMouseSensitivityOY());
		}
	}
	else
	{
		window.GetMouse().Flush();
	}

	const float moveSpeed = 30;
	if (window.GetKeyboard().KeyIsPressed('W'))
		pGraphics->GetCameraController().MoveForward(moveSpeed * m_timer.GetDeltaTime());
	if (window.GetKeyboard().KeyIsPressed('S'))
		pGraphics->GetCameraController().MoveForward(-moveSpeed * m_timer.GetDeltaTime());
	if (window.GetKeyboard().KeyIsPressed('A'))
		pGraphics->GetCameraController().TranslateRight(moveSpeed * m_timer.GetDeltaTime());
	if (window.GetKeyboard().KeyIsPressed('D'))
		pGraphics->GetCameraController().TranslateRight(-moveSpeed * m_timer.GetDeltaTime());
	if (window.GetKeyboard().KeyIsPressed('E'))
		pGraphics->GetCameraController().TranslateUpward(-moveSpeed * m_timer.GetDeltaTime());
	if (window.GetKeyboard().KeyIsPressed('Q'))
		pGraphics->GetCameraController().TranslateUpward(moveSpeed * m_timer.GetDeltaTime());

	const char ok = window.GetKeyboard().ReadChar();
	if (ok == 'c')
	{
		std::stringstream text;
		text << "===================================================="
			 << "\n";
		text << "Position: " << pGraphics->GetCamera().GetPosition().ToString() << "\n";
		text << "Forward: " << pGraphics->GetCamera().GetForward().ToString() << "\n";
		text << "Up: " << pGraphics->GetCamera().GetUp().ToString() << "\n";
		text << "====================================================="
			 << "\n";
		OutputDebugStringA(text.str().c_str());
	}

	// Set the render mode
	if (window.GetKeyboard().KeyIsPressed('1'))
		pGraphics->SetRenderMode(RenderMode::Everything);
	if (window.GetKeyboard().KeyIsPressed('2'))
		pGraphics->SetRenderMode(RenderMode::Everything & (~RenderMode::BigWaves));
	if (window.GetKeyboard().KeyIsPressed('3'))
		pGraphics->SetRenderMode(RenderMode::Everything & (~RenderMode::SmallWaves));
	if (window.GetKeyboard().KeyIsPressed('4'))
		pGraphics->SetRenderMode(RenderMode::Everything & ~(RenderMode::BigWaves | RenderMode::SmallWaves));

	///////////////////////////////////////////////////////
	// Actualizarea camerei se face in pGraphics pentru a nu relua procesul de frustum culling in cazul in care camera
	// nu s-a miscat
	///////////////////////////////////////////////////////

	pGraphics->Update(m_timer.GetDeltaTime(), m_timer.GetTimeSinceStart());
	pGraphics->Render();
}

void App::CalculateFrameStats()
{
	using namespace std;

	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	if ((m_timer.GetTimeSinceStart() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt;
		float mspf = 1000.0f / fps;

		wstring fpsStr = to_wstring(fps);
		wstring mspfStr = to_wstring(mspf);
		wstring windowText = L"fps: " + fpsStr + L" mspf: " + mspfStr;

		window.SetTitle(windowText.c_str());

		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}