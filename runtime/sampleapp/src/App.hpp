#pragma once

#include "engine/platform/Window.hpp"
#include "engine/core/TickTimer.hpp"

#include "engine/gfx/RasterizationGraphics.hpp"
#include "engine/gfx/RayTracingGraphics.hpp"

class App
{
public:
	App(Window& wnd);
	int Run();

private:
	void CalculateFrameStats();
	void Update();

private:
	Window& window;
	Graphics::Ptr pGraphics;

	misc::TickTimer<float> m_timer;
};
