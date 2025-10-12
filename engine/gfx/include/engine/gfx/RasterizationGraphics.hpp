#pragma once

#include "Graphics.hpp"
#include "DynamicCubeMap.hpp"
#include "ShadowMap.hpp"
#include "engine/core/winCPUTickTimer.hpp"

class RasterizationGraphics : public Graphics
{
public:
	using Ptr = std::unique_ptr<RasterizationGraphics>;

	RasterizationGraphics(GraphicsResources&);
	~RasterizationGraphics();

	RasterizationGraphics(const RasterizationGraphics&) = delete;
	RasterizationGraphics(RasterizationGraphics&&) = delete;
	RasterizationGraphics& operator=(const RasterizationGraphics&) = delete;
	RasterizationGraphics& operator=(RasterizationGraphics&&) = delete;

public:
	void Update(float deltaTime, float totalTime) override;

private:
	void LoadAssets() override;
	void PopulateCommandList() override;

private:
	PSO_Manager<GraphicsPSO> m_graphicsPSOsManager;

	misc::winCpuTickTimer<float> m_timer;

	Object::Vec m_dynamicGameComponents;

	BaseCamera::Ptr m_inspectionCamera;

	DynamicCubeMap::Ptr m_dynamicCubeMap;
	ShadowMap::Ptr m_shadowMap;
	TextureRenderer::Ptr m_textureRenderer;
};