#pragma once

#include "GeometryRenderer.hpp"
#include "Object.hpp"

namespace engine::gfx
{

class TerrainRenderer : public GeometryRenderer, public Object
{
public:
	using Ptr = std::unique_ptr<TerrainRenderer>;

	static TerrainRenderer::Ptr CreateTerrainRenderer(engine::gfx::render_descriptors::DX_TERRAIN_DESCRIPTOR& descriptor);

	void Render(engine::gfx::rasterization::RenderLayer::Value renderLayer) const override;
	void FrustumCulling(const CameraController& cameraController) override;
	void BuildAccelerationStructures() override;
	void Update(float delatTime) override;

private:
	TerrainRenderer() = delete;
	TerrainRenderer(const engine::gfx::render_descriptors::DX_OBJECT_DESCRIPTOR&);

	void LoadGeometry(DescriptorVariant descriptor) override;

	std::vector<Chunk> m_chunks;
};

}  // namespace engine::gfx
