#pragma once

#include "GeometryRenderer.hpp"
#include "Object.hpp"

class TerrainRenderer : public GeometryRenderer, public Object
{
public:
	using Ptr = std::unique_ptr<TerrainRenderer>;

	static TerrainRenderer::Ptr CreateTerrainRenderer(misc::render_descriptors::DX_TERRAIN_DESCRIPTOR& descriptor);

	void Render(misc::rasterization::RenderLayer::Value renderLayer) const override;
	void FrustumCulling(const CameraController& cameraController) override;
	void BuildAccelerationStructures() override;
	void Update(float delatTime) override;

private:
	TerrainRenderer() = delete;
	TerrainRenderer(const misc::render_descriptors::DX_OBJECT_DESCRIPTOR&);

	void LoadGeometry(DescriptorVariant descriptor) override;

	std::vector<Chunk> m_chunks;
};