#pragma once

#include "engine/platform/Window.hpp"
#include "engine/core/winCPUTickTimer.hpp"

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

	misc::winCpuTickTimer<float> m_timer;
};
